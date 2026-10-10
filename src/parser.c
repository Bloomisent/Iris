#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "include/token.h"
#include "include/parser.h"
#include "include/scope.h"

extern char* read_file_to_string(const char *filename);
extern char* g_iris_library_dir;

extern int current_line;
extern int current_col;

int available_id = 0;

char* get_directory(const char* path){
    const char* last_slash = strrchr(path, '/');
    const char* last_backslash = strrchr(path, '\\');
    const char* last_sep = last_slash;
    if (last_backslash != (void*)0 && (last_sep == (void*)0 || last_backslash > last_sep)) {
        last_sep = last_backslash;
    }

    if (last_sep == (void*)0) {
        char* empty = malloc(1);
        empty[0] = '\0';
        return empty; // no directory component -- file is in the cwd
    }

    size_t len = (size_t)(last_sep - path) + 1; // include the separator itself
    char* dir = malloc(len + 1);
    memcpy(dir, path, len);
    dir[len] = '\0';
    return dir;
};

char* join_path(const char* base_dir, const char* relative_path){
    size_t total_len = strlen(base_dir) + strlen(relative_path) + 1;
    char* result = malloc(total_len);
    strcpy(result, base_dir);
    strcat(result, relative_path);
    return result;
};

Parser_T* Init_Parser(Lexer_T* Lexer){
    Parser_T* parser = calloc(1, sizeof(struct PARSER_STRUCT));
    parser->lexer = Lexer;
    parser->id = available_id;
    parser->current_token = Lexer_Get_Next_Token(Lexer);
    parser->previous_token = parser->current_token;
    parser->Scope = Init_Scope();
    parser->table_names = (void*)0;
    parser->table_names_size = 0;
    parser->dict_names = (void*)0;
    parser->dict_names_size = 0;
    parser->class_names = (void*)0;
    parser->class_names_size = 0;
    parser->included_paths = (void*)0;
    parser->included_paths_size = 0;
    available_id++;
    return parser;
};

int Parser_Is_Known_Table(Parser_T* Parser, const char* name){
    for (size_t i = 0; i < Parser->table_names_size; i++) {
        if (strcmp(Parser->table_names[i], name) == 0) {
            return 1;
        }
    }
    return 0;
};
int Parser_Is_Known_Class(Parser_T* Parser, const char* name){
    for (size_t i = 0; i < Parser->class_names_size; i++) {
        if (strcmp(Parser->class_names[i], name) == 0) {
            return 1;
        }
    }
    return 0;
};
int Parser_Is_Known_Dict(Parser_T* Parser, const char* name){
    for (size_t i = 0; i < Parser->dict_names_size; i++) {
        if (strcmp(Parser->dict_names[i], name) == 0) {
            return 1;
        }
    }
    return 0;
};

void Parser_Eat(Parser_T* Parser, int token_type){
    if (Parser->current_token->type == token_type) {
        Parser->previous_token = Parser->current_token;
        Parser->current_token = Lexer_Get_Next_Token(Parser->lexer);
    } else {
        printf("Tripped on unknown token '%s' with type %d, at line %d", Parser->current_token->value, Parser->current_token->type, Parser->current_token->line);
        exit(1);
    };
};

