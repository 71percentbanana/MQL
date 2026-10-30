const $ = selector => document.querySelector(selector);
const esc = value => String(value ?? '').replace(/[&<>"']/g, char => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[char]));
const examples = {
  top: 'SELECT name, age, gpa\nFROM students\nWHERE age >= 20\nORDER BY gpa DESC\nLIMIT 3;',
  engineering: "SELECT name, salary, experience\nFROM employees\nWHERE department = 'Engineering'\nORDER BY salary DESC;",
  logical: "SELECT name, city, gpa\nFROM students\nWHERE (city = 'Seattle' OR city = 'Boston')\n  AND gpa >= 3.5\nORDER BY gpa DESC;",
  all: 'SELECT *\nFROM students;',
  empty: 'SELECT name, age\nFROM students\nWHERE age > 100;',
  semantic: "SELECT name, age\nFROM students\nWHERE age = 'twenty';",
  syntax: 'SELECT name, age\nstudents\nWHERE age >= 20;',
};
const statusMark = { complete: '✓', error: '✗', skipped: '–', pending: '' };
const mongoType = { INT: 'int', FLOAT: 'double', STRING: 'string' };

let phases = [], result = null, schema = [], activePhase = 1, activePart = 0, busy = false;
let database = null, activeCollection = 0, adding = false;
const format = { results: 'json', database: 'json' };

async function api(path, options) {
  const response = await fetch(path, options);
  const payload = await response.json();
  if (!response.ok) throw new Error(payload.error || 'Request failed.');
  return payload;
}

// Escape every piece of source text, wrapping recognised tokens in a span.
function highlighter(pattern, classify) {
  return text => {
    let html = '', last = 0;
    for (const match of text.matchAll(pattern)) {
      const type = classify(match, text.slice(match.index + match[0].length));
      html += esc(text.slice(last, match.index)) + (type ? `<span class="t-${type}">${esc(match[0])}</span>` : esc(match[0]));
      last = match.index + match[0].length;
    }
    return html + esc(text.slice(last));
  };
}

const highlightJson = highlighter(/"(?:\\.|[^"\\])*"|-?\b\d+(?:\.\d+)?(?:[eE][+-]?\d+)?\b|\b(?:true|false|null|db|aggregate)\b/g, ([token], rest) => {
  if (token.startsWith('"')) return /^\s*:/.test(rest) ? (token.startsWith('"$') ? 'op' : 'key') : 'str';
  if (/^-?\d/.test(token)) return 'num';
  return /^(db|aggregate)$/.test(token) ? 'fn' : 'kw';
});

const highlightSql = highlighter(/'(?:''|[^'])*'?|\b\d+(?:\.\d+)?\b|\b[A-Za-z_]\w*\b|>=|<=|!=|<>|[=<>*]/g, ([token]) => {
  if (token.startsWith("'")) return 'str';
  if (/^\d/.test(token)) return 'num';
  if (/^(select|from|where|and|or|not|order|by|asc|desc|limit)$/i.test(token)) return 'kw';
  return /^[=<>!*]/.test(token) ? 'op' : '';
});

function documents(docs) {
  if (!docs.length) return '<p class="empty">No documents.</p>';
  return `<div class="docs">${docs.map(doc => `<pre class="doc">${highlightJson(JSON.stringify(doc, null, 2))}</pre>`).join('')}</div>`;
}

function table(docs) {
  if (!docs.length) return '<p class="empty">No documents.</p>';
  const fields = [...new Set(docs.flatMap(doc => Object.keys(doc)))];
  return `<div class="scroll-x"><table><thead><tr>${fields.map(field => `<th>${esc(field)}</th>`).join('')}</tr></thead><tbody>${docs.map(doc => `<tr>${fields.map(field => `<td${typeof doc[field] === 'number' ? ' class="num"' : ''}>${esc(doc[field])}</td>`).join('')}</tr>`).join('')}</tbody></table></div>`;
}

const show = (view, docs) => format[view] === 'json' ? documents(docs) : table(docs);

function tabs(items, active, attribute) {
  return items.map((label, index) => `<button role="tab" aria-selected="${index === active}" ${attribute}="${index}">${label}</button>`).join('');
}

