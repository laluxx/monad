/*
 * @file sexparser.c
 * @version 0.0.1
 */

#include "sexparser.h"

#ifndef NULL
#define NULL ((void*)0)
#endif


/// UTILITY FUNCTIONS

int sexp_isspace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

int sexp_isdigit(char c) {
    return c >= '0' && c <= '9';
}

int sexp_isalpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int sexp_issymbol_start(char c) {
    return sexp_isalpha(c) ||
           c == '+' || c == '-' || c == '*' || c == '/' ||
           c == '=' || c == '>' || c == '<' || c == '?' ||
           c == '!' || c == '_' || c == '&' || c == '|' ||
           c == '%' || c == '^' || c == '~';
}

int sexp_issymbol_char(char c) {
    return sexp_issymbol_start(c) || sexp_isdigit(c);
}

/// PARSER IMPLEMENTATION

static void parser_advance(SexpParser *p) {
    if (p->current == '\n') {
        p->line++;
        p->column = 1;
    } else {
        p->column++;
    }
    p->current = p->input[p->pos++];
}

static void parser_set_error(SexpParser *p, SexpResult code, const char *msg) {
    p->error_code = code;
    p->error_line = p->line;
    p->error_column = p->column;

    // Copy error message
    int i = 0;
    while (msg[i] && i < SEXP_MAX_ERROR_LENGTH - 1) {
        p->error_msg[i] = msg[i];
        i++;
    }
    p->error_msg[i] = '\0';
}

void sexp_parser_init(SexpParser *parser, const char *input) {
    parser->input = input;
    parser->pos = 1;
    parser->line = 1;
    parser->column = 1;
    parser->current = input[0];
    parser->error_code = SEXP_OK;
    parser->error_msg[0] = '\0';
    parser->error_line = 0;
    parser->error_column = 0;
}

void sexp_skip_whitespace(SexpParser *p) {
    while (p->current != '\0') {
        // Skip whitespace
        if (sexp_isspace(p->current)) {
            parser_advance(p);
            continue;
        }

        // Skip comments (semicolon to end of line)
        if (p->current == ';') {
            while (p->current != '\0' && p->current != '\n') {
                parser_advance(p);
            }
            continue;
        }

        break;
    }
}

int sexp_has_more(SexpParser *parser) {
    sexp_skip_whitespace(parser);
    return parser->current != '\0';
}

// Forward declaration
static void* parse_expr(SexpParser *p, const SexpAllocator *alloc);

static void* parse_number(SexpParser *p, const SexpAllocator *alloc) {
    int32_t num = 0;
    int negative = 0;
    int has_digits = 0;

    // Handle negative sign
    if (p->current == '-') {
        negative = 1;
        parser_advance(p);
    } else if (p->current == '+') {
        parser_advance(p);
    }

    // Parse digits
    while (sexp_isdigit(p->current)) {
        has_digits = 1;

        // Check for overflow (simple check)
        if (num > 214748364) { // INT_MAX/10
            parser_set_error(p, SEXP_ERROR_INVALID_NUMBER, "Number too large");
            return NULL;
        }

        num = num * 10 + (p->current - '0');
        parser_advance(p);
    }

    if (!has_digits) {
        parser_set_error(p, SEXP_ERROR_INVALID_NUMBER, "Invalid number format");
        return NULL;
    }

    return alloc->alloc_int(negative ? -num : num);
}

static void* parse_symbol(SexpParser *p, const SexpAllocator *alloc) {
    static char symbol_buf[SEXP_MAX_SYMBOL_LENGTH];
    int i = 0;

    // First character must be valid symbol start
    if (!sexp_issymbol_start(p->current)) {
        parser_set_error(p, SEXP_ERROR_UNEXPECTED_CHAR, "Invalid symbol start");
        return NULL;
    }

    // Collect symbol characters
    while (sexp_issymbol_char(p->current) && i < SEXP_MAX_SYMBOL_LENGTH - 1) {
        symbol_buf[i++] = p->current;
        parser_advance(p);
    }

    if (i >= SEXP_MAX_SYMBOL_LENGTH - 1 && sexp_issymbol_char(p->current)) {
        parser_set_error(p, SEXP_ERROR_SYMBOL_TOO_LONG, "Symbol exceeds maximum length");
        return NULL;
    }

    symbol_buf[i] = '\0';

    // Check for special literals
    if (symbol_buf[0] == 'n' && symbol_buf[1] == 'i' &&
        symbol_buf[2] == 'l' && symbol_buf[3] == '\0') {
        return alloc->alloc_nil();
    }

    if (symbol_buf[0] == '#') {
        if (symbol_buf[1] == 't' && symbol_buf[2] == '\0') {
            return alloc->alloc_bool(1);
        }
        if (symbol_buf[1] == 'f' && symbol_buf[2] == '\0') {
            return alloc->alloc_bool(0);
        }
    }

    return alloc->alloc_symbol(symbol_buf);
}