AST_T* Parser_Parse(Parser_T* Parser, Scope_T* Scope){
    AST_T* result = Parser_Parse_Statements(Parser, Scope);

    if (Parser->current_token->type != TOKEN_EOF) {
        printf(
            "Tripped on trailing content, unexpected token '%s' with type %d after the last recognized statement (at line %d)\n",
            Parser->current_token->value,
            Parser->current_token->type,
            Parser->current_token->line
        );
        exit(1);
    }

    return result;
};
AST_T* Parser_Parse_Statement(Parser_T* Parser, Scope_T* Scope){
    switch (Parser->current_token->type){
        case TOKEN_ID: {
            AST_T* expr = Parser_Parse_Expr(Parser, Scope);

            if (Parser->current_token->type == TOKEN_EQUALS) {
                if (expr->type != AST_DOT && expr->type != AST_VARIABLE) {
                    printf("Tripped on assignment, left-hand side must be a member access (obj > \"key\") or a variable (at line %d)\n", Parser->current_token->line);
                    exit(1);
                };

                Parser_Eat(Parser, TOKEN_EQUALS);
                AST_T* value = Parser_Parse_Expr(Parser, Scope);
                value->current_line = Parser->current_token->line;

                AST_T* assignment = Init_AST(AST_ASSIGNMENT);
                assignment->current_line = Parser->current_token->line;
                assignment->assignment_target = expr;
                assignment->assignment_value = value;
                assignment->scope = Scope;
                return assignment;
            } else if (Parser->current_token->type == TOKEN_NULLADD) {
                if (expr->type != AST_DOT && expr->type != AST_VARIABLE) {
                    printf("Tripped on assignment, left-hand side must be a member access (obj > \"key\") or a variable (at line %d)\n", Parser->current_token->line);
                    exit(1);
                };

                Parser_Eat(Parser, TOKEN_NULLADD);
                AST_T* right = Parser_Parse_Additive(Parser, Scope);
                right->current_line = Parser->current_token->line;

                AST_T* value = Init_AST(AST_BINOP);
                value->current_line = Parser->current_token->line;
                value->binop_left = expr;
                value->binop_op = TOKEN_NULLADD;
                value->binop_right = right;
                value->scope = Scope;

                AST_T* assignment = Init_AST(AST_ASSIGNMENT);
                assignment->current_line = Parser->current_token->line;
                assignment->assignment_target = expr;
                assignment->assignment_value = value;
                assignment->scope = Scope;
                return assignment;
            } else if (Parser->current_token->type == TOKEN_ADD) {
                if (expr->type != AST_DOT && expr->type != AST_VARIABLE) {
                    printf("Tripped on assignment, left-hand side must be a member access (obj > \"key\") or a variable (at line %d)\n", Parser->current_token->line);
                    exit(1);
                };

                Parser_Eat(Parser, TOKEN_ADD);
                AST_T* right = Parser_Parse_Additive(Parser, Scope);
                right->current_line = Parser->current_token->line;

                AST_T* value = Init_AST(AST_BINOP);
                value->current_line = Parser->current_token->line;
                value->binop_left = expr;
                value->binop_op = TOKEN_ADD;
                value->binop_right = right;
                value->scope = Scope;

                AST_T* assignment = Init_AST(AST_ASSIGNMENT);
                assignment->current_line = Parser->current_token->line;
                assignment->assignment_target = expr;
                assignment->assignment_value = value;
                assignment->scope = Scope;
                return assignment;
            } else if (Parser->current_token->type == TOKEN_SUB) {
                if (expr->type != AST_DOT && expr->type != AST_VARIABLE) {
                    printf("Tripped on assignment, left-hand side must be a member access (obj > \"key\") or a variable (at line %d)\n", Parser->current_token->line);
                    exit(1);
                };

                Parser_Eat(Parser, TOKEN_SUB);
                AST_T* right = Parser_Parse_Additive(Parser, Scope);
                right->current_line = Parser->current_token->line;

                AST_T* value = Init_AST(AST_BINOP);
                value->current_line = Parser->current_token->line;
                value->binop_left = expr;
                value->binop_op = TOKEN_SUB;
                value->binop_right = right;
                value->scope = Scope;

                AST_T* assignment = Init_AST(AST_ASSIGNMENT);
                assignment->current_line = Parser->current_token->line;
                assignment->assignment_target = expr;
                assignment->assignment_value = value;
                assignment->scope = Scope;
                return assignment;
            } else if (Parser->current_token->type == TOKEN_MULT) {
                if (expr->type != AST_DOT && expr->type != AST_VARIABLE) {
                    printf("Tripped on assignment, left-hand side must be a member access (obj > \"key\") or a variable (at line %d)\n", Parser->current_token->line);
                    exit(1);
                };

                Parser_Eat(Parser, TOKEN_MULT);
                AST_T* right = Parser_Parse_Additive(Parser, Scope);
                right->current_line = Parser->current_token->line;

                AST_T* value = Init_AST(AST_BINOP);
                value->current_line = Parser->current_token->line;
                value->binop_left = expr;
                value->binop_op = TOKEN_MULT;
                value->binop_right = right;
                value->scope = Scope;

                AST_T* assignment = Init_AST(AST_ASSIGNMENT);
                assignment->current_line = Parser->current_token->line;
                assignment->assignment_target = expr;
                assignment->assignment_value = value;
                assignment->scope = Scope;
                return assignment;
            } else if (Parser->current_token->type == TOKEN_DIV) {
                if (expr->type != AST_DOT && expr->type != AST_VARIABLE) {
                    printf("Tripped on assignment, left-hand side must be a member access (obj > \"key\") or a variable (at line %d)\n", Parser->current_token->line);
                    exit(1);
                };

                Parser_Eat(Parser, TOKEN_DIV);
                AST_T* right = Parser_Parse_Additive(Parser, Scope);
                right->current_line = Parser->current_token->line;

                AST_T* value = Init_AST(AST_BINOP);
                value->current_line = Parser->current_token->line;
                value->binop_left = expr;
                value->binop_op = TOKEN_DIVIDE;
                value->binop_right = right;
                value->scope = Scope;

                AST_T* assignment = Init_AST(AST_ASSIGNMENT);
                assignment->current_line = Parser->current_token->line;
                assignment->assignment_target = expr;
                assignment->assignment_value = value;
                assignment->scope = Scope;
                return assignment;
            }

            return expr;
        }
    };
    return Init_AST(AST_NOOP);
};
AST_T* Parser_Parse_Statements(Parser_T* Parser, Scope_T* Scope){

    AST_T* compound = Init_AST(AST_COMPOUND);
    compound->scope=Scope;
    compound->compound_value = calloc(1, sizeof(struct AST_STRUCT*));
    compound->current_line = Parser->current_token->line;

    AST_T* ast_statement = Parser_Parse_Statement(Parser, Scope);
    ast_statement->current_line = Parser->current_token->line;
    compound->compound_value[0] = ast_statement;
    compound->compound_size += 1;

    while (Parser->current_token->type == TOKEN_SEMI) {
        Parser_Eat(Parser, TOKEN_SEMI);

        if (Parser->current_token->type == TOKEN_ID) {
            AST_T* ast_statement2 = Parser_Parse_Statement(Parser, Scope);
            ast_statement2->current_line = Parser->current_token->line;

            compound->compound_size += 1;
            compound->compound_value = realloc(
                compound->compound_value,
                compound->compound_size * sizeof(struct AST_STRUCT*)
            );
            compound->compound_value[compound->compound_size - 1] = ast_statement2;
            ast_statement2->scope=Scope;
        }
    };

    ast_statement->scope=Scope;
    return compound;
};

