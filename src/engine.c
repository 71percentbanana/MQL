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

// ---------------------------------------------------------------------------
// Minimal JSON reader for a collection file: an array of flat documents, e.g.
//   [ { "name": "Alice", "age": 22, "gpa": 3.85 }, ... ]
// Values may be strings or numbers (true/false/null are skipped).
// ---------------------------------------------------------------------------

typedef struct {
    const char* p;
    char err[160];
} JsonReader;

static void json_skip_ws(JsonReader* r) {
    while (*r->p && isspace((unsigned char)*r->p)) r->p++;
}

static bool json_expect(JsonReader* r, char c) {
    json_skip_ws(r);
    if (*r->p != c) {
        snprintf(r->err, sizeof(r->err), "expected '%c' but found '%.10s'", c, *r->p ? r->p : "end of file");
        return false;
    }
    r->p++;
    return true;
}

static bool json_read_string(JsonReader* r, char* out, size_t out_sz) {
    if (!json_expect(r, '"')) return false;
    size_t n = 0;
    while (*r->p && *r->p != '"') {
        char c = *r->p++;
        if (c == '\\') {
            char e = *r->p++;
            switch (e) {
                case 'n': c = '\n'; break;
                case 't': c = '\t'; break;
                case 'r': c = '\r'; break;
                case '"': case '\\': case '/': c = e; break;
                default:
                    snprintf(r->err, sizeof(r->err), "unsupported escape '\\%c' in string", e ? e : ' ');
                    return false;
            }
        }
        if (n + 1 < out_sz) out[n++] = c;
    }
    if (*r->p != '"') {
        snprintf(r->err, sizeof(r->err), "unterminated string");
        return false;
    }
    r->p++;
    out[n] = '\0';
    return true;
}

static bool json_read_document(JsonReader* r, const char* collection, Document* doc) {
    doc->field_count = 0;
    if (!json_expect(r, '{')) return false;
    json_skip_ws(r);
    if (*r->p == '}') { r->p++; return true; }

    for (;;) {
        char key[64];
        if (!json_read_string(r, key, sizeof(key))) return false;
        if (!json_expect(r, ':')) return false;
        json_skip_ws(r);

        if (*r->p == '"') {
            char value[128];
            if (!json_read_string(r, value, sizeof(value))) return false;
            add_doc_field(doc, key, TYPE_STRING, 0, value);
        } else if (*r->p == '-' || isdigit((unsigned char)*r->p)) {
            char* end;
            double num = strtod(r->p, &end);
            bool has_fraction = false;
            for (const char* q = r->p; q < end; q++) {
                if (*q == '.' || *q == 'e' || *q == 'E') has_fraction = true;
            }
            r->p = end;
            // Prefer the declared schema type; fall back to the literal's shape.
            DataType type = schema_get_field_type(collection, key);
            if (type != TYPE_INT && type != TYPE_FLOAT) type = has_fraction ? TYPE_FLOAT : TYPE_INT;
            add_doc_field(doc, key, type, num, NULL);
        } else if (strncmp(r->p, "true", 4) == 0 || strncmp(r->p, "null", 4) == 0) {
            r->p += 4;
        } else if (strncmp(r->p, "false", 5) == 0) {
            r->p += 5;
        } else {
            snprintf(r->err, sizeof(r->err), "unsupported value for field '%s'", key);
            return false;
        }

        json_skip_ws(r);
        if (*r->p == ',') { r->p++; continue; }
        return json_expect(r, '}');
    }
}

static bool load_collection(CollectionStore* store, const char* data_dir, const char* collection) {
    store->doc_count = 0;

    char lower[64];
    size_t i = 0;
    for (; collection[i] && i + 1 < sizeof(lower); i++) lower[i] = (char)tolower((unsigned char)collection[i]);
    lower[i] = '\0';

    char path[512];
    snprintf(path, sizeof(path), "%s/%s.json", data_dir, lower);

    FILE* f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "Execution Error: Cannot open collection file '%s'\n", path);
        return false;
    }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* text = (char*)malloc(sz + 1);
    if (!text) {
        fclose(f);
        return false;
    }
    size_t read_bytes = fread(text, 1, sz, f);
    text[read_bytes] = '\0';
    fclose(f);

    JsonReader r = { text, "" };
    bool ok = json_expect(&r, '[');
    json_skip_ws(&r);
    if (ok && *r.p == ']') {
        r.p++;
    } else {
        while (ok) {
            if (store->doc_count >= MAX_DOCS) {
                snprintf(r.err, sizeof(r.err), "more than %d documents", MAX_DOCS);
                ok = false;
                break;
            }
            ok = json_read_document(&r, collection, &store->docs[store->doc_count++]);
            json_skip_ws(&r);
            if (ok && *r.p == ',') { r.p++; continue; }
            if (ok) ok = json_expect(&r, ']');
            break;
        }
    }

    if (!ok) fprintf(stderr, "Execution Error: Invalid JSON in '%s': %s\n", path, r.err);
    free(text);
    return ok;
}

static void print_json_string(const char* s) {
    putchar('"');
    for (; *s; s++) {
        switch (*s) {
            case '"':  printf("\\\""); break;
            case '\\': printf("\\\\"); break;
            case '\n': printf("\\n"); break;
            case '\t': printf("\\t"); break;
            case '\r': printf("\\r"); break;
            default:   putchar(*s);
        }
    }
    putchar('"');
}

static void print_doc_field(const DocField* f) {
    printf("  ");
    print_json_string(f->key);
    printf(": ");
    if (f->type == TYPE_STRING) {
        print_json_string(f->str_val);
    } else if (f->type == TYPE_INT) {
        printf("%d", (int)f->num_val);
    } else {
        printf("%.2f", f->num_val);
    }
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
    static CollectionStore store;
    if (!load_collection(&store, data_dir, query->collection)) {
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
    printf("Database: local_db | Collection: %s | Evaluated: %d | Returned: %d\n",
           query->collection, store.doc_count, final_count);
    printf("Source: %s/%s.json\n\n", data_dir, query->collection);

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
                print_doc_field(&doc->fields[j]);
                has_prev = true;
            }
        } else {
            for (int p = 0; p < query->projection.field_count; p++) {
                const DocField* f = find_field(doc, query->projection.fields[p]);
                if (f) {
                    if (has_prev) printf(",\n");
                    print_doc_field(f);
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
