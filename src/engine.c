#include "engine.h"
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

#define MAX_DOCS 32
#define MAX_FIELDS 16

typedef struct {
    char key[64];
    DataType type;
    double num_val;
    char str_val[128];
} DocField;

typedef struct {
    DocField fields[MAX_FIELDS];
    int field_count;
} Document;

typedef struct {
    Document docs[MAX_DOCS];
    int doc_count;
} CollectionStore;

static void add_doc_field(Document* doc, const char* key, DataType type, double num_val, const char* str_val) {
    if (doc->field_count >= MAX_FIELDS) return;
    DocField* f = &doc->fields[doc->field_count++];
    strncpy(f->key, key, sizeof(f->key) - 1);
    f->key[sizeof(f->key) - 1] = '\0';
    f->type = type;
    f->num_val = num_val;
    if (str_val) {
        strncpy(f->str_val, str_val, sizeof(f->str_val) - 1);
        f->str_val[sizeof(f->str_val) - 1] = '\0';
    } else {
        f->str_val[0] = '\0';
    }
}

static void load_built_in_students(CollectionStore* store) {
    store->doc_count = 0;

    // Doc 1
    Document* d = &store->docs[store->doc_count++];
    d->field_count = 0;
    add_doc_field(d, "name", TYPE_STRING, 0, "Alice Walker");
    add_doc_field(d, "age", TYPE_INT, 22, NULL);
    add_doc_field(d, "gpa", TYPE_FLOAT, 3.85, NULL);
    add_doc_field(d, "city", TYPE_STRING, 0, "Seattle");
    add_doc_field(d, "status", TYPE_STRING, 0, "Active");

    // Doc 2
    d = &store->docs[store->doc_count++];
    d->field_count = 0;
    add_doc_field(d, "name", TYPE_STRING, 0, "Bob Smith");
    add_doc_field(d, "age", TYPE_INT, 21, NULL);
    add_doc_field(d, "gpa", TYPE_FLOAT, 3.40, NULL);
    add_doc_field(d, "city", TYPE_STRING, 0, "Boston");
    add_doc_field(d, "status", TYPE_STRING, 0, "Active");

    // Doc 3
    d = &store->docs[store->doc_count++];
    d->field_count = 0;
    add_doc_field(d, "name", TYPE_STRING, 0, "Charlie Brown");
    add_doc_field(d, "age", TYPE_INT, 19, NULL);
    add_doc_field(d, "gpa", TYPE_FLOAT, 3.90, NULL);
    add_doc_field(d, "city", TYPE_STRING, 0, "Seattle");
    add_doc_field(d, "status", TYPE_STRING, 0, "Probation");

    // Doc 4
    d = &store->docs[store->doc_count++];
    d->field_count = 0;
    add_doc_field(d, "name", TYPE_STRING, 0, "Diana Prince");
    add_doc_field(d, "age", TYPE_INT, 23, NULL);
    add_doc_field(d, "gpa", TYPE_FLOAT, 3.95, NULL);
    add_doc_field(d, "city", TYPE_STRING, 0, "New York");
    add_doc_field(d, "status", TYPE_STRING, 0, "Active");

    // Doc 5
    d = &store->docs[store->doc_count++];
    d->field_count = 0;
    add_doc_field(d, "name", TYPE_STRING, 0, "Evan Wright");
    add_doc_field(d, "age", TYPE_INT, 20, NULL);
    add_doc_field(d, "gpa", TYPE_FLOAT, 3.10, NULL);
    add_doc_field(d, "city", TYPE_STRING, 0, "Chicago");
    add_doc_field(d, "status", TYPE_STRING, 0, "Graduated");
}

