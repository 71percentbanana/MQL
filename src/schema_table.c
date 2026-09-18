#include "schema_table.h"
#include <ctype.h>

static bool ci_equals(const char* a, const char* b) {
    if (!a || !b) return false;
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return false;
        a++;
        b++;
    }
    return *a == *b;
}

#define MAX_COLLECTIONS 8
static CollectionSchema g_catalog[MAX_COLLECTIONS];
static int g_catalog_count = 0;

const char* data_type_name(DataType type) {
    switch (type) {
        case TYPE_INT:    return "INT";
        case TYPE_FLOAT:  return "FLOAT";
        case TYPE_STRING: return "STRING";
        default:          return "UNKNOWN";
    }
}

static void add_collection(const char* name) {
    if (g_catalog_count >= MAX_COLLECTIONS) return;
    CollectionSchema* c = &g_catalog[g_catalog_count++];
    strncpy(c->collection_name, name, sizeof(c->collection_name) - 1);
    c->collection_name[sizeof(c->collection_name) - 1] = '\0';
    c->field_count = 0;
}

static void add_field(const char* collection, const char* field, DataType type) {
    for (int i = 0; i < g_catalog_count; i++) {
        if (ci_equals(g_catalog[i].collection_name, collection)) {
            CollectionSchema* c = &g_catalog[i];
            if (c->field_count < 32) {
                FieldSymbol* f = &c->fields[c->field_count++];
                strncpy(f->name, field, sizeof(f->name) - 1);
                f->name[sizeof(f->name) - 1] = '\0';
                f->type = type;
            }
            return;
        }
    }
}

void schema_init(void) {
    g_catalog_count = 0;

    // Collection 1: students
    add_collection("students");
    add_field("students", "name", TYPE_STRING);
    add_field("students", "age", TYPE_INT);
    add_field("students", "gpa", TYPE_FLOAT);
    add_field("students", "city", TYPE_STRING);
    add_field("students", "status", TYPE_STRING);

    // Collection 2: employees
    add_collection("employees");
    add_field("employees", "name", TYPE_STRING);
    add_field("employees", "department", TYPE_STRING);
    add_field("employees", "salary", TYPE_FLOAT);
    add_field("employees", "experience", TYPE_INT);
    add_field("employees", "role", TYPE_STRING);
}

bool schema_collection_exists(const char* collection) {
    for (int i = 0; i < g_catalog_count; i++) {
        if (ci_equals(g_catalog[i].collection_name, collection)) {
            return true;
        }
    }
    return false;
}

DataType schema_get_field_type(const char* collection, const char* field) {
    for (int i = 0; i < g_catalog_count; i++) {
        if (ci_equals(g_catalog[i].collection_name, collection)) {
            for (int j = 0; j < g_catalog[i].field_count; j++) {
                if (ci_equals(g_catalog[i].fields[j].name, field)) {
                    return g_catalog[i].fields[j].type;
                }
            }
        }
    }
    return TYPE_UNKNOWN;
}

static bool validate_filter(const char* collection, const ASTFilter* f, char* err_buf, size_t buf_sz) {
    if (!f) return true;

    if (f->type == FILTER_BINARY) {
        DataType field_type = schema_get_field_type(collection, f->field);
        if (field_type == TYPE_UNKNOWN) {
            snprintf(err_buf, buf_sz,
                     "Semantic Error: Field '%s' does not exist in collection '%s'",
                     f->field, collection);
            return false;
        }

        // Type checking
        if (field_type == TYPE_INT || field_type == TYPE_FLOAT) {
            if (f->literal.type == LIT_STRING) {
                snprintf(err_buf, buf_sz,
                         "Semantic Type Error: Cannot compare numeric field '%s' with string literal '%s'",
                         f->field, f->literal.str_val);
                return false;
            }
        } else if (field_type == TYPE_STRING) {
            if (f->literal.type != LIT_STRING) {
                snprintf(err_buf, buf_sz,
                         "Semantic Type Error: Cannot compare string field '%s' with numeric literal",
                         f->field);
                return false;
            }
        }
        return true;
    } else if (f->type == FILTER_LOGICAL) {
        if (!validate_filter(collection, f->left, err_buf, buf_sz)) return false;
        return validate_filter(collection, f->right, err_buf, buf_sz);
    }

    return true;
}

bool schema_validate_query(const ASTQuery* query, char* err_buf, size_t buf_sz) {
    // 1. Verify collection
    if (!schema_collection_exists(query->collection)) {
        snprintf(err_buf, buf_sz,
                 "Semantic Error: Collection '%s' does not exist in schema catalog",
                 query->collection);
        return false;
    }

    // 2. Verify projected columns
    if (!query->projection.is_wildcard) {
        for (int i = 0; i < query->projection.field_count; i++) {
            const char* field = query->projection.fields[i];
            if (schema_get_field_type(query->collection, field) == TYPE_UNKNOWN) {
                snprintf(err_buf, buf_sz,
                         "Semantic Error: Column '%s' in SELECT clause is not defined in collection '%s'",
                         field, query->collection);
                return false;
            }
        }
    }

    // 3. Verify WHERE filter
    if (query->where_clause) {
        if (!validate_filter(query->collection, query->where_clause, err_buf, buf_sz)) {
            return false;
        }
    }

    // 4. Verify ORDER BY column
    if (query->order_by.has_order) {
        if (schema_get_field_type(query->collection, query->order_by.field) == TYPE_UNKNOWN) {
            snprintf(err_buf, buf_sz,
                     "Semantic Error: Field '%s' in ORDER BY does not exist in collection '%s'",
                     query->order_by.field, query->collection);
            return false;
        }
    }

    return true;
}

void schema_print_catalog(void) {
    printf("\nSymbol Table (Schema Catalog & Type Registry):\n");
    printf("======================================================================\n");
    printf("+----------------------+----------------------+----------------------+\n");
    printf("| Collection Name      | Field Name           | Data Type            |\n");
    printf("+----------------------+----------------------+----------------------+\n");
    for (int i = 0; i < g_catalog_count; i++) {
        for (int j = 0; j < g_catalog[i].field_count; j++) {
            printf("| %-20s | %-20s | %-20s |\n",
                   g_catalog[i].collection_name,
                   g_catalog[i].fields[j].name,
                   data_type_name(g_catalog[i].fields[j].type));
        }
    }
    printf("+----------------------+----------------------+----------------------+\n");
    printf("Total Collections: %d | Status: Catalog Verified\n", g_catalog_count);
}
