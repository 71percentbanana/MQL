import { test } from 'node:test';
import assert from 'node:assert/strict';
import { once } from 'node:events';
import { cp, mkdtemp, readFile, rm } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { compile } from '../compiler.mjs';
import { createDashboardServer } from '../server.mjs';

test('actual compiler preserves projection, filtering, sorting, and limiting', async () => {
  const result = await compile('SELECT name, age, gpa FROM students WHERE age >= 20 ORDER BY gpa DESC LIMIT 3;');
  assert.equal(result.success, true);
  assert.deepEqual(result.phases.map(phase => phase.status), Array(7).fill('complete'));
  assert.deepEqual(result.documents, [
    { name: 'Diana Prince', age: 23, gpa: 3.95 },
    { name: 'Alice Walker', age: 22, gpa: 3.85 },
    { name: 'Bob Smith', age: 21, gpa: 3.4 },
  ]);
  assert.equal(result.tokens.length, 19);
  assert.equal(result.schema.length, 10);
  assert.equal(result.evaluated, 5);
  assert.equal(result.returned, 3);
  assert.deepEqual(result.pipeline.map(stage => Object.keys(stage)[0]), ['$match', '$sort', '$limit', '$project']);
  assert.equal(result.phases[1].parts.length, 2);
  assert.match(result.phases[3].parts[0].output, /Triple Representation/);
  assert.doesNotMatch(result.phases[6].parts[0].output, /FINAL VERIFICATION/);
});

test('employee queries execute against the engine collection', async () => {
  const result = await compile("SELECT name, salary FROM employees WHERE department = 'Engineering' ORDER BY salary DESC;");
  assert.equal(result.collection, 'employees');
  assert.deepEqual(result.documents, [{ name: 'Sarah Connor', salary: 95000 }, { name: 'John Doe', salary: 72000 }]);
});

test('nested AND / OR filters use the C parser and engine', async () => {
  const result = await compile("SELECT name FROM students WHERE (city = 'Seattle' OR city = 'Boston') AND gpa >= 3.5 ORDER BY gpa DESC;");
  assert.equal(result.success, true);
  assert.deepEqual(result.documents, [{ name: 'Charlie Brown' }, { name: 'Alice Walker' }]);
  assert.ok(result.pipeline[0].$match.$and);
});

test('a successful query returning zero documents stays successful', async () => {
  const result = await compile('SELECT name FROM students WHERE age > 100;');
  assert.equal(result.success, true);
  assert.equal(result.returned, 0);
  assert.deepEqual(result.documents, []);
});

test('syntax errors stop the parser and skip later phases', async () => {
  const result = await compile('SELECT name students;');
  assert.equal(result.success, false);
  assert.match(result.error, /Syntax error/);
  assert.deepEqual(result.phases.map(phase => phase.status), ['complete', 'error', 'skipped', 'skipped', 'skipped', 'skipped', 'skipped']);
  assert.equal(result.mongo, '');
});

test('lexical errors are assigned to the scanner', async () => {
  const result = await compile('SELECT @ FROM students;');
  assert.equal(result.success, false);
  assert.match(result.error, /Lexical error/);
  assert.equal(result.phases[0].status, 'error');
  assert.ok(result.phases.slice(1).every(phase => phase.status === 'skipped'));
});

test('semantic diagnostics cover types, fields, and collections', async () => {
  for (const sql of ["SELECT name FROM students WHERE age = 'twenty';", 'SELECT missing FROM students;', 'SELECT * FROM missing;']) {
    const result = await compile(sql);
    assert.equal(result.success, false);
    assert.match(result.error, /Semantic/);
    assert.equal(result.phases[2].status, 'error');
    assert.equal(result.phases[3].status, 'skipped');
    assert.equal(result.documents.length, 0);
  }
});