AST_T* Parser_Parse_Expr(Parser_T* Parser, Scope_T* Scope){
    AST_T* node = Parser_Parse_Logic_And(Parser, Scope);
    node->current_line = Parser->current_token->line;
    while (Parser->current_token->type == TOKEN_OR) {
        Parser_Eat(Parser, TOKEN_OR);
        AST_T* right = Parser_Parse_Logic_And(Parser, Scope);
        right->current_line = Parser->current_token->line;
        AST_T* binop = Init_AST(AST_BINOP);
        binop->current_line = Parser->current_token->line;
        binop->binop_left = node; binop->binop_op = TOKEN_OR; binop->binop_right = right;
        binop->scope = Scope;
        node = binop;
    };
    return node;
};
AST_T* Parser_Parse_Logic_And(Parser_T* Parser, Scope_T* Scope){
    AST_T* node = Parser_Parse_Comparison(Parser, Scope);
    node->current_line = Parser->current_token->line;
    while (Parser->current_token->type == TOKEN_AND) {
        Parser_Eat(Parser, TOKEN_AND);
        AST_T* right = Parser_Parse_Comparison(Parser, Scope);
        right->current_line = Parser->current_token->line;
        AST_T* binop = Init_AST(AST_BINOP);
        binop->current_line = Parser->current_token->line;
        binop->binop_left = node; binop->binop_op = TOKEN_AND; binop->binop_right = right;
        binop->scope = Scope;
        node = binop;
    };
    return node;
};
AST_T* Parser_Parse_Comparison(Parser_T* Parser, Scope_T* Scope){
    AST_T* node = Parser_Parse_Additive(Parser, Scope);
    node->current_line = Parser->current_token->line;
    while (Parser->current_token->type == TOKEN_GT ||
           Parser->current_token->type == TOKEN_LT ||
           Parser->current_token->type == TOKEN_GTE ||
           Parser->current_token->type == TOKEN_LTE ||
           Parser->current_token->type == TOKEN_EQ ||
           Parser->current_token->type == TOKEN_NEQ) {
        int op = Parser->current_token->type;
        Parser_Eat(Parser, op);

        AST_T* right = Parser_Parse_Additive(Parser, Scope);
        right->current_line = Parser->current_token->line;

        AST_T* binop = Init_AST(AST_BINOP);
        binop->current_line = Parser->current_token->line;
        binop->binop_left = node;
        binop->binop_op = op;
        binop->binop_right = right;
        binop->scope = Scope;

        node = binop;
    };
    return node;
};
AST_T* Parser_Parse_Additive(Parser_T* Parser, Scope_T* Scope){
    AST_T* node = Parser_Parse_Term(Parser, Scope);
    node->current_line = Parser->current_token->line;
    while (Parser->current_token->type == TOKEN_PLUS || Parser->current_token->type == TOKEN_MINUS) {
        int op = Parser->current_token->type;
        Parser_Eat(Parser, op);

        AST_T* right = Parser_Parse_Term(Parser, Scope);
        right->current_line = Parser->current_token->line;

        AST_T* binop = Init_AST(AST_BINOP);
        binop->current_line = Parser->current_token->line;
        binop->binop_left = node;
        binop->binop_op = op;
        binop->binop_right = right;
        binop->scope = Scope;

        node = binop;
    };
    return node;
};
AST_T* Parser_Parse_Factor(Parser_T* Parser, Scope_T* Scope){
    if (Parser->current_token->type == TOKEN_NOT) {
        Parser_Eat(Parser, TOKEN_NOT);
        AST_T* operand = Parser_Parse_Factor(Parser, Scope);
        operand->current_line = Parser->current_token->line;

        AST_T* node = Init_AST(AST_UNARY_NOT);
        node->current_line = Parser->current_token->line;
        node->unary_operand = operand;
        node->scope = Scope;
        return node;
    };
    if (Parser->current_token->type == TOKEN_MINUS) {
        Parser_Eat(Parser, TOKEN_MINUS);

        AST_T* operand = Parser_Parse_Factor(Parser, Scope);
        AST_T* zero = Init_AST(AST_NUMBER);
        operand->current_line = Parser->current_token->line;
        zero->current_line = Parser->current_token->line;
        zero->number_value = 0;

        AST_T* binop = Init_AST(AST_BINOP);
        binop->current_line = Parser->current_token->line;
        binop->binop_left = zero;
        binop->binop_op = TOKEN_MINUS;
        binop->binop_right = operand;
        binop->scope = Scope;
        return binop;
    };
    AST_T* node = NULL;
    switch (Parser->current_token->type){
        case TOKEN_LPAREN: {
            Parser_Eat(Parser, TOKEN_LPAREN);
            node = Parser_Parse_Expr(Parser, Scope);
            node->current_line = Parser->current_token->line;
            Parser_Eat(Parser, TOKEN_RPAREN);
            return node;
        }
        case TOKEN_TERNARY: node = Parser_Parse_Ternary(Parser, Scope); node->current_line = Parser->current_token->line; return node; break;
        case TOKEN_STRING: node = Parser_Parse_String(Parser, Scope); node->current_line = Parser->current_token->line; return node; break;
        case TOKEN_NUMBER: node = Parser_Parse_Number(Parser, Scope); node->current_line = Parser->current_token->line; return node; break;
        case TOKEN_ID: node = Parser_Parse_Id(Parser, Scope); node->current_line = Parser->current_token->line; return node; break;
        default:
            printf("Tripped on factor, unexpected token type %d, at line %d\n", Parser->current_token->type, Parser->current_token->line);
            exit(1);
            break;
    };
    return Init_AST(AST_NOOP);
};
static AST_T* Parser_Parse_Dot_Chain(Parser_T* Parser, Scope_T* Scope) {
    AST_T* node = Parser_Parse_Factor(Parser, Scope);
    node->current_line = Parser->current_token->line;

    while (Parser->current_token->type == TOKEN_DOT) {
        Parser_Eat(Parser, TOKEN_DOT);

        if (Parser->current_token->type == TOKEN_ID && strcmp(Parser->current_token->value, "init") == 0) {
            Parser_Eat(Parser, TOKEN_ID);
            Parser_Eat(Parser, TOKEN_LPAREN);

            AST_T* call = Init_AST(AST_INIT_CALL);
            call->current_line = Parser->current_token->line;
            call->init_call_instance = node;
            call->scope = Scope;

            if (Parser->current_token->type != TOKEN_RPAREN) {
                AST_T* arg = Parser_Parse_Expr(Parser, Scope);
                arg->current_line = Parser->current_token->line;
                call->function_call_arguments = calloc(1, sizeof(struct AST_STRUCT*));
                call->function_call_arguments[0] = arg;
                call->function_call_arguments_size = 1;

                while (Parser->current_token->type == TOKEN_COMMA) {
                    Parser_Eat(Parser, TOKEN_COMMA);
                    AST_T* arg2 = Parser_Parse_Expr(Parser, Scope);
                    arg2->current_line = Parser->current_token->line;
                    call->function_call_arguments_size += 1;
                    call->function_call_arguments = realloc(
                        call->function_call_arguments,
                        call->function_call_arguments_size * sizeof(struct AST_STRUCT*)
                    );
                    call->function_call_arguments[call->function_call_arguments_size - 1] = arg2;
                };
            }

            Parser_Eat(Parser, TOKEN_RPAREN);
            node = call;
            node->current_line = Parser->current_token->line;
            continue;
        }
        AST_T* str = Init_AST(AST_STRING);
        str->current_line = Parser->current_token->line;
        str->current_line = Parser->current_token->line;
        if (Parser->current_token->type != TOKEN_ID) {
            printf("Tripped on dot chain, invalid key (at line %d)\n", str->current_line);
            exit(1);
        };
        str->string_value = Parser->current_token->value;
        Parser_Eat(Parser, TOKEN_ID);
        AST_T* right = str;
        right->current_line = Parser->current_token->line;

        AST_T* dot = Init_AST(AST_DOT);
        dot->current_line = Parser->current_token->line;
        dot->dot_left = node;
        dot->dot_right = right;
        dot->scope = Scope;

        node = dot;
    };

    return node;
};
AST_T* Parser_Parse_Term(Parser_T* Parser, Scope_T* Scope){
    AST_T* node = Parser_Parse_Dot_Chain(Parser, Scope);
    node->current_line = Parser->current_token->line;

    while (Parser->current_token->type == TOKEN_MULTIPLY || Parser->current_token->type == TOKEN_DIVIDE || Parser->current_token->type == TOKEN_EXPONENT || Parser->current_token->type == TOKEN_MODULO) {
        int op = Parser->current_token->type;
        Parser_Eat(Parser, op);

        AST_T* right = Parser_Parse_Dot_Chain(Parser, Scope);
        right->current_line = Parser->current_token->line;

        AST_T* binop = Init_AST(AST_BINOP);
        binop->current_line = Parser->current_token->line;
        binop->binop_left = node;
        binop->binop_op = op;
        binop->binop_right = right;
        binop->scope = Scope;

        node = binop;
    };

    return node;
};
AST_T* Parser_Parse_Variable_Definition(Parser_T* Parser, Scope_T* Scope) {
    Parser_Eat(Parser, TOKEN_ID); // variable
    char* variable_def_name = Parser->current_token->value;
    Parser_Eat(Parser, TOKEN_ID); // variable name
    Parser_Eat(Parser, TOKEN_EQUALS); // equals sign

    AST_T* variable_def_val = Parser_Parse_Expr(Parser, Scope);
    AST_T* variable_def = Init_AST(AST_VARIABLE_DEFINITION);
    variable_def_val->current_line = Parser->current_token->line;
    variable_def->current_line = Parser->current_token->line;
    variable_def->variable_definition_variable_name = variable_def_name;
    variable_def->variable_definition_value = variable_def_val;
    variable_def->variable_const = false;

    variable_def->scope=Scope;
    return variable_def;
};
AST_T* Parser_Parse_Const_Variable_Definition(Parser_T* Parser, Scope_T* Scope) {
    Parser_Eat(Parser, TOKEN_ID); // const
    char* const_def_name = Parser->current_token->value;
    Parser_Eat(Parser, TOKEN_ID); // const name
    Parser_Eat(Parser, TOKEN_EQUALS); // equals sign

    AST_T* const_def_val = Parser_Parse_Expr(Parser, Scope);
    AST_T* const_def = Init_AST(AST_VARIABLE_DEFINITION);
    const_def_val->current_line = Parser->current_token->line;
    const_def->current_line = Parser->current_token->line;
    const_def->variable_definition_variable_name = const_def_name;
    const_def->variable_definition_value = const_def_val;
    const_def->variable_const = true;

    const_def->scope=Scope;
    return const_def;
};
AST_T* Parser_Parse_Variable(Parser_T* Parser, Scope_T* Scope){
    char* token_value = Parser->current_token->value;
    Parser_Eat(Parser, TOKEN_ID);

    if (Parser->current_token->type == TOKEN_LPAREN) {
        return Parser_Parse_Function_Call(Parser, Scope);
    }

    AST_T* ast_variable = Init_AST(AST_VARIABLE);
    ast_variable->variable_name = token_value;
    ast_variable->current_line = Parser->current_token->line;

    ast_variable->scope=Scope;
    return ast_variable;
};

