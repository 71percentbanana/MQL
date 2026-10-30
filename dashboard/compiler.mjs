import { execFile } from 'node:child_process';
import { promisify } from 'node:util';
import { mkdtemp, writeFile, unlink, rmdir, stat } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { join, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';

const execute = promisify(execFile);
export const projectRoot = dirname(dirname(fileURLToPath(import.meta.url)));
export const binary = join(projectRoot, process.platform === 'win32' ? 'sql2mongo.exe' : 'sql2mongo');
export const phaseDefinitions = [
  { id: 1, name: 'Lexical analysis', short: 'Lexer', description: 'The scanner turns SQL characters into keywords, identifiers, operators, and literals.', artifact: 'Token stream' },
  { id: 2, name: 'Syntax analysis', short: 'Parser', description: 'The parser checks the grammar and builds the concrete parse tree and abstract syntax tree.', artifact: 'CST + AST' },
  { id: 3, name: 'Semantic analysis', short: 'Semantics', description: 'The schema catalog validates collection names, field names, and comparison types.', artifact: 'Symbol table' },
  { id: 4, name: 'Intermediate code', short: 'IR generation', description: 'The compiler represents the query as three-address instructions, quadruples, and triples.', artifact: 'TAC + quads + triples' },
  { id: 5, name: 'Code optimization', short: 'Optimizer', description: 'Inspect the compiler’s reported predicate pushdown, projection pruning, and literal-resolution passes.', artifact: 'Optimization report' },
  { id: 6, name: 'Target generation', short: 'MongoDB code', description: 'The code generator emits a MongoDB aggregation pipeline for the validated SQL query.', artifact: 'MongoDB pipeline' },
  { id: 7, name: 'Query execution', short: 'Execution', description: 'The C execution engine loads the collection from data/<collection>.json, then filters, sorts, limits, and projects it.', artifact: 'Result documents' },
];

function cleanSection(section) {
  return section.replace(/^={5,}\s*$/gm, '').replace(/\n{3,}/g, '\n\n').trim();
}

export function parseOutput(stdout, stderr = '', exitCode = 0, durationMs = 0) {
  const raw = stdout.replace(/\r\n/g, '\n');
  const headings = [...raw.matchAll(/^  STAGE (\d)([ab]?): ([^\n]+)\n=+\n/gm)];
  const parts = headings.map((match, index) => ({
    id: Number(match[1]), variant: match[2], title: match[3],
    output: cleanSection(raw.slice(match.index + match[0].length, headings[index + 1]?.index ?? raw.length).split(/\n=+\n  FINAL VERIFICATION:/)[0]),
  }));
  const error = stderr.trim() || (exitCode !== 0 ? 'The compiler could not complete this query.' : null);
  const failedPhase = exitCode === 0 ? null : /LEXICAL ERROR|Lexical error/.test(raw + stderr) ? 1 : /SYNTAX VALIDATION|Syntax error/.test(raw + stderr) ? 2 : /Semantic/.test(raw + stderr) ? 3 : (parts.at(-1)?.id ?? 1);
  const phases = phaseDefinitions.map(phase => ({
    ...phase,
    status: failedPhase === phase.id ? 'error' : failedPhase && phase.id > failedPhase ? 'skipped' : parts.some(part => part.id === phase.id) ? 'complete' : 'skipped',
    parts: parts.filter(part => part.id === phase.id),
  }));
  if (failedPhase && !phases[failedPhase - 1].parts.length) phases[failedPhase - 1].parts.push({ id: failedPhase, title: 'Compiler diagnostic', variant: '', output: error });
  const tokens = [];
  for (const match of (phases[0].parts[0]?.output ?? '').matchAll(/^\|\s*(\d+)\s*\|\s*(.*?)\s*\|\s*(.*?)\s*\|\s*(\d+)\s*\|\s*(\d+)\s*\|$/gm)) {
    tokens.push({ index: Number(match[1]), type: match[2], lexeme: match[3], line: Number(match[4]), column: Number(match[5]) });
  }
  const schema = [];
  for (const match of (phases[2].parts[0]?.output ?? '').matchAll(/^\|\s*Table\s*\|\s*(\w+)\s*\|\s*(\w+)\s*\|\s*(\w+)\s*\|\s*(\d+) B\s*\|$/gm)) {
    schema.push({ collection: match[1], field: match[2], type: match[3], bytes: Number(match[4]) });
  }
  const execution = phases[6].parts[0]?.output ?? '';
  const meta = execution.match(/Database: (\w+) \| Collection: (\w+) \| Evaluated: (\d+) \| Returned: (\d+)/);
  const documents = [];
  for (const match of execution.matchAll(/^\{\n[\s\S]*?^\}/gm)) {
    try { documents.push(JSON.parse(match[0])); } catch { /* Preserve the raw output when the compiler emits invalid JSON. */ }
  }
  const mongo = (phases[5].parts[0]?.output ?? '').match(/db\.[\s\S]*?\]\);/)?.[0] ?? '';
  let pipeline = [];
  try { pipeline = JSON.parse(mongo.slice(mongo.indexOf('['), mongo.lastIndexOf(']') + 1)); } catch { /* Generated text remains visible. */ }
  return {
    success: exitCode === 0 && /Status: Query Executed Successfully/.test(execution),
    exitCode, durationMs: Math.round(durationMs * 100) / 100, error,
    phases, tokens, schema, mongo, pipeline, documents,
    database: meta?.[1] ?? null, collection: meta?.[2] ?? null,
    evaluated: meta ? Number(meta[3]) : null, returned: meta ? Number(meta[4]) : null,
    raw: raw + (stderr ? '\n[stderr]\n' + stderr : ''),
  };
}

