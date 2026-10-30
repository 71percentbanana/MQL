import http from 'node:http';
import { readFile, readdir, writeFile } from 'node:fs/promises';
import { dirname, join } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { compile, phaseDefinitions, projectRoot } from './compiler.mjs';

const publicRoot = join(dirname(fileURLToPath(import.meta.url)), 'public');
const defaultDataRoot = join(projectRoot, 'data');
const maxDocuments = 32; // MAX_DOCS in src/engine.c
const collectionOrder = file => { const index = ['students.json', 'employees.json'].indexOf(file); return index === -1 ? Infinity : index; };
const assets = new Map([
  ['/', ['index.html', 'text/html; charset=utf-8']],
  ['/styles.css', ['styles.css', 'text/css; charset=utf-8']],
  ['/app.js', ['app.js', 'text/javascript; charset=utf-8']],
  ['/theme.js', ['theme.js', 'text/javascript; charset=utf-8']],
  ['/favicon.svg', ['favicon.svg', 'image/svg+xml']],
]);

function json(response, status, value) {
  response.writeHead(status, { 'Content-Type': 'application/json; charset=utf-8', 'Cache-Control': 'no-store' });
  response.end(JSON.stringify(value));
}

async function readBody(request) {
  const chunks = [];
  let size = 0;
  for await (const chunk of request) {
    size += chunk.length;
    if (size > 16384) throw Object.assign(new Error('Request body too large.'), { status: 413 });
    chunks.push(chunk);
  }
  try { return JSON.parse(Buffer.concat(chunks).toString('utf8')); }
  catch { throw Object.assign(new Error('Send a valid JSON request body.'), { status: 400 }); }
}

const fail = (status, message) => { throw Object.assign(new Error(message), { status }); };

// Browsers may post only from the dashboard itself, and only as JSON.
function postError(request) {
  if (request.headers.origin && request.headers.origin !== `http://${request.headers.host}`) return [403, 'Cross-origin requests are not allowed.'];
  if (!request.headers['content-type']?.startsWith('application/json')) return [415, 'Use application/json for this request.'];
  return null;
}

// Field types come from the compiler's own symbol table, so new documents match what semantic analysis checks.
let catalog = null;
async function schemaCatalog() {
  if (!catalog) {
    const { schema } = await compile('SELECT * FROM students;');
    if (!schema.length) fail(503, 'Could not read the schema from the compiler.');
    catalog = schema;
  }
  return catalog;
}

async function readDatabase(dataRoot) {
  // The same files the C engine reads at execution time.
  const schema = await schemaCatalog().catch(() => []);
  const files = (await readdir(dataRoot)).filter(name => name.endsWith('.json')).sort((a, b) => collectionOrder(a) - collectionOrder(b) || a.localeCompare(b));
  const collections = await Promise.all(files.map(async file => {
    const name = file.slice(0, -'.json'.length);
    const fields = schema.filter(item => item.collection === name).map(item => ({ name: item.field, type: item.type }));
    try { return { name, fields, documents: JSON.parse(await readFile(join(dataRoot, file), 'utf8')) }; }
    catch (error) { return { name, fields, documents: [], error: `Could not read data/${file}: ${error.message}` }; }
  }));
  return { name: 'local_db', collections };
}

// Values must fit what the C engine's JSON reader accepts: strings under 128 bytes with no control characters.
function formatField({ field, type }, value) {
  if (type === 'STRING') {
    if (typeof value !== 'string' || !value.trim()) fail(400, `Enter a value for ${field}.`);
    if (/[\u0000-\u001f\u007f]/.test(value)) fail(400, `${field} cannot contain control characters.`);
    if (Buffer.byteLength(value.trim()) > 127) fail(400, `${field} must be under 128 bytes.`);
    return `${JSON.stringify(field)}: ${JSON.stringify(value.trim())}`;
  }
  if (typeof value !== 'number' || !Number.isFinite(value)) fail(400, `${field} must be a number.`);
  if (type === 'INT' && (!Number.isInteger(value) || Math.abs(value) > 2147483647)) fail(400, `${field} must be a whole number.`);
  if (Math.abs(value) >= 1e15) fail(400, `${field} is too large.`);
  return `${JSON.stringify(field)}: ${type === 'FLOAT' && Number.isInteger(value) ? value.toFixed(1) : value}`;
}