AST_T* Parser_Parse_Table_Definition(Parser_T* Parser, Scope_T* Scope) {
    Parser_Eat(Parser, TOKEN_ID); // table
    char* table_def_name = Parser->current_token->value;
    Parser_Eat(Parser, TOKEN_ID); // table name
    Parser_Eat(Parser, TOKEN_EQUALS); // equals sign

    AST_T* table_def = Init_AST(AST_TABLE_DEFINITION);
    table_def->current_line = Parser->current_token->line;
    table_def->table_definition_name = table_def_name;
    table_def->scope=Scope;

    if (Parser->current_token->type == TOKEN_LSQUARE) {

        Parser_Eat(Parser, TOKEN_LSQUARE); // square bracket

        table_def->table_definition_value = calloc(1, sizeof(struct AST_STRUCT*));
        table_def->table_size = 1;
        if (Parser->current_token->type == TOKEN_RSQUARE) {
            AST_T* null_var = Init_AST(AST_NULL);
            null_var->scope = table_def->scope;
            table_def->table_definition_value[0] = null_var;
        } else {
            table_def->table_definition_value[0] = Parser_Parse_Expr(Parser, Scope);

            while (Parser->current_token->type == TOKEN_COMMA) {
                Parser_Eat(Parser, TOKEN_COMMA);
                table_def->table_size += 1;
                table_def->table_definition_value = realloc(
                    table_def->table_definition_value,
                    table_def->table_size * sizeof(struct AST_STRUCT*)
                );
                table_def->table_definition_value[table_def->table_size - 1] = Parser_Parse_Expr(Parser, Scope);
            }
        };

        Parser_Eat(Parser, TOKEN_RSQUARE); // square bracket

        Parser->table_names_size += 1;
        Parser->table_names = realloc(
            Parser->table_names,
            Parser->table_names_size * sizeof(char*)
        );
        Parser->table_names[Parser->table_names_size - 1] = table_def_name;
    } else if (Parser->current_token->type == TOKEN_ID) {

        AST_T* table = Parser_Parse_Id(Parser, Scope);
        table->current_line = Parser->current_token->line;

        if (table->type != AST_TABLE) {
            printf("Tripped on table assignment, not a table/invalid type (at line %d)\n", Parser->current_token->line);
            exit(1);
        };

        AST_T* true_val = Scope_Get_Table_Definition(Scope, table->table_name);
        true_val->current_line = Parser->current_token->line;

        table_def->table_size = true_val->table_size;
        table_def->table_definition_value = true_val->table_definition_value;

        Parser->table_names_size += 1;
        Parser->table_names = realloc(
            Parser->table_names,
            Parser->table_names_size * sizeof(char*)
        );
        Parser->table_names[Parser->table_names_size - 1] = table_def_name;
    };

    return table_def;
};
AST_T* Parser_Parse_Table(Parser_T* Parser, Scope_T* Scope){
    char* token_value = Parser->current_token->value;
    Parser_Eat(Parser, TOKEN_ID);

    if (Parser->current_token->type == TOKEN_LPAREN) {
        return Parser_Parse_Function_Call(Parser, Scope);
    }

    AST_T* ast_table = Init_AST(AST_TABLE);
    ast_table->current_line = Parser->current_token->line;
    ast_table->table_name = token_value;
    ast_table->type = AST_TABLE;

    ast_table->scope=Scope;
    return ast_table;
};

AST_T* Parser_Parse_Dictionary_Definition(Parser_T* Parser, Scope_T* Scope) {
    Parser_Eat(Parser, TOKEN_ID); // dict
    char* dict_def_name = Parser->current_token->value;
    Parser_Eat(Parser, TOKEN_ID); // dict name
    Parser_Eat(Parser, TOKEN_EQUALS); // equals sign

    Parser_Eat(Parser, TOKEN_LSQUARE); // square bracket

    AST_T* dict_def = Init_AST(AST_DICTIONARY_DEFINITION);
    dict_def->current_line = Parser->current_token->line;
    dict_def->dictionary_definition_name = dict_def_name;
    dict_def->scope=Scope;

    dict_def->dictionary_definition_value_name = calloc(1, sizeof(struct AST_STRUCT*));
    dict_def->dictionary_definition_value_name[0] = Parser_Parse_String(Parser, Scope);
    Parser_Eat(Parser, TOKEN_COLON);
    dict_def->dictionary_definition_value = calloc(1, sizeof(struct AST_STRUCT*));
    dict_def->dictionary_definition_value[0] = Parser_Parse_Expr(Parser, Scope);
    dict_def->dictionary_size = 1;

    while (Parser->current_token->type == TOKEN_COMMA) {
        Parser_Eat(Parser, TOKEN_COMMA);
        dict_def->dictionary_size += 1;
        dict_def->dictionary_definition_value_name = realloc(
            dict_def->dictionary_definition_value_name,
            dict_def->dictionary_size * sizeof(struct AST_STRUCT*)
        );
        dict_def->dictionary_definition_value_name[dict_def->dictionary_size - 1] = Parser_Parse_String(Parser, Scope);
        Parser_Eat(Parser, TOKEN_COLON);
        dict_def->dictionary_definition_value = realloc(
            dict_def->dictionary_definition_value,
            dict_def->dictionary_size * sizeof(struct AST_STRUCT*)
        );
        dict_def->dictionary_definition_value[dict_def->dictionary_size - 1] = Parser_Parse_Expr(Parser, Scope);
    }

    Parser_Eat(Parser, TOKEN_RSQUARE); // square bracket

    Parser->dict_names_size += 1;
    Parser->dict_names = realloc(
        Parser->dict_names,
        Parser->dict_names_size * sizeof(char*)
    );
    Parser->dict_names[Parser->dict_names_size - 1] = dict_def_name;

    return dict_def;
};
AST_T* Parser_Parse_Dictionary(Parser_T* Parser, Scope_T* Scope){
    char* token_value = Parser->current_token->value;
    Parser_Eat(Parser, TOKEN_ID);

    if (Parser->current_token->type == TOKEN_LPAREN) {
        return Parser_Parse_Function_Call(Parser, Scope);
    }

    AST_T* ast_dict = Init_AST(AST_DICTIONARY);
    ast_dict->current_line = Parser->current_token->line;
    ast_dict->dictionary_name = token_value;
    ast_dict->type = AST_DICTIONARY;

    ast_dict->scope=Scope;
    return ast_dict;
};