let active = 0;
export async function compile(sql) {
  if (typeof sql !== 'string' || !sql.trim()) throw Object.assign(new Error('Enter a SQL query to compile.'), { status: 400 });
  if (sql.length > 4000) throw Object.assign(new Error('Keep queries under 4,000 characters.'), { status: 400 });
  if (sql.includes('\0')) throw Object.assign(new Error('SQL cannot contain null characters.'), { status: 400 });
  if (active >= 4) throw Object.assign(new Error('The compiler is busy. Try again in a moment.'), { status: 429 });
  try { await stat(binary); } catch { throw Object.assign(new Error('Compiler executable missing. Run npm run build:compiler from the dashboard folder.'), { status: 503 }); }
  active++;
  let directory;
  const started = performance.now();
  try {
    directory = await mkdtemp(join(tmpdir(), 'sql2mongo-dashboard-'));
    const queryFile = join(directory, 'query.sql');
    await writeFile(queryFile, sql, 'utf8');
    let stdout = '', stderr = '', exitCode = 0;
    try {
      ({ stdout, stderr } = await execute(binary, [queryFile, '--all'], { cwd: projectRoot, windowsHide: true, timeout: 6000, maxBuffer: 2 * 1024 * 1024 }));
    } catch (error) {
      if (error.killed) throw Object.assign(new Error('The compiler exceeded the 6-second time limit. Simplify your query and try again.'), { status: 504 });
      if (error.code === 'ENOENT' || error.code === 'EACCES') throw Object.assign(new Error('Unable to start the compiler executable. Rebuild it with npm run build:compiler.'), { status: 503 });
      stdout = error.stdout ?? '';
      stderr = error.stderr || `Compiler exited with code ${error.code}.`;
      exitCode = typeof error.code === 'number' ? error.code : 1;
    }
    return { sql, ...parseOutput(stdout, stderr, exitCode, performance.now() - started) };
  } finally {
    active--;
    if (directory) {
      await unlink(join(directory, 'query.sql')).catch(() => {});
      await rmdir(directory).catch(() => {});
    }
  }
}