static void load_built_in_employees(CollectionStore* store) {
    store->doc_count = 0;

    // Doc 1
    Document* d = &store->docs[store->doc_count++];
    d->field_count = 0;
    add_doc_field(d, "name", TYPE_STRING, 0, "Sarah Connor");
    add_doc_field(d, "department", TYPE_STRING, 0, "Engineering");
    add_doc_field(d, "salary", TYPE_FLOAT, 95000.0, NULL);
    add_doc_field(d, "experience", TYPE_INT, 6, NULL);
    add_doc_field(d, "role", TYPE_STRING, 0, "Lead Dev");

    // Doc 2
    d = &store->docs[store->doc_count++];
    d->field_count = 0;
    add_doc_field(d, "name", TYPE_STRING, 0, "John Doe");
    add_doc_field(d, "department", TYPE_STRING, 0, "Engineering");
    add_doc_field(d, "salary", TYPE_FLOAT, 72000.0, NULL);
    add_doc_field(d, "experience", TYPE_INT, 3, NULL);
    add_doc_field(d, "role", TYPE_STRING, 0, "Software Engineer");

    // Doc 3
    d = &store->docs[store->doc_count++];
    d->field_count = 0;
    add_doc_field(d, "name", TYPE_STRING, 0, "Jane Smith");
    add_doc_field(d, "department", TYPE_STRING, 0, "Marketing");
    add_doc_field(d, "salary", TYPE_FLOAT, 65000.0, NULL);
    add_doc_field(d, "experience", TYPE_INT, 4, NULL);
    add_doc_field(d, "role", TYPE_STRING, 0, "Marketing Lead");

    // Doc 4
    d = &store->docs[store->doc_count++];
    d->field_count = 0;
    add_doc_field(d, "name", TYPE_STRING, 0, "Michael Scott");
    add_doc_field(d, "department", TYPE_STRING, 0, "Management");
    add_doc_field(d, "salary", TYPE_FLOAT, 85000.0, NULL);
    add_doc_field(d, "experience", TYPE_INT, 10, NULL);
    add_doc_field(d, "role", TYPE_STRING, 0, "Regional Manager");

    // Doc 5
    d = &store->docs[store->doc_count++];
    d->field_count = 0;
    add_doc_field(d, "name", TYPE_STRING, 0, "Dwight Schrute");
    add_doc_field(d, "department", TYPE_STRING, 0, "Sales");
    add_doc_field(d, "salary", TYPE_FLOAT, 58000.0, NULL);
    add_doc_field(d, "experience", TYPE_INT, 5, NULL);
    add_doc_field(d, "role", TYPE_STRING, 0, "Assistant Regional Manager");
}

static const DocField* find_field(const Document* doc, const char* key) {
    for (int i = 0; i < doc->field_count; i++) {
        if (ci_equals(doc->fields[i].key, key)) {
            return &doc->fields[i];
        }
    }
    return NULL;
}

static bool eval_binary_filter(const Document* doc, const ASTFilter* f) {
    const DocField* field = find_field(doc, f->field);
    if (!field) return false;

    if (field->type == TYPE_STRING) {
        int cmp = strcmp(field->str_val, f->literal.str_val);
        switch (f->comp_op) {
            case OP_EQ:  return cmp == 0;
            case OP_NEQ: return cmp != 0;
            case OP_GT:  return cmp > 0;
            case OP_GTE: return cmp >= 0;
            case OP_LT:  return cmp < 0;
            case OP_LTE: return cmp <= 0;
        }
    } else {
        double val = field->num_val;
        double target = f->literal.num_val;
        switch (f->comp_op) {
            case OP_EQ:  return val == target;
            case OP_NEQ: return val != target;
            case OP_GT:  return val > target;
            case OP_GTE: return val >= target;
            case OP_LT:  return val < target;
            case OP_LTE: return val <= target;
        }
    }
    return false;
}

static bool eval_filter(const Document* doc, const ASTFilter* f) {
    if (!f) return true;
    if (f->type == FILTER_BINARY) {
        return eval_binary_filter(doc, f);
    } else if (f->type == FILTER_LOGICAL) {
        bool left = eval_filter(doc, f->left);
        bool right = eval_filter(doc, f->right);
        return (f->logical_op == LOGIC_AND) ? (left && right) : (left || right);
    }
    return true;
}