static void Parser_Parse_Class_Init(Parser_T* Parser, Scope_T* Scope, AST_T* class_def) {
    Parser_Eat(Parser, TOKEN_STRING); // consume the 'init' key itself
    Parser_Eat(Parser, TOKEN_COLON);
    Parser_Eat(Parser, TOKEN_ID); // func
    Parser_Eat(Parser, TOKEN_LPAREN);

    if (Parser->current_token->type != TOKEN_RPAREN) {
        class_def->init_args = calloc(1, sizeof(struct AST_STRUCT*));
        AST_T* arg = Parser_Parse_Variable(Parser, Scope);
        arg->current_line = Parser->current_token->line;
        class_def->init_args_size += 1;
        class_def->current_line = Parser->current_token->line;
        class_def->init_args[class_def->init_args_size - 1] = arg;

        while (Parser->current_token->type == TOKEN_COMMA) {
            Parser_Eat(Parser, TOKEN_COMMA);
            class_def->init_args_size += 1;
            class_def->init_args = realloc(
                class_def->init_args,
                class_def->init_args_size * sizeof(struct AST_STRUCT*)
            );
            AST_T* arg2 = Parser_Parse_Variable(Parser, Scope);
            arg2->current_line = Parser->current_token->line;
            class_def->init_args[class_def->init_args_size - 1] = arg2;
        };
    }

    Parser_Eat(Parser, TOKEN_RPAREN);
    Parser_Eat(Parser, TOKEN_LCURLY);

    class_def->init_value = Parser_Parse_Statements(Parser, Scope);
    class_def->init_step = 1;

    Parser_Eat(Parser, TOKEN_RCURLY);
};

AST_T* Parser_Parse_Class_Definition(Parser_T* Parser, Scope_T* Scope) {
    Parser_Eat(Parser, TOKEN_ID); // class
    char* class_def_name = Parser->current_token->value;
    Parser_Eat(Parser, TOKEN_ID); // class name

    Parser_Eat(Parser, TOKEN_LCURLY);

    AST_T* class_def = Init_AST(AST_CLASS_DEFINITION);
    class_def->current_line = Parser->current_token->line;
    class_def->class_definition_name = class_def_name;
    class_def->scope=Scope;

    class_def->class_definition_value = calloc(1, sizeof(struct AST_STRUCT*));
    class_def->class_definition_value_name = calloc(1, sizeof(struct AST_STRUCT*));
    class_def->class_size = 0;

    if (strcmp(Parser->current_token->value, "init") == 0) {
        Parser_Parse_Class_Init(Parser, Scope, class_def);
    } else {
        class_def->class_definition_value_name[0] = Parser_Parse_String(Parser, Scope);
        Parser_Eat(Parser, TOKEN_COLON);
        class_def->class_definition_value[0] = Parser_Parse_Expr(Parser, Scope);
        class_def->class_size = 1;
    }

    while (Parser->current_token->type == TOKEN_COMMA) {
        Parser_Eat(Parser, TOKEN_COMMA);

        if (strcmp(Parser->current_token->value, "init") == 0) {
            Parser_Parse_Class_Init(Parser, Scope, class_def);
        } else {
            class_def->class_size += 1;
            class_def->class_definition_value_name = realloc(
                class_def->class_definition_value_name,
                class_def->class_size * sizeof(struct AST_STRUCT*)
            );
            class_def->class_definition_value = realloc(
                class_def->class_definition_value,
                class_def->class_size * sizeof(struct AST_STRUCT*)
            );
            class_def->class_definition_value_name[class_def->class_size - 1] = Parser_Parse_String(Parser, Scope);
            Parser_Eat(Parser, TOKEN_COLON);
            class_def->class_definition_value[class_def->class_size - 1] = Parser_Parse_Expr(Parser, Scope);
        }
    };

    Parser_Eat(Parser, TOKEN_RCURLY);

    Parser->class_names_size += 1;
    Parser->class_names = realloc(
        Parser->class_names,
        Parser->class_names_size * sizeof(char*)
    );
    Parser->class_names[Parser->class_names_size - 1] = class_def_name;
    return class_def;
};
AST_T* Parser_Parse_Class(Parser_T* Parser, Scope_T* Scope){
    char* token_value = Parser->current_token->value;
    Parser_Eat(Parser, TOKEN_ID);

    if (Parser->current_token->type == TOKEN_LPAREN) {
        AST_T* call = Parser_Parse_Function_Call(Parser, Scope);
        call->current_line = Parser->current_token->line;
    }

    if (Parser->current_token->type == TOKEN_ID) {
        // ClassName instance_name; -- declare a new independent instance of this class
        char* instance_name = Parser->current_token->value;
        Parser_Eat(Parser, TOKEN_ID);

        AST_T* ast_instantiation = Init_AST(AST_CLASS_INSTANTIATION);
        ast_instantiation->current_line = Parser->current_token->line;
        ast_instantiation->instance_class_name = token_value;
        ast_instantiation->instance_variable_name = instance_name;
        ast_instantiation->scope = Scope;

        Parser->class_names_size += 1;
        Parser->class_names = realloc(
            Parser->class_names,
            Parser->class_names_size * sizeof(char*)
        );
        Parser->class_names[Parser->class_names_size - 1] = instance_name;

        return ast_instantiation;
    }

    AST_T* ast_class = Init_AST(AST_CLASS);
    ast_class->current_line = Parser->current_token->line;
    ast_class->class_name = token_value;
    ast_class->type = AST_CLASS;

    ast_class->scope=Scope;
    return ast_class;
};