function partsFor(phase) {
  if (!phase?.parts?.length) return [];
  if (phase.id === 1 && result.tokens.length) return [{ name: 'Tokens', html: `<table><thead><tr><th>#</th><th>Type</th><th>Lexeme</th><th>Line</th><th>Col</th></tr></thead><tbody>${result.tokens.map(t => `<tr><td class="num">${t.index}</td><td>${esc(t.type)}</td><td><code>${esc(t.lexeme)}</code></td><td class="num">${t.line}</td><td class="num">${t.column}</td></tr>`).join('')}</tbody></table>` }, { name: 'Raw', text: phase.parts[0].output }];
  if (phase.id === 2) return phase.parts.map(part => ({ name: part.variant === 'a' ? 'Parse tree' : part.variant === 'b' ? 'AST' : 'Diagnostic', text: part.output }));
  if (phase.id === 3 && result.schema.length) return [{ name: 'Symbol table', html: `<table><thead><tr><th>Collection</th><th>Field</th><th>Type</th><th>Width</th></tr></thead><tbody>${result.schema.map(f => `<tr><td>${esc(f.collection)}</td><td><code>${esc(f.field)}</code></td><td>${esc(f.type)}</td><td class="num">${f.bytes} B</td></tr>`).join('')}</tbody></table>` }, { name: 'Raw', text: phase.parts[0].output }];
  if (phase.id === 4) {
    const text = phase.parts[0].output;
    const headings = [...text.matchAll(/^(Three-Address Code[^\n]*|Quadruple Representation[^\n]*|Triple Representation[^\n]*)/gm)];
    if (headings.length) return headings.map((match, index) => ({ name: ['TAC', 'Quadruples', 'Triples'][index], text: text.slice(match.index, headings[index + 1]?.index ?? text.length).trim() }));
  }
  return phase.parts.map(part => ({ name: phase.artifact, text: part.output }));
}

function renderPhases() {
  const items = result?.phases ?? phases.map(phase => ({ ...phase, status: 'pending' }));
  $('#phases').innerHTML = items.map(phase => `<button role="tab" class="${phase.status}" aria-selected="${phase.id === activePhase}" data-phase="${phase.id}"><span class="step">${phase.id}</span>${esc(phase.short)} <span class="mark">${statusMark[phase.status]}</span></button>`).join('');
  const failed = result?.phases?.find(phase => phase.status === 'error');
  $('#phase-summary').textContent = !result?.phases?.length ? '' : failed ? `Stopped at phase ${failed.id}` : `${result.phases.filter(phase => phase.status === 'complete').length} of 7 passed`;
  $('#nav-phases').textContent = failed ? '✗' : '';
  $('#phase-sql').hidden = !result?.sql;
  $('#phase-sql').innerHTML = result?.sql ? highlightSql(result.sql.trim()) : '';
  const phase = items.find(item => item.id === activePhase);
  $('#phase-description').textContent = phase ? `${phase.name}: ${phase.description}` : '';
  const parts = result?.phases ? partsFor(phase) : [];
  activePart = Math.min(activePart, Math.max(parts.length - 1, 0));
  $('#parts').innerHTML = parts.length > 1 ? tabs(parts.map(part => esc(part.name)), activePart, 'data-part') : '';
  const part = parts[activePart];
  const diagnostic = phase?.status === 'error' && result.error ? `<p class="error">${esc(result.error)}</p>` : '';
  $('#phase-output').innerHTML = diagnostic + (!result ? '<p class="empty">Run a query on the <a href="#query">Query</a> page to see each phase.</p>' : !part ? '<p class="empty">Not reached — an earlier phase failed.</p>' : part.html ?? `<pre class="code">${esc(part.text)}</pre>`);
}

function renderResults() {
  document.querySelectorAll('[data-view="results"]').forEach(button => button.setAttribute('aria-pressed', String(button.dataset.format === format.results)));
  $('#summary').textContent = result?.success ? `${result.returned} of ${result.evaluated} documents from ${result.collection} · ${result.durationMs} ms` : '';
  $('#results').innerHTML = busy ? '<p class="empty">Running…</p>'
    : !result ? '<p class="empty">No query run yet.</p>'
    : !result.success ? '<p class="empty">No results. The query failed to compile — open <a href="#phases">Compiler phases</a> to see where it stopped.</p>'
    : show('results', result.documents);
}

function render() {
  $('#mongo').innerHTML = result?.mongo ? highlightJson(result.mongo) : `<span class="muted">${busy ? '' : result ? '// No pipeline generated.' : '// Run a query to generate the pipeline.'}</span>`;
  $('#pipeline-info').textContent = result?.mongo ? `${result.pipeline.length} stage${result.pipeline.length === 1 ? '' : 's'}` : '';
  $('#copy').disabled = !result?.mongo;
  $('#trace').disabled = !result?.raw;
  $('#error').hidden = !result?.error;
  $('#error').textContent = result?.error ?? '';
  renderResults();
  renderPhases();
}

