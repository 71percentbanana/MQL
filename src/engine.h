#ifndef ENGINE_H
#define ENGINE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "sql_ast.h"
#include "schema_table.h"

// Executes the compiled AST query against a JSON dataset and prints MongoDB document results
bool engine_execute_query(const ASTQuery* query, const char* data_dir);

#endif // ENGINE_H
