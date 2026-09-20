#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/lexer.h"

extern int current_line;
extern int current_col;

Lexer_T* Init_Lexer(const char* contents) {

    Lexer_T* lexer = calloc(1, sizeof(Lexer_T));

    if (!lexer) exit(EXIT_FAILURE);

    lexer->contents = contents ? contents : "";

    lexer->content_size = strlen(lexer->contents);
    lexer->i = 0;
    lexer->c = lexer->content_size ? lexer->contents[0] : '\0';

    return lexer;
}

void Lexer_Advance(Lexer_T* lexer) {

    if(lexer->i < lexer->content_size) {
        lexer->i++;
    };

    lexer->c = lexer->i < lexer->content_size ? lexer->contents[lexer->i] : '\0';
}

void Lexer_Skip_WhiteSpace(Lexer_T* lexer) {

    while (lexer->c && isspace((unsigned char)lexer->c)) {
        Lexer_Advance(lexer);
    }
}

static char Lexer_Peek(Lexer_T* lexer) {
    size_t next = lexer->i + 1;
    return next < lexer->content_size ? lexer->contents[next] : '\0';
}

char* Lexer_Get_Current_Char_As_String(Lexer_T* lexer) {
    char* s = malloc(2);
    if (!s) exit(EXIT_FAILURE);
    s[0] = lexer->c;
    s[1] = '\0';
    return s;
}

Token_T* Lexer_Advance_With_Token(Lexer_T* lexer, Token_T* token) {
    token->position = lexer->i;
    token->line = current_line;
    token->col = current_col;
    Lexer_Advance(lexer);
    return token;
}

Token_T* Lexer_Collect_String(Lexer_T* lexer) {

    Lexer_Advance(lexer);

    size_t capacity = 32;
    size_t length = 0;

    char* value =
        malloc(capacity);

    if (!value)
        exit(EXIT_FAILURE);

    while (lexer->c != '"' &&
        lexer->c != '\0') {

        char ch = lexer->c;

        if (lexer->c == '\n'){
            while (lexer->c == '\n') {
                current_line += 1;
                current_col = 1;
                Lexer_Advance(lexer);
            };
        } else if (lexer->c == '\r'){
            while (lexer->c == '\r') {
                current_line += 1;
                current_col = 1;
                Lexer_Advance(lexer);
            };
        };

        if (ch == '\\') {
            Lexer_Advance(lexer);
            if(lexer->c == '\0') {
                free(value);
                fprintf(
                    stderr,
                    "Lexer error: "
                    "unterminated escape sequence"
                    "at line %d, col %zu\n",
                    current_line,
                    lexer->i - (current_line-1)
                );
                exit(EXIT_FAILURE);
            }

            switch (lexer->c) {

                case 'n':
                    ch = '\n';
                    break;

                case 't':
                    ch = '\t';
                    break;

                case '"':
                    ch = '"';
                    break;

                case '\\':
                    ch = '\\';
                    break;

                default:
                    ch = lexer->c;
                    break;
            }

        }

        if (length + 2 > capacity) {
            capacity *= 2;
            char* new_value =realloc(value, capacity);

            if (!new_value) {
                free(value);
                exit(EXIT_FAILURE);
            }
            value = new_value;
        }

        value[length++] = ch;
        Lexer_Advance(lexer);
    }

    if(lexer->c == '\0') {
        free(value);
        fprintf(
            stderr,
            "Lexer error: "
            "unterminated string"
            "at line %d, col %zu\n",
            current_line,
            lexer->i-(current_line-1)
        );
        exit(EXIT_FAILURE);
    }

    value[length] = '\0';
    Lexer_Advance(lexer);

    Token_T* token = Init_Token(TOKEN_STRING, value);
    free(value);

    return token;
}

Token_T* Lexer_Collect_Number(Lexer_T* lexer) {

    size_t capacity = 32;
    size_t length = 0;

    int decimal_found = 0;

    char* value = malloc(capacity);

    if (!value) exit(EXIT_FAILURE);

    while (isdigit((unsigned char)lexer->c) || lexer->c == '.') {
        if (lexer->c == '.') {
            if (decimal_found) {
                free(value);
                fprintf(
                    stderr,
                    "Lexer error: "
                    "invalid number"
                    "at line %d, col %zu\n",
                    current_line,
                    lexer->i-(current_line-1)
                );
                exit(EXIT_FAILURE);
            }
            decimal_found = 1;
        }
        if (length + 2 > capacity) {
            capacity *= 2;
            char* new_value =realloc(value, capacity);

            if (!new_value) {
                free(value);
                exit(EXIT_FAILURE);
            };
            value = new_value;
        }

        value[length++] = lexer->c;
        Lexer_Advance(lexer);
    }

    value[length] = '\0';

    Token_T* token = Init_Token(TOKEN_NUMBER, value);
    free(value);

    return token;
}

