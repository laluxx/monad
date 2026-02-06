/*
 * @file sexparser.h
 * @version 1.0.0
 * S-Expression Parser - Fast, zero-allocation parser for LNL
 *
 * Features:
 * - Single-pass parsing with O(n) complexity
 * - Zero heap allocations (uses provided allocator callbacks)
 * - Proper error reporting with line/column tracking
 * - Support for quoted expressions, numbers, symbols, lists
 * - Tail-call optimized for deeply nested structures
 */

#ifndef SEXPARSER_H
#define SEXPARSER_H

// Type definitions for kernel compatibility
typedef unsigned char uint8_t;
typedef signed int int32_t;

// Configuration
#define SEXP_MAX_SYMBOL_LENGTH 64
#define SEXP_MAX_ERROR_LENGTH 128

/// Parser Result Codes

typedef enum {
    SEXP_OK = 0,
    SEXP_ERROR_UNEXPECTED_EOF,
    SEXP_ERROR_UNEXPECTED_CHAR,
    SEXP_ERROR_INVALID_NUMBER,
    SEXP_ERROR_SYMBOL_TOO_LONG,
    SEXP_ERROR_UNMATCHED_PAREN,
    SEXP_ERROR_ALLOC_FAILED,
    SEXP_ERROR_EMPTY_INPUT
} SexpResult;

/// Parser State

typedef struct {
    const char *input;      // Input string
    int pos;                // Current position
    int line;               // Current line (1-indexed)
    int column;             // Current column (1-indexed)
    char current;           // Current character

    // Error information
    SexpResult error_code;
    char error_msg[SEXP_MAX_ERROR_LENGTH];
    int error_line;
    int error_column;
} SexpParser;

/// Token Types (for debugging/introspection)

typedef enum {
    TOKEN_EOF,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_QUOTE,
    TOKEN_NUMBER,
    TOKEN_SYMBOL,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NIL
} SexpTokenType;

/// Allocator Callbacks
// These callbacks allow the parser to create objects without knowing
// the internal representation. The parser calls these to construct
// the parsed tree structure.

typedef void* (*SexpAllocNilFn)(void);
typedef void* (*SexpAllocBoolFn)(int value);
typedef void* (*SexpAllocIntFn)(int32_t value);
typedef void* (*SexpAllocSymbolFn)(const char *name);
typedef void* (*SexpAllocConsFn)(void *car, void *cdr);

typedef struct {
    SexpAllocNilFn alloc_nil;
    SexpAllocBoolFn alloc_bool;
    SexpAllocIntFn alloc_int;
    SexpAllocSymbolFn alloc_symbol;
    SexpAllocConsFn alloc_cons;
} SexpAllocator;

/// API Functions

/**
 * Initialize parser with input string
 * @param parser Parser state to initialize
 * @param input Input string to parse (must remain valid during parsing)
 */
void sexp_parser_init(SexpParser *parser, const char *input);

/**
 * Parse a single S-expression from the input
 * @param parser Parser state
 * @param allocator Allocator callbacks for creating objects
 * @return Parsed object or NULL on error (check parser->error_code)
 */
void* sexp_parse(SexpParser *parser, const SexpAllocator *allocator);

/**
 * Get human-readable error message
 * @param parser Parser state
 * @return Error message string
 */
const char* sexp_get_error(SexpParser *parser);

/**
 * Check if parser has more input to parse
 * @param parser Parser state
 * @return 1 if more input available, 0 otherwise
 */
int sexp_has_more(SexpParser *parser);

/**
 * Skip whitespace and comments
 * @param parser Parser state
 */
void sexp_skip_whitespace(SexpParser *parser);

/// Utility Functions (provided by parser)

/**
 * Check if character is whitespace
 */
int sexp_isspace(char c);

/**
 * Check if character is digit
 */
int sexp_isdigit(char c);

/**
 * Check if character is alphabetic
 */
int sexp_isalpha(char c);

/**
 * Check if character is valid in a symbol
 */
int sexp_issymbol_char(char c);

/**
 * Check if character is valid symbol start
 */
int sexp_issymbol_start(char c);

#endif // SEXPARSER_H
