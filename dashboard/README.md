# SQL2Mongo dashboard

A local web UI for the SQL2Mongo C compiler. It runs the real `sql2mongo` executable and shows its output; nothing is compiled in JavaScript.

## Start

Requires Node.js 20+. No npm dependencies.

```powershell
cd dashboard
npm start
```

Open http://127.0.0.1:3000. Write a query or pick an example, then press **Run** (or Ctrl+Enter). Set `$env:PORT` to use another port.

After changing anything in `src/`, rebuild the executable (needs GCC / MinGW on PATH):

```powershell
npm run build:compiler
```

## Pages

- **Query** — the SQL editor, the MongoDB aggregation pipeline the compiler generated, and the returned documents (JSON or table).
- **Compiler phases** — the output of each of the seven phases for the last query run: tokens, parse tree and AST, symbol table, TAC / quadruples / triples, optimizer report, MongoDB code, and execution. A failed phase is marked ✗ (also shown in the nav) and later phases are shown as not reached. **Download trace** saves the complete compiler output.
- **Database** — the documents in each collection file in `data/`, with field types from the symbol table.

## Data

Each collection is a JSON file in `data/` (`students.json`, `employees.json`) holding an array of flat documents, the same shape you would get from `mongoexport --jsonArray`. The C engine reads the file at execution time, so editing it changes query results immediately — no rebuild needed.

Field names and types used for semantic checking are declared in the symbol table in `src/schema_table.c`. If you add a field to the JSON, add it there too, or queries that reference it will be rejected.

The compiler generates MongoDB code but does not connect to a MongoDB server; execution happens in the C engine against the JSON files.

## Tests

```powershell
npm test
```

The tests run the real executable and cover projection, filtering, sorting, limits, AND / OR, empty results, lexical / syntax / semantic errors, and the HTTP API.

The server listens on `127.0.0.1` only, runs the compiler without a shell, limits query size and runtime, and rejects cross-origin requests. It is meant for local use.
