#pragma once

#include <stddef.h>
#include "token.h"

typedef struct LEXER_STRUCT {
    char c;
    size_t i;
    size_t content_size;
    const char* contents;
} Lexer_T;

Lexer_T* Init_Lexer(const char* contents);

void Lexer_Advance(Lexer_T*);
void Lexer_Skip_WhiteSpace(Lexer_T*);

Token_T* Lexer_Get_Next_Token(Lexer_T*);
Token_T* Lexer_Collect_String(Lexer_T*);
Token_T* Lexer_Collect_Number(Lexer_T*);
Token_T* Lexer_Collect_Id(Lexer_T*);

Token_T* Lexer_Advance_With_Token(
    Lexer_T*,
    Token_T*
);

char* Lexer_Get_Current_Char_As_String(
    Lexer_T*
);
