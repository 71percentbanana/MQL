#ifndef MONGO_CODEGEN_H
#define MONGO_CODEGEN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "sql_ast.h"

void mongo_codegen_pipeline(const ASTQuery* query, char* out_buf, size_t buf_sz);
void mongo_print_code(const ASTQuery* query);

#endif // MONGO_CODEGEN_H
