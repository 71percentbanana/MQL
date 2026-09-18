#ifndef SCHEMA_TABLE_H
#define SCHEMA_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "sql_ast.h"

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_UNKNOWN
} DataType;

typedef struct {
    char name[64];
    DataType type;
} FieldSymbol;

typedef struct {
    char collection_name[64];
    FieldSymbol fields[32];
    int field_count;
} CollectionSchema;

void schema_init(void);
const char* data_type_name(DataType type);
bool schema_collection_exists(const char* collection);
DataType schema_get_field_type(const char* collection, const char* field);
bool schema_validate_query(const ASTQuery* query, char* err_buf, size_t buf_sz);
void schema_print_catalog(void);

#endif // SCHEMA_TABLE_H
