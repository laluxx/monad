/*
 * @file lnlisp.h
 * @version 0.0.1
 * LNLISP - A Scheme interpreter for LNL Kernel
 */

#ifndef LNLISP_H
#define LNLISP_H

// Type definitions
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;

// Configuration
#define MAX_INPUT 1000
#define MAX_SYMBOLS 1000
#define MAX_SYMBOL_LENGTH 64

// Forward declarations
typedef struct LNL LNL;
typedef struct Environment Environment;
typedef LNL* (*LNLBuiltin)(LNL *args, Environment *env);

// LNL object types
typedef enum {
    TYPE_FREE,     // Free object in heap
    TYPE_NIL,      // Scheme nil/()
    TYPE_BOOLEAN,  // #t or #f
    TYPE_INTEGER,  // Integer numbers
    TYPE_FLOAT,    // Floating point (future)
    TYPE_SYMBOL,   // Symbols
    TYPE_STRING,   // Strings (future)
    TYPE_CONS,     // Cons cell (pair)
    TYPE_FUNCTION, // Lambda function
    TYPE_BUILTIN   // Built-in primitive
} LNLType;

// LNL object structure
struct LNL {
    LNLType type;
    uint8_t marked;   // For garbage collection
    struct LNL *next; // For free list

    union {
        int32_t integer;
        uint8_t boolean;
        char *symbol;

        struct {
            struct LNL *car;
            struct LNL *cdr;
        } cons;

        struct {
            struct LNL *params;      // Parameter list
            struct LNL *body;        // Body expressions
            struct Environment *env; // Closure environment
        } function;

        LNLBuiltin builtin;
    } value;
};

// Environment structure (lexical scoping)
struct Environment {
    char *symbols[MAX_SYMBOLS];
    LNL *values[MAX_SYMBOLS];
    int size;
    struct Environment *parent;
};

/// CORE API

void lnlisp_init(void);
void lnlisp_repl(void);
void lnlisp_repl_input(char c);

LNL* lnlisp_read(const char *input);           // R
LNL* lnlisp_eval(LNL *expr, Environment *env); // E
void lnlisp_print(LNL *obj);                   // P
                                               // L
/// MEMORY MANAGEMENT

void lnl_heap_init(void);
LNL* lnl_alloc(void);
void lnl_free(LNL *obj);
void lnl_gc(void);
void lnl_gc_add_root(LNL **root);

/// CONSTRUCTORS

LNL* lnl_nil(void);
LNL* lnl_true(void);
LNL* lnl_false(void);
LNL* lnl_int(int32_t val);
LNL* lnl_symbol(const char *name);
LNL* lnl_cons(LNL *car, LNL *cdr);
LNL* lnl_builtin(LNLBuiltin func);
LNL* lnl_function(LNL *params, LNL *body, Environment *env);

/// ENVIRONMENT OPERATIONS

Environment* env_create(Environment *parent);
void env_define(Environment *env, const char *symbol, LNL *value);
LNL* env_lookup(Environment *env, const char *symbol);
void env_set(Environment *env, const char *symbol, LNL *value);

/// UTILITY FUNCTIONS

// String operations (kernel doesn't have libc)
int  lnl_strcmp(const char *s1, const char *s2);
int  lnl_strlen(const char *s);
void lnl_strcpy(char *dst, const char *src);

// Character classification
int lnl_isdigit(char c);
int lnl_isspace(char c);
int lnl_isalpha(char c);
int lnl_issymbol_char(char c);

// List operations
int lnl_is_nil(LNL *obj);
int lnl_is_pair(LNL *obj);
LNL* lnl_car(LNL *obj);
LNL* lnl_cdr(LNL *obj);
int lnl_list_length(LNL *list);

/// REGISTER C FUNCTIONS (for kernel integration)

// Register a C function as a Lisp primitive
void lnl_register_builtin(const char *name, LNLBuiltin func);

#endif // LNLISP_H
