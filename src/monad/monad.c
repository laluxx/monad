/*
 * @file lnlisp.c
 * @version 0.0.1
 * LNLISP - Minimal Scheme interpreter for LNL Kernel
 */

#include "monad.h"
#include "sexparser.h"
#include "../cursor.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

// This probably means that those functions
// should be in theyr own module...
extern void print(const char *str);
extern void putchar(char c);

/// SIMPLE MEMORY

#define HEAP_SIZE 8192
static LNL heap[HEAP_SIZE];
static int heap_pos = 0;

// Special singletons
static LNL nil_obj   = {TYPE_NIL,     0, NULL, {0}};
static LNL true_obj  = {TYPE_BOOLEAN, 0, NULL, {.boolean = 1}};
static LNL false_obj = {TYPE_BOOLEAN, 0, NULL, {.boolean = 0}};

LNL* lnl_nil(void)   { return &nil_obj;   }
LNL* lnl_true(void)  { return &true_obj;  }
LNL* lnl_false(void) { return &false_obj; }

static LNL* alloc_obj(void) {
    if (heap_pos >= HEAP_SIZE) {
        return NULL;
    }
    LNL *obj = &heap[heap_pos++];
    obj->marked = 0;
    obj->next = NULL;
    return obj;
}

/// STRING HELPERS
static int str_equal(const char *s1, const char *s2) {
    if (!s1 || !s2) return 0;
    while (*s1 && *s2 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *s1 == *s2;
}

static void str_copy(char *dst, const char *src, int max_len) {
    int i = 0;
    while (src[i] && i < max_len - 1) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

/// SYMBOL TABLE

static char symbol_table[MAX_SYMBOLS][MAX_SYMBOL_LENGTH];
static int symbol_count = 0;

static const char* intern_symbol(const char *name) {
    // Check if exists
    for (int i = 0; i < symbol_count; i++) {
        if (str_equal(symbol_table[i], name)) {
            return symbol_table[i];
        }
    }

    // Add new
    if (symbol_count < MAX_SYMBOLS) {
        str_copy(symbol_table[symbol_count], name, MAX_SYMBOL_LENGTH);
        return symbol_table[symbol_count++];
    }
    return NULL;
}

/// CONSTRUCTORS

LNL* lnl_int(int32_t val) {
    LNL *obj = alloc_obj();
    if (!obj) return lnl_nil();
    obj->type = TYPE_INTEGER;
    obj->value.integer = val;
    return obj;
}

LNL* lnl_symbol(const char *name) {
    LNL *obj = alloc_obj();
    if (!obj) return lnl_nil();
    obj->type = TYPE_SYMBOL;
    obj->value.symbol = (char*)intern_symbol(name);
    return obj;
}

LNL* lnl_cons(LNL *car, LNL *cdr) {
    LNL *obj = alloc_obj();
    if (!obj) return lnl_nil();
    obj->type = TYPE_CONS;
    obj->value.cons.car = car ? car : lnl_nil();
    obj->value.cons.cdr = cdr ? cdr : lnl_nil();
    return obj;
}

LNL* lnl_builtin(LNLBuiltin func) {
    LNL *obj = alloc_obj();
    if (!obj) return lnl_nil();
    obj->type = TYPE_BUILTIN;
    obj->value.builtin = func;
    return obj;
}

LNL* lnl_function(LNL *params, LNL *body, Environment *env) {
    LNL *obj = alloc_obj();
    if (!obj) return lnl_nil();
    obj->type = TYPE_FUNCTION;
    obj->value.function.params = params;
    obj->value.function.body = body;
    obj->value.function.env = env;
    return obj;
}

/// ENVIRONMENT

static Environment envs[128];
static int env_count = 0;
static Environment *global_env = NULL;

Environment* env_create(Environment *parent) {
    if (env_count >= 128) return NULL;
    Environment *env = &envs[env_count++];
    env->size = 0;
    env->parent = parent;
    return env;
}

void env_define(Environment *env, const char *symbol, LNL *value) {
    const char *sym = intern_symbol(symbol);
    if (!sym) return;

    // Check if exists - update it
    for (int i = 0; i < env->size; i++) {
        if (str_equal(env->symbols[i], sym)) {
            env->values[i] = value;
            return;
        }
    }

    // Add new
    if (env->size < MAX_SYMBOLS) {
        env->symbols[env->size] = (char*)sym;
        env->values[env->size] = value;
        env->size++;
    }
}

LNL* env_lookup(Environment *env, const char *symbol) {
    while (env) {
        for (int i = 0; i < env->size; i++) {
            if (str_equal(env->symbols[i], symbol)) {
                return env->values[i];
            }
        }
        env = env->parent;
    }
    return NULL;
}

/// UTILITIES

int lnl_is_nil(LNL *obj) {
    return !obj || obj == &nil_obj || obj->type == TYPE_NIL;
}

int lnl_is_pair(LNL *obj) {
    return obj && obj->type == TYPE_CONS;
}

LNL* lnl_car(LNL *obj) {
    if (!obj || obj->type != TYPE_CONS) return lnl_nil();
    return obj->value.cons.car ? obj->value.cons.car : lnl_nil();
}

LNL* lnl_cdr(LNL *obj) {
    if (!obj || obj->type != TYPE_CONS) return lnl_nil();
    return obj->value.cons.cdr ? obj->value.cons.cdr : lnl_nil();
}

/// PARSER CALLBACKS

static void* cb_nil(void)              { return lnl_nil();                    }
static void* cb_bool(int v)            { return v ? lnl_true() : lnl_false(); }
static void* cb_int(int32_t v)         { return lnl_int(v);                   }
static void* cb_sym(const char *n)     { return lnl_symbol(n);                }
static void* cb_cons(void *a, void *b) { return lnl_cons((LNL*)a, (LNL*)b);   }

LNL* lnlisp_read(const char *input) {
    SexpParser parser;
    SexpAllocator alloc = {cb_nil, cb_bool, cb_int, cb_sym, cb_cons};
    sexp_parser_init(&parser, input);
    LNL *result = (LNL*)sexp_parse(&parser, &alloc);

    // Check for parse errors
    if (parser.error_code != SEXP_OK) {
        print("Parse error: ");
        print(sexp_get_error(&parser));
        print("\n");
        return NULL;
    }

    return result;
}

/// EVALUATOR

static LNL* eval_list(LNL *exprs, Environment *env);

LNL* lnlisp_eval(LNL *expr, Environment *env) {
    if (!expr) return lnl_nil();

    // NIL evaluates to itself
    if (lnl_is_nil(expr)) return expr;

    // Self-evaluating types
    if (expr->type == TYPE_INTEGER || expr->type == TYPE_BOOLEAN) {
        return expr;
    }

    // Variable lookup
    if (expr->type == TYPE_SYMBOL) {
        LNL *val = env_lookup(env, expr->value.symbol);
        if (!val) {
            print("Undefined variable: ");
            print(expr->value.symbol);
            print("\n");
            return lnl_nil();
        }
        return val;
    }

    // List - special form or function call
    if (expr->type == TYPE_CONS) {
        LNL *first = lnl_car(expr);
        LNL *rest = lnl_cdr(expr);

        // Empty list
        if (lnl_is_nil(first)) {
            return expr;
        }

        // Check for special forms
        if (first->type == TYPE_SYMBOL) {
            const char *sym = first->value.symbol;

            // quote
            if (str_equal(sym, "quote")) {
                return lnl_car(rest);
            }

            // define
            if (str_equal(sym, "define")) {
                LNL *var = lnl_car(rest);
                LNL *val_expr = lnl_car(lnl_cdr(rest));

                if (var->type != TYPE_SYMBOL) {
                    print("define: first argument must be a symbol\n");
                    return lnl_nil();
                }

                LNL *val = lnlisp_eval(val_expr, env);
                env_define(env, var->value.symbol, val);
                return val;
            }

            // lambda
            if (str_equal(sym, "lambda")) {
                LNL *params = lnl_car(rest);
                LNL *body = lnl_cdr(rest);
                return lnl_function(params, body, env);
            }

            // if
            if (str_equal(sym, "if")) {
                LNL *cond = lnlisp_eval(lnl_car(rest), env);
                int is_false = (cond->type == TYPE_BOOLEAN && !cond->value.boolean);

                if (is_false) {
                    LNL *else_expr = lnl_car(lnl_cdr(lnl_cdr(rest)));
                    if (lnl_is_nil(else_expr)) return lnl_nil();
                    return lnlisp_eval(else_expr, env);
                } else {
                    return lnlisp_eval(lnl_car(lnl_cdr(rest)), env);
                }
            }
        }

        // Function application
        LNL *fn = lnlisp_eval(first, env);

        if (lnl_is_nil(fn)) {
            print("Cannot apply nil\n");
            return lnl_nil();
        }

        // Eval args
        LNL *args = lnl_nil();
        LNL *tail = NULL;
        LNL *curr = rest;

        while (lnl_is_pair(curr)) {
            LNL *arg = lnlisp_eval(lnl_car(curr), env);
            LNL *new_cons = lnl_cons(arg, lnl_nil());

            if (lnl_is_nil(args)) {
                args = new_cons;
                tail = new_cons;
            } else {
                tail->value.cons.cdr = new_cons;
                tail = new_cons;
            }
            curr = lnl_cdr(curr);
        }

        // Apply function
        if (fn->type == TYPE_BUILTIN) {
            return fn->value.builtin(args, env);
        }

        if (fn->type == TYPE_FUNCTION) {
            Environment *new_env = env_create(fn->value.function.env);
            LNL *params = fn->value.function.params;
            LNL *arg_vals = args;

            while (lnl_is_pair(params) && lnl_is_pair(arg_vals)) {
                LNL *param = lnl_car(params);
                if (param->type == TYPE_SYMBOL) {
                    env_define(new_env, param->value.symbol, lnl_car(arg_vals));
                }
                params = lnl_cdr(params);
                arg_vals = lnl_cdr(arg_vals);
            }

            return eval_list(fn->value.function.body, new_env);
        }

        print("Not a function\n");
        return lnl_nil();
    }

    return lnl_nil();
}

static LNL* eval_list(LNL *exprs, Environment *env) {
    LNL *result = lnl_nil();
    while (lnl_is_pair(exprs)) {
        result = lnlisp_eval(lnl_car(exprs), env);
        exprs = lnl_cdr(exprs);
    }
    return result;
}

/// PRIMITIVES

static LNL* prim_add(LNL *args, Environment *env) {
    (void)env;
    int32_t sum = 0;
    while (lnl_is_pair(args)) {
        LNL *arg = lnl_car(args);
        if (arg->type == TYPE_INTEGER) {
            sum += arg->value.integer;
        }
        args = lnl_cdr(args);
    }
    return lnl_int(sum);
}

static LNL* prim_sub(LNL *args, Environment *env) {
    (void)env;
    if (!lnl_is_pair(args)) return lnl_int(0);

    LNL *first = lnl_car(args);
    if (first->type != TYPE_INTEGER) return lnl_int(0);

    int32_t result = first->value.integer;
    args = lnl_cdr(args);

    if (lnl_is_nil(args)) {
        return lnl_int(-result);
    }

    while (lnl_is_pair(args)) {
        LNL *arg = lnl_car(args);
        if (arg->type == TYPE_INTEGER) {
            result -= arg->value.integer;
        }
        args = lnl_cdr(args);
    }
    return lnl_int(result);
}

static LNL* prim_mul(LNL *args, Environment *env) {
    (void)env;
    int32_t prod = 1;
    while (lnl_is_pair(args)) {
        LNL *arg = lnl_car(args);
        if (arg->type == TYPE_INTEGER) {
            prod *= arg->value.integer;
        }
        args = lnl_cdr(args);
    }
    return lnl_int(prod);
}

static LNL* prim_eq(LNL *args, Environment *env) {
    (void)env;
    if (!lnl_is_pair(args)) return lnl_true();

    LNL *first = lnl_car(args);
    args = lnl_cdr(args);

    while (lnl_is_pair(args)) {
        LNL *curr = lnl_car(args);

        if (first->type != curr->type) {
            return lnl_false();
        }

        if (first->type == TYPE_INTEGER) {
            if (first->value.integer != curr->value.integer) {
                return lnl_false();
            }
        }

        args = lnl_cdr(args);
    }
    return lnl_true();
}

static LNL* prim_cons(LNL *args, Environment *env) {
    (void)env;
    if (!lnl_is_pair(args)) return lnl_nil();
    LNL *car = lnl_car(args);
    LNL *cdr = lnl_car(lnl_cdr(args));
    return lnl_cons(car, cdr);
}

static LNL* prim_car(LNL *args, Environment *env) {
    (void)env;
    if (!lnl_is_pair(args)) return lnl_nil();
    return lnl_car(lnl_car(args));
}

static LNL* prim_cdr(LNL *args, Environment *env) {
    (void)env;
    if (!lnl_is_pair(args)) return lnl_nil();
    return lnl_cdr(lnl_car(args));
}

static LNL* prim_list(LNL *args, Environment *env) {
    (void)env;
    return args;
}

/// PRINTER

static void print_list(LNL *obj) {
    putchar('(');
    int first = 1;
    while (lnl_is_pair(obj)) {
        if (!first) putchar(' ');
        first = 0;
        lnlisp_print(lnl_car(obj));
        obj = lnl_cdr(obj);
    }
    if (!lnl_is_nil(obj)) {
        print(" . ");
        lnlisp_print(obj);
    }
    putchar(')');
}

void lnlisp_print(LNL *obj) {
    if (!obj || lnl_is_nil(obj)) {
        print("()");
        return;
    }

    switch (obj->type) {
        case TYPE_INTEGER: {
            int32_t n = obj->value.integer;
            if (n == 0) {
                putchar('0');
                return;
            }

            char buf[12];
            int i = 0;
            int neg = 0;

            if (n < 0) {
                neg = 1;
                if (n == -2147483648) {
                    print("-2147483648");
                    return;
                }
                n = -n;
            }

            while (n > 0) {
                buf[i++] = '0' + (n % 10);
                n /= 10;
            }

            if (neg) putchar('-');
            for (int j = i - 1; j >= 0; j--) {
                putchar(buf[j]);
            }
            break;
        }

        case TYPE_BOOLEAN:
            print(obj->value.boolean ? "#t" : "#f");
            break;

        case TYPE_SYMBOL:
            if (obj->value.symbol) {
                print(obj->value.symbol);
            } else {
                print("<symbol?>");
            }
            break;

        case TYPE_CONS:
            print_list(obj);
            break;

        case TYPE_FUNCTION:
            print("<lambda>");
            break;

        case TYPE_BUILTIN:
            print("<builtin>");
            break;

        default:
            print("<?>");
            break;
    }
}

/// REPL

static char input_buf[MAX_INPUT];
static int input_pos = 0;

extern void putchar_at(char c, uint8_t color, uint32_t x, uint32_t y);
extern uint32_t cursor_x;
extern uint32_t cursor_y;

void lnlisp_repl_input(char c) {
    const int PROMPT_LEN = 5;  // Length of "LNL> "

    // Handle Ctrl+A - beginning of line
    if (c == 1) {  // Ctrl+A
        cursor_x = PROMPT_LEN;
        cursor_update();
        return;
    }

    // Handle Ctrl+E - end of line
    if (c == 5) {  // Ctrl+E
        cursor_x = PROMPT_LEN + input_pos;
        cursor_update();
        return;
    }

    // Handle Ctrl+F - forward one character
    if (c == 6) {  // Ctrl+F
        if (cursor_x < PROMPT_LEN + input_pos) {
            cursor_x++;
            cursor_update();
        }
        return;
    }

    // Handle Ctrl+B - backward one character
    if (c == 2) {  // Ctrl+B
        if (cursor_x > PROMPT_LEN) {
            cursor_x--;
            cursor_update();
        }
        return;
    }

    // Handle Ctrl+K - kill to end of line
    if (c == 11) {  // Ctrl+K
        int cursor_offset = cursor_x - PROMPT_LEN;
        int old_pos = input_pos;
        input_pos = cursor_offset;

        // Clear to end of line visually
        int saved_x = cursor_x;
        for (int i = cursor_offset; i < old_pos; i++) {
            putchar(' ');
        }
        cursor_x = saved_x;
        cursor_update();
        return;
    }

    // Handle Ctrl+D - delete character at cursor
    if (c == 4) {  // Ctrl+D
        int cursor_offset = cursor_x - PROMPT_LEN;
        if (cursor_offset < input_pos) {
            // Remove character at cursor
            for (int i = cursor_offset; i < input_pos - 1; i++) {
                input_buf[i] = input_buf[i + 1];
            }
            input_pos--;

            // Redraw from cursor position
            int saved_x = cursor_x;
            for (int i = cursor_offset; i < input_pos; i++) {
                putchar(input_buf[i]);
            }
            putchar(' ');  // Clear last character
            cursor_x = saved_x;
            cursor_update();
        }
        return;
    }

    if (c == '\n') {
        input_buf[input_pos] = '\0';
        putchar('\n');

        if (input_pos > 0) {
            LNL *expr = lnlisp_read(input_buf);
            if (expr) {
                LNL *result = lnlisp_eval(expr, global_env);
                if (result) {
                    lnlisp_print(result);
                    putchar('\n');
                }
            }
        }

        input_pos = 0;
        print("LNL> ");
    } else if (c == '\b' || c == 127) {
        int cursor_offset = cursor_x - PROMPT_LEN;
        if (cursor_offset > 0) {
            // Remove character before cursor
            for (int i = cursor_offset - 1; i < input_pos - 1; i++) {
                input_buf[i] = input_buf[i + 1];
            }
            input_pos--;
            cursor_x--;

            // Redraw from cursor position
            int saved_x = cursor_x;
            for (int i = cursor_offset - 1; i < input_pos; i++) {
                putchar(input_buf[i]);
            }
            putchar(' ');  // Clear last character
            cursor_x = saved_x;
            cursor_update();
        }
    } else if (c >= 32 && c < 127) {
        if (input_pos < MAX_INPUT - 1) {
            int cursor_offset = cursor_x - PROMPT_LEN;

            // Insert character at cursor position
            for (int i = input_pos; i > cursor_offset; i--) {
                input_buf[i] = input_buf[i - 1];
            }
            input_buf[cursor_offset] = c;
            input_pos++;

            // Redraw from cursor position
            for (int i = cursor_offset; i < input_pos; i++) {
                putchar(input_buf[i]);
            }
            cursor_x = PROMPT_LEN + cursor_offset + 1;
            cursor_update();
        }
    }
}

void lnlisp_init(void) {
    heap_pos = 0;
    symbol_count = 0;
    env_count = 0;

    global_env = env_create(NULL);

    env_define(global_env, "+", lnl_builtin(prim_add));
    env_define(global_env, "-", lnl_builtin(prim_sub));
    env_define(global_env, "*", lnl_builtin(prim_mul));
    env_define(global_env, "=", lnl_builtin(prim_eq));
    env_define(global_env, "cons", lnl_builtin(prim_cons));
    env_define(global_env, "car", lnl_builtin(prim_car));
    env_define(global_env, "cdr", lnl_builtin(prim_cdr));
    env_define(global_env, "list", lnl_builtin(prim_list));

    print("MONADLISP v0.0.1\n");
}

void lnlisp_repl(void) {
    print("LNL> ");
}