// Appends one line to the collection file, leaving the existing lines exactly as written.
async function insertDocument(dataRoot, name, input) {
  const fields = (await schemaCatalog()).filter(item => item.collection === name);
  if (!fields.length) fail(404, `Unknown collection '${name}'.`);
  if (!input || typeof input !== 'object' || Array.isArray(input)) fail(400, 'Send the new document as a JSON object.');
  const unknown = Object.keys(input).find(key => !fields.some(item => item.field === key));
  if (unknown) fail(400, `${name} has no field '${unknown}'.`);
  const line = `  { ${fields.map(item => formatField(item, input[item.field])).join(', ')} }`;
  const file = join(dataRoot, `${name}.json`);
  const text = await readFile(file, 'utf8').catch(() => fail(404, `data/${name}.json does not exist.`));
  let documents;
  try { documents = JSON.parse(text); } catch { documents = null; }
  if (!Array.isArray(documents)) fail(409, `data/${name}.json is not a valid JSON array. Fix the file before adding documents.`);
  if (documents.length >= maxDocuments) fail(409, `${name} already has ${maxDocuments} documents, the most the C engine loads.`);
  const eol = text.includes('\r\n') ? '\r\n' : '\n';
  await writeFile(file, documents.length ? text.replace(/\s*\]\s*$/, `,${eol}${line}${eol}]${eol}`) : `[${eol}${line}${eol}]${eol}`, 'utf8');
}

export function createDashboardServer({ dataRoot = defaultDataRoot } = {}) {
  let writes = Promise.resolve();
  return http.createServer(async (request, response) => {
    response.setHeader('X-Content-Type-Options', 'nosniff');
    response.setHeader('Referrer-Policy', 'no-referrer');
    response.setHeader('Content-Security-Policy', "default-src 'self'; script-src 'self'; style-src 'self'; img-src 'self' data:; connect-src 'self'; frame-ancestors 'none'; base-uri 'none'; form-action 'self'");
    try {
      const url = new URL(request.url, 'http://localhost');
      if (request.method === 'GET' && url.pathname === '/api/health') return json(response, 200, { status: 'ok', engine: 'SQL2Mongo C compiler', storage: 'JSON collection files in data/', phases: phaseDefinitions });
      if (request.method === 'GET' && url.pathname === '/api/database') return json(response, 200, await readDatabase(dataRoot));
      const insert = url.pathname.match(/^\/api\/database\/(\w+)$/);
      if (request.method === 'POST' && (insert || url.pathname === '/api/compile')) {
        const rejected = postError(request);
        if (rejected) return json(response, rejected[0], { error: rejected[1] });
        const body = await readBody(request);
        if (!insert) return json(response, 200, await compile(body?.sql));
        // One write at a time, so two quick saves cannot overwrite each other.
        const task = writes.then(() => insertDocument(dataRoot, insert[1], body?.document));
        writes = task.catch(() => {});
        await task;
        return json(response, 201, await readDatabase(dataRoot));
      }
      if (request.method !== 'GET' && request.method !== 'HEAD') return json(response, 405, { error: 'Method not allowed.' });
      const asset = assets.get(url.pathname);
      if (!asset) return json(response, 404, { error: 'Not found.' });
      const content = await readFile(join(publicRoot, asset[0]));
      response.writeHead(200, { 'Content-Type': asset[1], 'Cache-Control': 'no-cache' });
      response.end(request.method === 'HEAD' ? undefined : content);
    } catch (error) {
      json(response, error.status ?? 500, { error: error.status ? error.message : 'The dashboard could not complete this request.' });
    }
  });
}

if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
  const port = Number(process.env.PORT || 3000);
  const server = createDashboardServer();
  server.on('error', error => { console.error(error.code === 'EADDRINUSE' ? `Port ${port} is already in use. Set PORT to use another port.` : error.message); process.exitCode = 1; });
  server.listen(port, '127.0.0.1', () => console.log(`SQL2Mongo dashboard: http://127.0.0.1:${port}\nPress Ctrl+C to stop.`));
}