async function run() {
  if (busy) return;
  const sql = $('#sql').value;
  if (!sql.trim()) return $('#sql').focus();
  busy = true; result = null;
  $('#run').disabled = true;
  render();
  try {
    result = await api('/api/compile', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ sql }) });
    activePhase = result.phases.find(phase => phase.status === 'error')?.id ?? activePhase;
    activePart = 0;
    if (result.schema.length) { schema = result.schema; if (database) renderDatabase(); }
    $('#status').textContent = 'Compiler ready';
  } catch (error) {
    result = { error: error.message, phases: phases.map(phase => ({ ...phase, status: 'skipped', parts: [] })), tokens: [], schema: [] };
    $('#status').textContent = 'Compiler unavailable';
  } finally {
    busy = false;
    $('#run').disabled = false;
    render();
  }
}

function fieldType(collection, field) {
  const declared = schema.find(item => item.collection === collection.name && item.field === field);
  if (declared) return mongoType[declared.type] ?? declared.type.toLowerCase();
  const value = collection.documents.find(doc => field in doc)?.[field];
  return typeof value === 'number' ? (Number.isInteger(value) ? 'int' : 'double') : typeof value;
}

function renderDatabase() {
  document.querySelectorAll('[data-view="database"]').forEach(button => button.setAttribute('aria-pressed', String(button.dataset.format === format.database)));
  const collection = database.collections[activeCollection];
  $('#collections').innerHTML = tabs(database.collections.map(item => `${esc(item.name)} <span class="count">${item.documents.length}</span>`), activeCollection, 'data-collection');
  const fields = [...new Set(collection.documents.flatMap(doc => Object.keys(doc)))];
  $('#collection-info').innerHTML = `<p class="muted small"><code>data/${esc(collection.name)}.json</code> · ${fields.map(field => `<span class="field">${esc(field)} <span class="type">${esc(fieldType(collection, field))}</span></span>`).join('')}</p>`;
  $('#collection').innerHTML = collection.error ? `<p class="error">${esc(collection.error)}</p>` : show('database', collection.documents);
  renderAddForm(collection);
}

// The form is rebuilt only when the collection changes, so typed values survive a JSON / Table switch.
function renderAddForm(collection) {
  const form = $('#add-form');
  $('#add-doc').disabled = !collection.fields?.length;
  $('#add-doc').title = collection.fields?.length ? '' : 'This collection is not in the compiler schema.';
  $('#add-doc').setAttribute('aria-expanded', String(adding));
  form.hidden = !adding;
  if (!adding) { delete form.dataset.collection; return; }
  if (form.dataset.collection === collection.name) return;
  form.dataset.collection = collection.name;
  form.innerHTML = `<div class="form-fields">${collection.fields.map(({ name, type }) => `<label>
      <span>${esc(name)} <span class="type">${esc(mongoType[type] ?? type)}</span></span>
      <input name="${esc(name)}" required ${type === 'STRING' ? 'type="text" maxlength="127" autocomplete="off"' : `type="number" step="${type === 'INT' ? 1 : 'any'}"`}>
    </label>`).join('')}</div>
    <div class="form-actions">
      <button type="submit" class="primary">Save to ${esc(collection.name)}.json</button>
      <button type="button" data-cancel>Cancel</button>
      <span class="form-status small" role="status"></span>
    </div>`;
}

function syncEditor() {
  // The trailing newline keeps the highlight layer as tall as the textarea.
  $('#sql-highlight').innerHTML = highlightSql($('#sql').value) + '\n';
  $('#sql-highlight').scrollTop = $('#sql').scrollTop;
  $('#sql-highlight').scrollLeft = $('#sql').scrollLeft;
}

function route() {
  const page = ['query', 'phases', 'database'].includes(location.hash.slice(1)) ? location.hash.slice(1) : 'query';
  document.querySelectorAll('.view').forEach(view => { view.hidden = view.id !== `view-${page}`; });
  document.querySelectorAll('nav a').forEach(link => {
    if (link.dataset.page === page) link.setAttribute('aria-current', 'page');
    else link.removeAttribute('aria-current');
  });
}