AST_T* Parser_Parse_Function_Definition(Parser_T* Parser, Scope_T* Scope) {
    AST_T* ast = Init_AST(AST_FUNCTION_DEFINITION);
    ast->current_line = Parser->current_token->line;
    ast->file_id = Parser->id;

    Parser_Eat(Parser, TOKEN_ID); // function

    bool pub = true;
    if (strcmp(Parser->current_token->value, "public") == 0){
        pub = true;
        Parser_Eat(Parser, TOKEN_ID);
    } else if (strcmp(Parser->current_token->value, "private") == 0){
        pub = false;
        Parser_Eat(Parser, TOKEN_ID);
    };

    char* func_name = Parser->current_token->value;
    ast->function_definition_name = calloc(
        strlen(func_name)+1,
        sizeof(char)
    );
    strcpy(ast->function_definition_name, func_name);

    Parser_Eat(Parser, TOKEN_ID); // function name
    Parser_Eat(Parser, TOKEN_LPAREN);

    ast->function_definition_public = pub;

    if (Parser->current_token->type != TOKEN_RPAREN) {

        ast->function_definition_args = calloc(1, sizeof(struct AST_STRUCT*));

        AST_T* arg = Parser_Parse_Variable(Parser, Scope);
        arg->current_line = Parser->current_token->line;
        ast->function_definition_args_size += 1;
        ast->function_definition_args[ast->function_definition_args_size-1] = arg;

        while (Parser->current_token->type == TOKEN_COMMA) {
            Parser_Eat(Parser, TOKEN_COMMA);
            ast->function_definition_args_size += 1;

            ast->function_definition_args = realloc(
                ast->function_definition_args,
                ast->function_definition_args_size * sizeof(struct AST_STRUCT*)
            );
            AST_T* new_arg = Parser_Parse_Variable(Parser, Scope);
            new_arg->current_line = Parser->current_token->line;
            ast->function_definition_args[ast->function_definition_args_size-1] = new_arg;
        };
    };

    Parser_Eat(Parser, TOKEN_RPAREN); // function compound
    Parser_Eat(Parser, TOKEN_LCURLY); // function compound

    size_t saved_table_names_size = Parser->table_names_size;
    size_t saved_dict_names_size = Parser->dict_names_size;
    size_t saved_class_names_size = Parser->class_names_size;

    ast->function_definition_body = Parser_Parse_Statements(Parser, Scope);

    Parser->table_names_size = saved_table_names_size;
    Parser->dict_names_size = saved_dict_names_size;
    Parser->class_names_size = saved_class_names_size;

    Parser_Eat(Parser, TOKEN_RCURLY); // function compound
    ast->scope=Scope;
    return ast;
};
AST_T* Parser_Parse_Function_Call(Parser_T* Parser, Scope_T* Scope){
    AST_T* function_call = Init_AST(AST_FUNCTION_CALL);
    function_call->current_line = Parser->current_token->line;
    function_call->function_call_name = Parser->previous_token->value;
    function_call->file_id = Parser->id;
    Parser_Eat(Parser, TOKEN_LPAREN);

    function_call->function_call_arguments = NULL;
    function_call->function_call_arguments_size = 0;

    if (Parser->current_token->type != TOKEN_RPAREN) {
        function_call->function_call_arguments = calloc(1, sizeof(struct AST_STRUCT*));
        function_call->function_call_arguments_size += 1;

        AST_T* ast_expr = Parser_Parse_Expr(Parser, Scope);
        ast_expr->current_line = Parser->current_token->line;
        function_call->function_call_arguments[0] = ast_expr;

        while (Parser->current_token->type == TOKEN_COMMA) {
            Parser_Eat(Parser, TOKEN_COMMA);

            AST_T* ast_expr2 = Parser_Parse_Expr(Parser, Scope);
            ast_expr2->current_line = Parser->current_token->line;
            function_call->function_call_arguments_size += 1;
            function_call->function_call_arguments = realloc(
                function_call->function_call_arguments,
                function_call->function_call_arguments_size * sizeof(struct AST_STRUCT*)
            );
            function_call->function_call_arguments[function_call->function_call_arguments_size - 1] = ast_expr2;
        }
    };

    Parser_Eat(Parser, TOKEN_RPAREN);

    function_call->scope=Scope;
    return function_call;
};

AST_T* Parser_Parse_Enum(Parser_T* Parser, Scope_T* Scope) {
    AST_T* ast = Init_AST(AST_ENUM);
    ast->current_line = Parser->current_token->line;
    Parser_Eat(Parser, TOKEN_ID); // enum
    Parser_Eat(Parser, TOKEN_LCURLY);

    AST_T* ast_enum_val = Init_AST(AST_NUMBER); //Parser_Parse_Expr(Parser, Scope);
    ast_enum_val->scope = Scope;
    ast_enum_val->number_value = 1.0f;
    ast_enum_val->current_line = Parser->current_token->line;

    AST_T* ast_enum = Init_AST(AST_VARIABLE_DEFINITION);
    ast_enum->scope = Scope;
    ast_enum->current_line = Parser->current_token->line;
    ast_enum->variable_definition_variable_name = Parser->current_token->value;
    ast_enum->variable_definition_value = ast_enum_val;
    ast_enum->variable_const = true;

    Parser_Eat(Parser, TOKEN_ID);

    ast->enum_body = calloc(1, sizeof(struct AST_STRUCT*));
    ast->enum_body_size = 1;

    ast->enum_body[0] = ast_enum;

    int e = 1;

    while (Parser->current_token->type == TOKEN_COMMA) {
        int lin = Parser->current_token->line;
        Parser_Eat(Parser, TOKEN_COMMA);

        if (Parser->current_token->type != TOKEN_ID) {
            printf("Tripped on enum, trailing comma (at line %d)\n", lin);
            exit(1);
        }

        ast->enum_body_size += 1;
        ast->enum_body = realloc(ast->enum_body, ast->enum_body_size * sizeof(struct AST_STRUCT*));

        AST_T* ast_enum_val2 = Init_AST(AST_NUMBER); //Parser_Parse_Expr(Parser, Scope);
        ast_enum_val2->scope = Scope;
        ast_enum_val2->number_value = (float)e;
        ast_enum_val2->current_line = Parser->current_token->line;

        AST_T* ast_enum2 = Init_AST(AST_VARIABLE_DEFINITION);
        ast_enum2->scope = Scope;
        ast_enum2->current_line = Parser->current_token->line;
        ast_enum2->variable_definition_variable_name = Parser->current_token->value;
        ast_enum2->variable_definition_value = ast_enum_val2;
        ast_enum2->variable_const = true;

        Parser_Eat(Parser, TOKEN_ID);

        ast->enum_body[e] = ast_enum2;
        e++;
    }

    Parser_Eat(Parser, TOKEN_RCURLY);
    return ast;
};

AST_T* Parser_Parse_Ternary(Parser_T* Parser, Scope_T* Scope) {
    AST_T* ast = Init_AST(AST_TERNARY);
    ast->current_line = Parser->current_token->line;
    Parser_Eat(Parser, TOKEN_TERNARY); // tertiary

    Parser_Eat(Parser, TOKEN_LPAREN);
    ast->ternary_condition = Parser_Parse_Expr(Parser, Scope);
    Parser_Eat(Parser, TOKEN_RPAREN);

    ast->ternary_success_var = Parser_Parse_Expr(Parser, Scope);

    Parser_Eat(Parser, TOKEN_COLON);

    ast->ternary_failure_var = Parser_Parse_Expr(Parser, Scope);
    return ast;
};