// Global context for sort comparator
static char g_sort_field[64] = "";
static bool g_sort_desc = false;

static int doc_sort_compare(const void* a, const void* b) {
    const Document* da = (const Document*)a;
    const Document* db = (const Document*)b;
    const DocField* fa = find_field(da, g_sort_field);
    const DocField* fb = find_field(db, g_sort_field);

    if (!fa || !fb) return 0;

    int res = 0;
    if (fa->type == TYPE_STRING) {
        res = strcmp(fa->str_val, fb->str_val);
    } else {
        if (fa->num_val < fb->num_val) res = -1;
        else if (fa->num_val > fb->num_val) res = 1;
        else res = 0;
    }

    return g_sort_desc ? -res : res;
}

bool engine_execute_query(const ASTQuery* query, const char* data_dir) {
    (void)data_dir;
    CollectionStore store;
    if (ci_equals(query->collection, "students")) {
        load_built_in_students(&store);
    } else if (ci_equals(query->collection, "employees")) {
        load_built_in_employees(&store);
    } else {
        fprintf(stderr, "Execution Error: Unknown collection '%s'\n", query->collection);
        return false;
    }

    // Filter matching documents
    Document matched[MAX_DOCS];
    int match_count = 0;

    for (int i = 0; i < store.doc_count; i++) {
        if (eval_filter(&store.docs[i], query->where_clause)) {
            matched[match_count++] = store.docs[i];
        }
    }

    // Sort matching documents
    if (query->order_by.has_order && match_count > 1) {
        strncpy(g_sort_field, query->order_by.field, sizeof(g_sort_field) - 1);
        g_sort_field[sizeof(g_sort_field) - 1] = '\0';
        g_sort_desc = query->order_by.is_desc;
        qsort(matched, match_count, sizeof(Document), doc_sort_compare);
    }

    // Limit count
    int final_count = match_count;
    if (query->limit.has_limit && query->limit.limit_count < final_count) {
        final_count = query->limit.limit_count;
    }

    printf("\nExecution Engine: MongoDB Document Query Results:\n");
    printf("======================================================================\n");
    printf("Database: local_db | Collection: %s | Evaluated: %d | Returned: %d\n\n",
           query->collection, store.doc_count, final_count);

    if (final_count == 0) {
        printf("/* 0 documents found matching criteria */\n");
    }

    for (int i = 0; i < final_count; i++) {
        printf("{\n");
        const Document* doc = &matched[i];
        bool has_prev = false;

        if (query->projection.is_wildcard) {
            for (int j = 0; j < doc->field_count; j++) {
                if (has_prev) printf(",\n");
                const DocField* f = &doc->fields[j];
                if (f->type == TYPE_STRING) {
                    printf("  \"%s\": \"%s\"", f->key, f->str_val);
                } else if (f->type == TYPE_INT) {
                    printf("  \"%s\": %d", f->key, (int)f->num_val);
                } else {
                    printf("  \"%s\": %.2f", f->key, f->num_val);
                }
                has_prev = true;
            }
        } else {
            for (int p = 0; p < query->projection.field_count; p++) {
                const DocField* f = find_field(doc, query->projection.fields[p]);
                if (f) {
                    if (has_prev) printf(",\n");
                    if (f->type == TYPE_STRING) {
                        printf("  \"%s\": \"%s\"", f->key, f->str_val);
                    } else if (f->type == TYPE_INT) {
                        printf("  \"%s\": %d", f->key, (int)f->num_val);
                    } else {
                        printf("  \"%s\": %.2f", f->key, f->num_val);
                    }
                    has_prev = true;
                }
            }
        }
        printf("\n}\n");
    }

    printf("======================================================================\n");
    printf("Status: Query Executed Successfully (0 errors)\n");
    return true;
}