function download(filename, content) {
  const url = URL.createObjectURL(new Blob([content], { type: 'text/plain' }));
  const link = document.createElement('a');
  link.href = url; link.download = filename; link.click();
  setTimeout(() => URL.revokeObjectURL(url), 1000);
}

$('#run').addEventListener('click', run);
$('#examples').addEventListener('change', event => {
  if (!event.target.value) return;
  $('#sql').value = examples[event.target.value];
  event.target.value = '';
  syncEditor();
  run();
});
$('#sql').addEventListener('input', syncEditor);
$('#sql').addEventListener('scroll', syncEditor);
$('#sql').addEventListener('keydown', event => {
  if (event.key === 'Tab') { event.preventDefault(); event.target.setRangeText('  ', event.target.selectionStart, event.target.selectionEnd, 'end'); syncEditor(); }
});
document.addEventListener('keydown', event => {
  if ((event.ctrlKey || event.metaKey) && event.key === 'Enter') {
    event.preventDefault();
    if (location.hash === '#database') location.hash = 'query';
    run();
  }
});
window.addEventListener('hashchange', route);
document.querySelectorAll('[data-format]').forEach(button => button.addEventListener('click', () => {
  format[button.dataset.view] = button.dataset.format;
  if (button.dataset.view === 'results') renderResults();
  else if (database) renderDatabase();
}));
$('#phases').addEventListener('click', event => { const button = event.target.closest('[data-phase]'); if (button) { activePhase = Number(button.dataset.phase); activePart = 0; renderPhases(); } });
$('#parts').addEventListener('click', event => { const button = event.target.closest('[data-part]'); if (button) { activePart = Number(button.dataset.part); renderPhases(); } });
$('#collections').addEventListener('click', event => { const button = event.target.closest('[data-collection]'); if (button) { activeCollection = Number(button.dataset.collection); renderDatabase(); } });
$('#copy').addEventListener('click', async () => {
  try { await navigator.clipboard.writeText(result.mongo); $('#copy').textContent = 'Copied'; }
  catch { $('#copy').textContent = 'Copy failed'; }
  setTimeout(() => { $('#copy').textContent = 'Copy'; }, 1500);
});
$('#trace').addEventListener('click', () => { if (result?.raw) download('sql2mongo-trace.txt', result.raw); });
$('#add-doc').addEventListener('click', () => {
  adding = !adding;
  renderDatabase();
  if (adding) $('#add-form input')?.focus();
});
$('#add-form').addEventListener('click', event => { if (event.target.closest('[data-cancel]')) { adding = false; renderDatabase(); } });
$('#add-form').addEventListener('submit', async event => {
  event.preventDefault();
  const form = event.target, collection = database.collections[activeCollection], status = form.querySelector('.form-status');
  const doc = Object.fromEntries(collection.fields.map(({ name, type }) => [name, type === 'STRING' ? form.elements[name].value : Number(form.elements[name].value)]));
  form.querySelector('[type="submit"]').disabled = true;
  status.classList.remove('failed');
  status.textContent = 'Saving…';
  try {
    database = await api(`/api/database/${encodeURIComponent(collection.name)}`, { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ document: doc }) });
    renderDatabase();
    form.reset();
    status.textContent = `Added to data/${collection.name}.json`;
    form.querySelector('input')?.focus();
  } catch (error) {
    status.classList.add('failed');
    status.textContent = error.message;
  } finally {
    form.querySelector('[type="submit"]').disabled = false;
  }
});

function setTheme(theme) {
  if (theme === 'dark') document.documentElement.dataset.theme = 'dark';
  else delete document.documentElement.dataset.theme;
  document.querySelectorAll('[data-set-theme]').forEach(button => button.setAttribute('aria-pressed', String(button.dataset.setTheme === theme)));
}
setTheme(document.documentElement.dataset.theme === 'dark' ? 'dark' : 'light');
document.querySelectorAll('[data-set-theme]').forEach(button => button.addEventListener('click', () => {
  setTheme(button.dataset.setTheme);
  try { localStorage.setItem('theme', button.dataset.setTheme); } catch {}
}));

async function initialize() {
  route();
  syncEditor();
  try {
    phases = (await api('/api/health')).phases;
    $('#status').textContent = 'Compiler ready';
    render();
    database = await api('/api/database');
    renderDatabase();
    await run();
  } catch (error) {
    $('#status').textContent = 'Server unavailable';
    $('#error').hidden = false;
    $('#error').textContent = `${error.message} Start the dashboard with npm start.`;
  }
}
initialize();