AST_T* Parser_Parse_If(Parser_T* Parser, Scope_T* Scope) {
    AST_T* ast = Init_AST(AST_IF);
    ast->current_line = Parser->current_token->line;
    Parser_Eat(Parser, TOKEN_ID); // if
    Parser_Eat(Parser, TOKEN_LPAREN);

    ast->if_condition = Parser_Parse_Expr(Parser, Scope);

    Parser_Eat(Parser, TOKEN_RPAREN);
    Parser_Eat(Parser, TOKEN_LCURLY); // if compound

    ast->if_body = Parser_Parse_Statements(Parser, Scope);

    Parser_Eat(Parser, TOKEN_RCURLY); // if compound

    int i = 0;
    bool alloced = false;

    while (Parser->current_token->type == TOKEN_ID) {
        int lin = Parser->current_token->line;
        bool iselse = false;
        if (Parser->current_token->type != TOKEN_ID) {
            printf("Tripped on if statement, trailing comma (at line %d)\n", lin);
            exit(1);
        }

        if (strcmp(Parser->current_token->value, "else") == 0) {
            iselse = true;
            Parser_Eat(Parser, TOKEN_ID);
            if (strcmp(Parser->current_token->value, "if") == 0) {
                iselse = false;
                Parser_Eat(Parser, TOKEN_ID);
            };
        } else {
            break;
        };

        if (iselse == false) {
            Parser_Eat(Parser, TOKEN_LPAREN);

            if (alloced) {
                ast->else_if_size += 1;
                ast->else_if_conditions = realloc(ast->else_if_conditions, ast->else_if_size * sizeof(struct AST_STRUCT*));
                ast->else_if_bodies = realloc(ast->else_if_bodies, ast->else_if_size * sizeof(struct AST_STRUCT*));
            } else {
                alloced = true;
                ast->else_if_size = 1;
                ast->else_if_conditions = calloc(1, sizeof(struct AST_STRUCT*));
                ast->else_if_bodies = calloc(1, sizeof(struct AST_STRUCT*));
            }
            AST_T* ast_condition = Parser_Parse_Expr(Parser, Scope);
            Parser_Eat(Parser, TOKEN_RPAREN);
            Parser_Eat(Parser, TOKEN_LCURLY);

            AST_T* ast_body = Parser_Parse_Statements(Parser, Scope);

            Parser_Eat(Parser, TOKEN_RCURLY);

            ast->else_if_conditions[i] = ast_condition;
            ast->else_if_bodies[i] = ast_body;
            i++;
        } else {
            Parser_Eat(Parser, TOKEN_LCURLY);
            AST_T* ast_body = Parser_Parse_Statements(Parser, Scope);
            Parser_Eat(Parser, TOKEN_RCURLY);

            ast->if_else_body = ast_body;
            break;
        }
    };
    return ast;
};
AST_T* Parser_Parse_Checks(Parser_T* Parser, Scope_T* Scope) {
    AST_T* ast = Init_AST(AST_CHECKS);
    ast->current_line = Parser->current_token->line;
    Parser_Eat(Parser, TOKEN_ID); // checks
    Parser_Eat(Parser, TOKEN_LPAREN);

    ast->checks_base_var = Parser_Parse_Id(Parser, Scope);

    Parser_Eat(Parser, TOKEN_RPAREN);
    Parser_Eat(Parser, TOKEN_LCURLY); // checks compound

    int index = 0;

    Parser_Eat(Parser, TOKEN_LPAREN);

    ast->checks_condition_body = calloc(1, sizeof(struct AST_STRUCT*));
    ast->checks_do_body = calloc(1, sizeof(struct AST_STRUCT*));
    ast->checks_condition_body[index] = Parser_Parse_Expr(Parser, Scope);

    Parser_Eat(Parser, TOKEN_RPAREN);
    Parser_Eat(Parser, TOKEN_COLON);

    Parser_Eat(Parser, TOKEN_LCURLY);

    ast->checks_do_body[index] = Parser_Parse_Statements(Parser, Scope);

    Parser_Eat(Parser, TOKEN_RCURLY);
    index++;

    while (Parser->current_token->type == TOKEN_COMMA) {
        ast->checks_condition_body = realloc(
            ast->checks_condition_body,
            (index+1) * sizeof(struct AST_STRUCT*)
        );
        ast->checks_do_body = realloc(
            ast->checks_do_body,
            (index+1) * sizeof(struct AST_STRUCT*)
        );

        Parser_Eat(Parser, TOKEN_COMMA);

        Parser_Eat(Parser, TOKEN_LPAREN);

        ast->checks_condition_body[index] = Parser_Parse_Expr(Parser, Scope);

        Parser_Eat(Parser, TOKEN_RPAREN);
        Parser_Eat(Parser, TOKEN_COLON);

        Parser_Eat(Parser, TOKEN_LCURLY);

        ast->checks_do_body[index] = Parser_Parse_Statements(Parser, Scope);

        Parser_Eat(Parser, TOKEN_RCURLY);
        index++;
    };
    ast->checks_condition_size = index;

    Parser_Eat(Parser, TOKEN_RCURLY);
    return ast;
};

AST_T* Parser_Parse_For(Parser_T* Parser, Scope_T* Scope) {
    AST_T* ast = Init_AST(AST_FOR);
    ast->current_line = Parser->current_token->line;
    ast->scope = Scope;
    Parser_Eat(Parser, TOKEN_ID); // for
    Parser_Eat(Parser, TOKEN_LPAREN);

    ast->for_variable = Parser_Parse_Expr(Parser, Scope);
    Parser_Eat(Parser, TOKEN_COLON);

    ast->for_condition = Parser_Parse_Expr(Parser, Scope);
    Parser_Eat(Parser, TOKEN_COLON);

    if (Parser->current_token->type == TOKEN_STRING) {
        ast->for_does_at_end = Parser_Parse_Expr(Parser, Scope);
    } else {
        ast->for_does_at_end = Parser_Parse_Statement(Parser, Scope);
    };
    Parser_Eat(Parser, TOKEN_RPAREN);
    Parser_Eat(Parser, TOKEN_LCURLY);

    ast->for_body = Parser_Parse_Statements(Parser, Scope);

    Parser_Eat(Parser, TOKEN_RCURLY);
    return ast;
};
AST_T* Parser_Parse_While(Parser_T* Parser, Scope_T* Scope) {
    AST_T* ast = Init_AST(AST_WHILE);
    ast->current_line = Parser->current_token->line;
    ast->scope = Scope;
    Parser_Eat(Parser, TOKEN_ID); // while
    Parser_Eat(Parser, TOKEN_LPAREN);

    ast->while_condition = Parser_Parse_Expr(Parser, Scope);

    Parser_Eat(Parser, TOKEN_RPAREN);
    Parser_Eat(Parser, TOKEN_LCURLY);

    ast->while_body = Parser_Parse_Statements(Parser, Scope);

    Parser_Eat(Parser, TOKEN_RCURLY);
    return ast;
};
AST_T* Parser_Parse_Return(Parser_T* Parser, Scope_T* Scope) {
    Parser_Eat(Parser, TOKEN_ID); // return
    AST_T* ast = Init_AST(AST_RETURN);
    ast->current_line = Parser->current_token->line;
    ast->return_value = Parser_Parse_Expr(Parser, Scope);
    ast->scope = Scope;
    return ast;
};
AST_T* Parser_Parse_Break(Parser_T* Parser, Scope_T* Scope) {
    Parser_Eat(Parser, TOKEN_ID); // break
    AST_T* ast = Init_AST(AST_BREAK);
    ast->current_line = Parser->current_token->line;
    ast->scope = Scope;
    return ast;
};