static void* parse_list(SexpParser *p, const SexpAllocator *alloc) {
    parser_advance(p); // Skip '('
    sexp_skip_whitespace(p);

    // Empty list
    if (p->current == ')') {
        parser_advance(p);
        return alloc->alloc_nil();
    }

    // Parse list elements
    void *head = NULL;
    void *tail = NULL;

    while (p->current != ')' && p->current != '\0') {
        void *expr = parse_expr(p, alloc);
        if (!expr) {
            return NULL; // Error already set
        }

        void *new_cons = alloc->alloc_cons(expr, alloc->alloc_nil());
        if (!new_cons) {
            parser_set_error(p, SEXP_ERROR_ALLOC_FAILED, "Allocation failed");
            return NULL;
        }

        if (head == NULL) {
            head = new_cons;
            tail = new_cons;
        } else {
            // Update the cdr of the tail to point to new_cons
            // This requires the allocator to expose a way to modify cons cells
            // For now, we'll build the list properly
            tail = alloc->alloc_cons(expr, alloc->alloc_nil());
            if (!tail) {
                parser_set_error(p, SEXP_ERROR_ALLOC_FAILED, "Allocation failed");
                return NULL;
            }
        }

        sexp_skip_whitespace(p);

        // Check for improper list (dotted pair)
        if (p->current == '.') {
            parser_advance(p);
            sexp_skip_whitespace(p);

            void *cdr_expr = parse_expr(p, alloc);
            if (!cdr_expr) {
                return NULL;
            }

            // Create improper list
            void *result = alloc->alloc_cons(expr, cdr_expr);
            if (!result) {
                parser_set_error(p, SEXP_ERROR_ALLOC_FAILED, "Allocation failed");
                return NULL;
            }

            sexp_skip_whitespace(p);
            if (p->current != ')') {
                parser_set_error(p, SEXP_ERROR_UNMATCHED_PAREN, "Expected ')' after dotted pair");
                return NULL;
            }
            parser_advance(p);
            return result;
        }
    }

    if (p->current != ')') {
        parser_set_error(p, SEXP_ERROR_UNMATCHED_PAREN, "Unmatched '('");
        return NULL;
    }

    parser_advance(p);
    return head ? head : alloc->alloc_nil();
}

static void* parse_list_proper(SexpParser *p, const SexpAllocator *alloc) {
    parser_advance(p); // Skip '('
    sexp_skip_whitespace(p);

    // Empty list
    if (p->current == ')') {
        parser_advance(p);
        return alloc->alloc_nil();
    }

    // Build list in reverse, then reverse it (or use recursive approach)
    // For efficiency, we'll collect elements in an array first
    #define MAX_LIST_ELEMENTS 256
    void *elements[MAX_LIST_ELEMENTS];
    int count = 0;

    while (p->current != ')' && p->current != '\0') {
        if (count >= MAX_LIST_ELEMENTS) {
            parser_set_error(p, SEXP_ERROR_ALLOC_FAILED, "List too long");
            return NULL;
        }

        void *expr = parse_expr(p, alloc);
        if (!expr) {
            return NULL;
        }

        elements[count++] = expr;
        sexp_skip_whitespace(p);

        // Check for dotted pair
        if (p->current == '.') {
            parser_advance(p);
            sexp_skip_whitespace(p);

            void *cdr_expr = parse_expr(p, alloc);
            if (!cdr_expr) {
                return NULL;
            }

            sexp_skip_whitespace(p);
            if (p->current != ')') {
                parser_set_error(p, SEXP_ERROR_UNMATCHED_PAREN, "Expected ')' after dotted pair");
                return NULL;
            }
            parser_advance(p);

            // Build improper list from right to left
            void *result = cdr_expr;
            for (int i = count - 1; i >= 0; i--) {
                result = alloc->alloc_cons(elements[i], result);
                if (!result) {
                    parser_set_error(p, SEXP_ERROR_ALLOC_FAILED, "Allocation failed");
                    return NULL;
                }
            }
            return result;
        }
    }

    if (p->current != ')') {
        parser_set_error(p, SEXP_ERROR_UNMATCHED_PAREN, "Unmatched '('");
        return NULL;
    }
    parser_advance(p);

    // Build proper list from right to left
    void *result = alloc->alloc_nil();
    for (int i = count - 1; i >= 0; i--) {
        result = alloc->alloc_cons(elements[i], result);
        if (!result) {
            parser_set_error(p, SEXP_ERROR_ALLOC_FAILED, "Allocation failed");
            return NULL;
        }
    }

    return result;
}