Token_T* Lexer_Collect_Id(Lexer_T* lexer) {

    size_t capacity = 32;
    size_t length = 0;

    char* value = malloc(capacity);

    if (!value) exit(EXIT_FAILURE);

    while (isalnum((unsigned char)lexer->c) || lexer->c == '_') {
        if (length + 2 > capacity) {
            capacity *= 2;

            char* new_value = realloc(value,capacity);

            if (!new_value) {
                free(value);
                exit(EXIT_FAILURE);
            }
            value = new_value;
        }
        value[length++] = lexer->c;
        Lexer_Advance(lexer);
    }

    value[length] = '\0';

    Token_T* token = Init_Token(TOKEN_ID, value);
    free(value);

    return token;
}

Token_T* Lexer_Get_Next_Token(Lexer_T* lexer) {
    while (lexer->c != '\0') {
        if (lexer->c == '\n'){
            while (lexer->c == '\n') {
                current_line += 1;
                current_col = 1;
                Lexer_Advance(lexer);
            };
        } else if (lexer->c == '\r'){
            while (lexer->c == '\r') {
                current_line += 1;
                current_col = 1;
                Lexer_Advance(lexer);
            };
        };

        if (lexer->c == '\t') {
            current_col += 4 - ((current_col - 1) % 4);
        } else {
            current_col++;
        }

        if (isspace((unsigned char)lexer->c)) {
            Lexer_Skip_WhiteSpace(lexer);

            continue;
        }

        if (isalpha((unsigned char)lexer->c) ||
            lexer->c == '_') {
            return Lexer_Collect_Id(lexer);
        }

        if (isdigit((unsigned char)lexer->c)) {
            return Lexer_Collect_Number(lexer);
        }

        if (lexer->c == '"') {
            return Lexer_Collect_String(lexer);
        }

        if (lexer->c == '#') {
            Lexer_Advance(lexer);
            while (lexer->c != '#' && lexer->c != '\0') {
                if (lexer->c == '\n'){
                    while (lexer->c == '\n') {
                        current_line += 1;
                        current_col = 1;
                        Lexer_Advance(lexer);
                    };
                } else if (lexer->c == '\r'){
                    while (lexer->c == '\r') {
                        current_line += 1;
                        current_col = 1;
                        Lexer_Advance(lexer);
                    };
                };
                Lexer_Advance(lexer);
            };
            if (lexer->c == '#') {
                Lexer_Advance(lexer);
            } else {
                fprintf(stderr, "Lexer Error: unterminated comment at position %zu, line %d, col %zu\n", lexer->i, current_line, lexer->i - (current_line-1));
                exit(EXIT_FAILURE);
            };
            continue;
        };

        switch (lexer->c) {

            case ';':
                return Lexer_Advance_With_Token(
                    lexer,
                    Init_Token(
                        TOKEN_SEMI,
                        ";"
                    )
                );

            case '(':
                return Lexer_Advance_With_Token(
                    lexer,
                    Init_Token(
                        TOKEN_LPAREN,
                        "("
                    )
                );

            case ')':
                return Lexer_Advance_With_Token(
                    lexer,
                    Init_Token(
                        TOKEN_RPAREN,
                        ")"
                    )
                );

            case '{':
                return Lexer_Advance_With_Token(
                    lexer,
                    Init_Token(
                        TOKEN_LCURLY,
                        "{"
                    )
                );

            case '}':
                return Lexer_Advance_With_Token(
                    lexer,
                    Init_Token(
                        TOKEN_RCURLY,
                        "}"
                    )
                );

            case '[':
                return Lexer_Advance_With_Token(
                    lexer,
                    Init_Token(
                        TOKEN_LSQUARE,
                        "["
                    )
                );

            case ']':
                return Lexer_Advance_With_Token(
                    lexer,
                    Init_Token(
                        TOKEN_RSQUARE,
                        "]"
                    )
                );

            case ',':
                return Lexer_Advance_With_Token(
                    lexer,
                    Init_Token(
                        TOKEN_COMMA,
                        ","
                    )
                );

            case ':':
                return Lexer_Advance_With_Token(
                    lexer,
                    Init_Token(
                        TOKEN_COLON,
                        ":"
                    )
                );

            case '+':
                if (Lexer_Peek(lexer) == '=') {
                    Lexer_Advance(lexer);
                    return Lexer_Advance_With_Token(lexer, Init_Token(TOKEN_ADD, "+="));
                };
                return Lexer_Advance_With_Token(
                    lexer,
                    Init_Token(
                        TOKEN_PLUS,
                        "+"
                    )
                );

            case '-':
                if (Lexer_Peek(lexer) == '=') {
                    Lexer_Advance(lexer);
                    return Lexer_Advance_With_Token(lexer, Init_Token(TOKEN_SUB, "-="));
                };
                return Lexer_Advance_With_Token(
                    lexer,
                    Init_Token(
                        TOKEN_MINUS,
                        "-"
                    )
                );

            case '*':
                if (Lexer_Peek(lexer) == '=') {
                    Lexer_Advance(lexer);
                    return Lexer_Advance_With_Token(lexer, Init_Token(TOKEN_MULT, "*="));
                };
                return Lexer_Advance_With_Token(
                    lexer,
                    Init_Token(
                        TOKEN_MULTIPLY,
                        "*"
                    )
                );

            case '/':
                if (Lexer_Peek(lexer) == '=') {
                    Lexer_Advance(lexer);
                    return Lexer_Advance_With_Token(lexer, Init_Token(TOKEN_DIV, "/="));
                };
                return Lexer_Advance_With_Token(
                    lexer,
                    Init_Token(
                        TOKEN_DIVIDE,
                        "/"
                    )
                );

            case '%':
                return Lexer_Advance_With_Token(
                    lexer,
                    Init_Token(
                        TOKEN_MODULO,
                        "%"
                    )
                );

            case '^':
                return Lexer_Advance_With_Token(
                    lexer,
                    Init_Token(
                        TOKEN_EXPONENT,
                        "^"
                    )
                );
            case '=':
                if (Lexer_Peek(lexer) == '=') {
                    Lexer_Advance(lexer);
                    return Lexer_Advance_With_Token(lexer, Init_Token(TOKEN_EQ, "=="));
                }
                return Lexer_Advance_With_Token(
                    lexer,
                    Init_Token(
                        TOKEN_EQUALS,
                        "="
                    )
                );

            case '!':
                if (Lexer_Peek(lexer) == '=') {
                    Lexer_Advance(lexer);
                    return Lexer_Advance_With_Token(lexer, Init_Token(TOKEN_NEQ, "!="));
                }
                fprintf(stderr, "Lexer error: '!' must be followed by '=' at position %zu, line %d, col %zu\n", lexer->i, current_line, lexer->i - (current_line-1));
                exit(EXIT_FAILURE);
            case '>':
                if (Lexer_Peek(lexer) == '=') {
                    Lexer_Advance(lexer);
                    return Lexer_Advance_With_Token(lexer, Init_Token(TOKEN_GTE, ">="));
                }
                return Lexer_Advance_With_Token(lexer, Init_Token(TOKEN_GT, ">"));
            case '<':
                if (Lexer_Peek(lexer) == '=') {
                    Lexer_Advance(lexer);
                    return Lexer_Advance_With_Token(lexer, Init_Token(TOKEN_LTE, "<="));
                }
                return Lexer_Advance_With_Token(lexer, Init_Token(TOKEN_LT, "<"));
            case '.':
                return Lexer_Advance_With_Token(
                    lexer,
                    Init_Token(
                        TOKEN_DOT,
                        "."
                    )
                );

            case '#' :
                return Lexer_Advance_With_Token(
                    lexer,
                    Init_Token(
                        TOKEN_HASH,
                        "#"
                    )
                );

            default:
                fprintf(
                    stderr,
                    "Lexer error: unexpected character '%c' at position %zu, line %d, col %zu\n",
                    lexer->c,
                    lexer->i,
                    current_line,
                    lexer->i - (current_line-1)
                );
                exit(EXIT_FAILURE);
        }
    }

    return Init_Token(TOKEN_EOF, "");
}