test('input is SQL text, never a user-selected local file', async () => {
  const result = await compile('src/main.c');
  assert.equal(result.success, false);
  assert.doesNotMatch(result.raw, /#include/);
  await assert.rejects(compile(''), /Enter a SQL/);
  await assert.rejects(compile('x'.repeat(4001)), /4,000/);
  await assert.rejects(compile('SELECT\0'), /null characters/);
});

test('HTTP API serves the dashboard, real database records, and query errors', async t => {
  const server = createDashboardServer();
  server.listen(0, '127.0.0.1');
  await once(server, 'listening');
  t.after(() => new Promise(resolve => server.close(resolve)));
  const base = `http://127.0.0.1:${server.address().port}`;
  const page = await fetch(base);
  assert.equal(page.status, 200);
  assert.match(await page.text(), /<title>SQL2Mongo<\/title>/);
  assert.match(page.headers.get('content-security-policy'), /frame-ancestors 'none'/);
  for (const file of ['/app.js', '/styles.css', '/favicon.svg']) assert.equal((await fetch(base + file)).status, 200);
  const database = await (await fetch(base + '/api/database')).json();
  assert.deepEqual(database.collections.map(collection => [collection.name, collection.documents.length]), [['students', 5], ['employees', 5]]);
  assert.deepEqual(database.collections[0].documents, JSON.parse(await readFile(new URL('../../data/students.json', import.meta.url), 'utf8')));
  const query = await fetch(base + '/api/compile', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ sql: 'SELECT * FROM students;' }) });
  assert.equal(query.status, 200);
  assert.equal((await query.json()).returned, 5);
  const invalid = await fetch(base + '/api/compile', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: '{oops' });
  assert.equal(invalid.status, 400);
  const crossOrigin = await fetch(base + '/api/compile', { method: 'POST', headers: { 'Content-Type': 'application/json', Origin: 'https://example.com' }, body: JSON.stringify({ sql: 'SELECT * FROM students;' }) });
  assert.equal(crossOrigin.status, 403);
  assert.equal((await fetch(base + '/api/compile', { method: 'POST', body: 'x' })).status, 415);
  assert.equal((await fetch(base + '/src/main.c')).status, 404);
});

test('adding a document validates it against the schema and appends it to the collection file', async t => {
  const dataRoot = await mkdtemp(join(tmpdir(), 'sql2mongo-data-'));
  await cp(new URL('../../data', import.meta.url), dataRoot, { recursive: true });
  const server = createDashboardServer({ dataRoot });
  server.listen(0, '127.0.0.1');
  await once(server, 'listening');
  t.after(async () => { await new Promise(resolve => server.close(resolve)); await rm(dataRoot, { recursive: true, force: true }); });
  const base = `http://127.0.0.1:${server.address().port}`;
  const post = (collection, document, headers = {}) => fetch(`${base}/api/database/${collection}`, { method: 'POST', headers: { 'Content-Type': 'application/json', ...headers }, body: JSON.stringify({ document }) });
  const before = await readFile(join(dataRoot, 'employees.json'), 'utf8');

  const database = await (await fetch(base + '/api/database')).json();
  assert.deepEqual(database.collections[1].fields, [
    { name: 'name', type: 'STRING' }, { name: 'department', type: 'STRING' }, { name: 'salary', type: 'FLOAT' },
    { name: 'experience', type: 'INT' }, { name: 'role', type: 'STRING' },
  ]);

  const added = await post('employees', { name: 'Pam Beesly', department: 'Sales', salary: 52000, experience: 2, role: 'Receptionist' });
  assert.equal(added.status, 201);
  assert.equal((await added.json()).collections[1].documents.length, 6);
  const after = await readFile(join(dataRoot, 'employees.json'), 'utf8');
  assert.ok(after.startsWith(before.trimEnd().slice(0, -1).trimEnd()), 'existing lines are left as written');
  assert.match(after, /\{ "name": "Pam Beesly", "department": "Sales", "salary": 52000\.0, "experience": 2, "role": "Receptionist" \}\n\]\n$/);
  assert.equal(JSON.parse(after).length, 6);

  const valid = { name: 'X', age: 20, gpa: 3.5, city: 'Austin', status: 'Active' };
  for (const [collection, document, status] of [
    ['students', { ...valid, age: 20.5 }, 400],
    ['students', { ...valid, gpa: 'high' }, 400],
    ['students', { ...valid, city: '' }, 400],
    ['students', { ...valid, city: 'x'.repeat(128) }, 400],
    ['students', { ...valid, city: 'A\tB' }, 400],
    ['students', { ...valid, email: 'x@example.com' }, 400],
    ['students', [valid], 400],
    ['missing', valid, 404],
  ]) assert.equal((await post(collection, document)).status, status, JSON.stringify(document));
  assert.equal((await post('students', valid, { Origin: 'https://example.com' })).status, 403);
  assert.equal(JSON.parse(await readFile(join(dataRoot, 'students.json'), 'utf8')).length, 5, 'rejected documents are never written');
});
