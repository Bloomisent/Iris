#pragma once

typedef enum {
    TOKEN_ID,
    TOKEN_EQUALS,
    TOKEN_STRING,
    TOKEN_SEMI,
    TOKEN_HASH, // For comments
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LCURLY,
    TOKEN_RCURLY,
    TOKEN_COMMA,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_MODULO,
    TOKEN_EXPONENT,
    TOKEN_GT, //comparison
    TOKEN_LT,
    TOKEN_GTE,
    TOKEN_LTE,
    TOKEN_EQ,
    TOKEN_NEQ,//comparison
    TOKEN_DOT,
    TOKEN_NUMBER,
    TOKEN_EOF,
    TOKEN_COLON,
    TOKEN_LSQUARE,
    TOKEN_RSQUARE
} TokenType;

typedef struct TOKEN_STRUCT {
    TokenType type;
    char* value;
} Token_T;

Token_T* Init_Token(int type, const char* value);
void Token_Free(Token_T* token);