AST_T* Parser_Parse_Include(Parser_T* Parser, Scope_T* Scope) {
    Parser_Eat(Parser, TOKEN_ID);

    char* file_path;
    char* requested_path;
    char* path;

    if (Parser->current_token->type == TOKEN_LT) {
        Parser_Eat(Parser, TOKEN_LT);
        char* libname = Parser->current_token->value;
        Parser_Eat(Parser, TOKEN_ID);
        Parser_Eat(Parser, TOKEN_GT);

        char* filename = malloc(strlen(libname) + strlen(".iris") + 1);
        file_path = malloc(strlen(libname) + strlen(".iris") + 1);
        sprintf(filename, "%s.iris", libname);
        sprintf(file_path, filename);
        requested_path = join_path(g_iris_library_dir, filename);
        free(filename);
    } else {
        AST_T* path_node = Parser_Parse_String(Parser, Scope);
        file_path = malloc(strlen(path_node->string_value) + 1);
        sprintf(file_path, path_node->string_value);
        requested_path = join_path(Parser->current_dir, path_node->string_value);
    };
    path = requested_path;

    for (size_t i = 0; i < Parser->included_paths_size; i++) {
        if (strcmp(Parser->included_paths[i], path) == 0) {
            return Init_AST(AST_NOOP); // already included -- skip (handles duplicates and circular includes)
        }
    }
    Parser->included_paths_size += 1;
    Parser->included_paths = realloc(
        Parser->included_paths,
        Parser->included_paths_size * sizeof(char*)
    );
    Parser->included_paths[Parser->included_paths_size - 1] = path;

    char* contents = read_file_to_string(path);
    if (contents == (void*)0) {
        printf("Tripped on include, could not read file '%s' (at line %d)\n", file_path, Parser->current_token->line);
        exit(1);
    }

    Lexer_T* saved_lexer = Parser->lexer;
    Token_T* saved_current = Parser->current_token;
    Token_T* saved_previous = Parser->previous_token;
    int saved_line = Parser->current_token->line;
    int saved_col = Parser->current_token->col;
    int saved_id = Parser->id;
    char* saved_path = Parser->current_dir;

    Lexer_T* include_lexer = Init_Lexer(contents);
    Parser->lexer = include_lexer;
    Parser->id = available_id;
    current_line = 1;
    current_col = 1;
    Parser->current_token = Lexer_Get_Next_Token(include_lexer);
    Parser->previous_token = Parser->current_token;
    Parser->current_dir = get_directory(path);

    available_id++;

    AST_T* included_statements = Parser_Parse_Statements(Parser, Scope);

    if (Parser->current_token->type != TOKEN_EOF) {
        printf("Tripped on include, unexpected trailing token in included file '%s', at line %d\n", file_path, Parser->current_token->line);
        exit(1);
    };

    Parser->lexer = saved_lexer;
    Parser->id = saved_id;
    current_line = saved_line;
    current_col = saved_col;
    Parser->current_token = saved_current;
    Parser->previous_token = saved_previous;
    Parser->current_dir = saved_path;

    return included_statements;
};

AST_T* Parser_Parse_String(Parser_T* Parser, Scope_T* Scope){
    AST_T* ast_string = Init_AST(AST_STRING);
    ast_string->string_value = Parser->current_token->value;
    ast_string->current_line = Parser->current_token->line;

    Parser_Eat(Parser, TOKEN_STRING);

    ast_string->scope=Scope;

    return ast_string;
};
AST_T* Parser_Parse_Number(Parser_T* Parser, Scope_T* Scope){
    AST_T* ast_number = Init_AST(AST_NUMBER);
    ast_number->number_value = strtod(Parser->current_token->value, NULL);
    ast_number->current_line = Parser->current_token->line;

    Parser_Eat(Parser, TOKEN_NUMBER);

    ast_number->scope=Scope;

    return ast_number;
};

AST_T* Parser_Parse_Id(Parser_T* Parser, Scope_T* Scope){
    if (strcmp(Parser->current_token->value, "table") == 0) {
        return Parser_Parse_Table_Definition(Parser, Scope);
    } else if (strcmp(Parser->current_token->value, "class") == 0) {
        return Parser_Parse_Class_Definition(Parser, Scope);
    } else if (strcmp(Parser->current_token->value, "dict") == 0) {
        return Parser_Parse_Dictionary_Definition(Parser, Scope);
    } else if (strcmp(Parser->current_token->value, "var") == 0) {
        return Parser_Parse_Variable_Definition(Parser, Scope);
    } else if (strcmp(Parser->current_token->value, "const") == 0) {
        return Parser_Parse_Const_Variable_Definition(Parser, Scope);
    } else if (strcmp(Parser->current_token->value, "func") == 0) {
        return Parser_Parse_Function_Definition(Parser, Scope);
    } else if (strcmp(Parser->current_token->value, "for") == 0) {
        return Parser_Parse_For(Parser, Scope);
    } else if (strcmp(Parser->current_token->value, "while") == 0) {
        return Parser_Parse_While(Parser, Scope);
    } else if (strcmp(Parser->current_token->value, "enum") == 0) {
        return Parser_Parse_Enum(Parser, Scope);
    } else if (strcmp(Parser->current_token->value, "if") == 0) {
        return Parser_Parse_If(Parser, Scope);
    } else if (strcmp(Parser->current_token->value, "break") == 0) {
        return Parser_Parse_Break(Parser, Scope);
    } else if (strcmp(Parser->current_token->value, "return") == 0) {
        return Parser_Parse_Return(Parser, Scope);
    } else if (strcmp(Parser->current_token->value, "include") == 0) {
        return Parser_Parse_Include(Parser, Scope);
    } else if (strcmp(Parser->current_token->value, "checks") == 0) {
        return Parser_Parse_Checks(Parser, Scope);
    } else if (Parser_Is_Known_Table(Parser, Parser->current_token->value)) {
        return Parser_Parse_Table(Parser, Scope);
    } else if (Parser_Is_Known_Dict(Parser, Parser->current_token->value)) {
        return Parser_Parse_Dictionary(Parser, Scope);
    } else if (Parser_Is_Known_Class(Parser, Parser->current_token->value)) {
        return Parser_Parse_Class(Parser, Scope);
    } else {
        return Parser_Parse_Variable(Parser, Scope);
    }
};