static void* parse_quote(SexpParser *p, const SexpAllocator *alloc) {
    parser_advance(p); // Skip '

    void *expr = parse_expr(p, alloc);
    if (!expr) {
        // If nothing after quote, quote nil
        expr = alloc->alloc_nil();
    }

    // Build (quote expr)
    void *quote_sym = alloc->alloc_symbol("quote");
    if (!quote_sym) {
        parser_set_error(p, SEXP_ERROR_ALLOC_FAILED, "Allocation failed");
        return NULL;
    }

    void *quoted_expr = alloc->alloc_cons(expr, alloc->alloc_nil());
    if (!quoted_expr) {
        parser_set_error(p, SEXP_ERROR_ALLOC_FAILED, "Allocation failed");
        return NULL;
    }

    void *result = alloc->alloc_cons(quote_sym, quoted_expr);
    if (!result) {
        parser_set_error(p, SEXP_ERROR_ALLOC_FAILED, "Allocation failed");
        return NULL;
    }

    return result;
}

static void* parse_expr(SexpParser *p, const SexpAllocator *alloc) {
    sexp_skip_whitespace(p);

    if (p->current == '\0') {
        parser_set_error(p, SEXP_ERROR_UNEXPECTED_EOF, "Unexpected end of input");
        return NULL;
    }

    // List
    if (p->current == '(') {
        return parse_list_proper(p, alloc);
    }

    // Quote
    if (p->current == '\'') {
        return parse_quote(p, alloc);
    }

    // Number (including negative)
    if (sexp_isdigit(p->current) ||
        (p->current == '-' && sexp_isdigit(p->input[p->pos]))) {
        return parse_number(p, alloc);
    }

    // Plus sign followed by digit is also a number
    if (p->current == '+' && sexp_isdigit(p->input[p->pos])) {
        return parse_number(p, alloc);
    }

    // Symbol
    if (sexp_issymbol_start(p->current)) {
        return parse_symbol(p, alloc);
    }

    // Unexpected character
    parser_set_error(p, SEXP_ERROR_UNEXPECTED_CHAR, "Unexpected character");
    return NULL;
}

void* sexp_parse(SexpParser *parser, const SexpAllocator *allocator) {
    return parse_expr(parser, allocator);
}

const char* sexp_get_error(SexpParser *parser) {
    static char error_buffer[256];

    if (parser->error_code == SEXP_OK) {
        return "No error";
    }

    // Format error with line and column
    char *buf = error_buffer;
    int i = 0;

    // Add line number
    const char *line_prefix = "Line ";
    while (*line_prefix && i < 250) {
        buf[i++] = *line_prefix++;
    }

    // Add line number (simple int to string)
    int line = parser->error_line;
    if (line == 0) line = 1;
    char line_buf[12];
    int line_len = 0;
    do {
        line_buf[line_len++] = '0' + (line % 10);
        line /= 10;
    } while (line > 0);

    for (int j = line_len - 1; j >= 0 && i < 250; j--) {
        buf[i++] = line_buf[j];
    }

    // Add column
    const char *col_prefix = ", column ";
    while (*col_prefix && i < 250) {
        buf[i++] = *col_prefix++;
    }

    int col = parser->error_column;
    if (col == 0) col = 1;
    char col_buf[12];
    int col_len = 0;
    do {
        col_buf[col_len++] = '0' + (col % 10);
        col /= 10;
    } while (col > 0);

    for (int j = col_len - 1; j >= 0 && i < 250; j--) {
        buf[i++] = col_buf[j];
    }

    // Add separator
    const char *sep = ": ";
    while (*sep && i < 250) {
        buf[i++] = *sep++;
    }

    // Add error message
    const char *msg = parser->error_msg;
    while (*msg && i < 255) {
        buf[i++] = *msg++;
    }

    buf[i] = '\0';
    return error_buffer;
}
