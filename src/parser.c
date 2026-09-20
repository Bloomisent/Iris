#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "include/token.h"
#include "include/parser.h"
#include "include/scope.h"

extern char* read_file_to_string(const char *filename);

extern int current_line;
extern int current_col;

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

static Scope_T* Get_Node_Scope(Parser_T* Parser, AST_T* Node) {
    return Node->scope == (void*)0 ? Parser->Scope : Node->scope;
}

Parser_T* Init_Parser(Lexer_T* Lexer){
    Parser_T* parser = calloc(1, sizeof(struct PARSER_STRUCT));
    parser->lexer = Lexer;
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

AST_T* Parser_Parse(Parser_T* Parser, Scope_T* Scope){ // main entry, return AST syntax tree
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

                AST_T* assignment = Init_AST(AST_ASSIGNMENT);
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

                AST_T* value = Init_AST(AST_BINOP);
                value->binop_left = expr;
                value->binop_op = TOKEN_ADD;
                value->binop_right = right;
                value->scope = Scope;

                AST_T* assignment = Init_AST(AST_ASSIGNMENT);
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

                AST_T* value = Init_AST(AST_BINOP);
                value->binop_left = expr;
                value->binop_op = TOKEN_SUB;
                value->binop_right = right;
                value->scope = Scope;

                AST_T* assignment = Init_AST(AST_ASSIGNMENT);
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

                AST_T* value = Init_AST(AST_BINOP);
                value->binop_left = expr;
                value->binop_op = TOKEN_MULT;
                value->binop_right = right;
                value->scope = Scope;

                AST_T* assignment = Init_AST(AST_ASSIGNMENT);
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

                AST_T* value = Init_AST(AST_BINOP);
                value->binop_left = expr;
                value->binop_op = TOKEN_DIVIDE;
                value->binop_right = right;
                value->scope = Scope;

                AST_T* assignment = Init_AST(AST_ASSIGNMENT);
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

    AST_T* ast_statement = Parser_Parse_Statement(Parser, Scope);
    compound->compound_value[0] = ast_statement;
    compound->compound_size += 1;

    while (Parser->current_token->type == TOKEN_SEMI) {
        Parser_Eat(Parser, TOKEN_SEMI);

        if (Parser->current_token->type == TOKEN_ID) {
            AST_T* ast_statement2 = Parser_Parse_Statement(Parser, Scope);

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
    AST_T* node = Parser_Parse_Additive(Parser, Scope);

    while (Parser->current_token->type == TOKEN_GT ||
           Parser->current_token->type == TOKEN_LT ||
           Parser->current_token->type == TOKEN_GTE ||
           Parser->current_token->type == TOKEN_LTE ||
           Parser->current_token->type == TOKEN_EQ ||
           Parser->current_token->type == TOKEN_NEQ) {
        int op = Parser->current_token->type;
        Parser_Eat(Parser, op);

        AST_T* right = Parser_Parse_Additive(Parser, Scope);

        AST_T* binop = Init_AST(AST_BINOP);
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

    while (Parser->current_token->type == TOKEN_PLUS || Parser->current_token->type == TOKEN_MINUS) {
        int op = Parser->current_token->type;
        Parser_Eat(Parser, op);

        AST_T* right = Parser_Parse_Term(Parser, Scope);

        AST_T* binop = Init_AST(AST_BINOP);
        binop->binop_left = node;
        binop->binop_op = op;
        binop->binop_right = right;
        binop->scope = Scope;

        node = binop;
    };
    return node;
};
AST_T* Parser_Parse_Factor(Parser_T* Parser, Scope_T* Scope){
    switch (Parser->current_token->type){
        case TOKEN_LPAREN: {
            Parser_Eat(Parser, TOKEN_LPAREN);
            AST_T* node = Parser_Parse_Expr(Parser, Scope);
            Parser_Eat(Parser, TOKEN_RPAREN);
            return node;
        }
        case TOKEN_STRING: return Parser_Parse_String(Parser, Scope); break;
        case TOKEN_NUMBER: return Parser_Parse_Number(Parser, Scope); break;
        case TOKEN_ID: return Parser_Parse_Id(Parser, Scope); break;
        default:
            printf("Tripped on factor, unexpected token type %d, at line %d\n", Parser->current_token->type, Parser->current_token->line);
            exit(1);
            break;
    };
    return Init_AST(AST_NOOP);
};
static AST_T* Parser_Parse_Dot_Chain(Parser_T* Parser, Scope_T* Scope) {
    AST_T* node = Parser_Parse_Factor(Parser, Scope);

    while (Parser->current_token->type == TOKEN_DOT) {
        Parser_Eat(Parser, TOKEN_DOT);

        if (Parser->current_token->type == TOKEN_ID && strcmp(Parser->current_token->value, "init") == 0) {
            Parser_Eat(Parser, TOKEN_ID);
            Parser_Eat(Parser, TOKEN_LPAREN);

            AST_T* call = Init_AST(AST_INIT_CALL);
            call->init_call_instance = node;
            call->scope = Scope;

            if (Parser->current_token->type != TOKEN_RPAREN) {
                AST_T* arg = Parser_Parse_Expr(Parser, Scope);
                call->function_call_arguments = calloc(1, sizeof(struct AST_STRUCT*));
                call->function_call_arguments[0] = arg;
                call->function_call_arguments_size = 1;

                while (Parser->current_token->type == TOKEN_COMMA) {
                    Parser_Eat(Parser, TOKEN_COMMA);
                    AST_T* arg2 = Parser_Parse_Expr(Parser, Scope);
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
            continue;
        }
        AST_T* right = Parser_Parse_String(Parser, Scope);

        AST_T* dot = Init_AST(AST_DOT);
        dot->dot_left = node;
        dot->dot_right = right;
        dot->scope = Scope;

        node = dot;
    };

    return node;
};
AST_T* Parser_Parse_Term(Parser_T* Parser, Scope_T* Scope){
    AST_T* node = Parser_Parse_Dot_Chain(Parser, Scope);

    while (Parser->current_token->type == TOKEN_MULTIPLY || Parser->current_token->type == TOKEN_DIVIDE || Parser->current_token->type == TOKEN_EXPONENT || Parser->current_token->type == TOKEN_MODULO) {
        int op = Parser->current_token->type;
        Parser_Eat(Parser, op);

        AST_T* right = Parser_Parse_Dot_Chain(Parser, Scope);

        AST_T* binop = Init_AST(AST_BINOP);
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
    variable_def->variable_definition_variable_name = variable_def_name;
    variable_def->variable_definition_value = variable_def_val;

    variable_def->scope=Scope;
    return variable_def;
};
AST_T* Parser_Parse_Variable(Parser_T* Parser, Scope_T* Scope){
    char* token_value = Parser->current_token->value;
    Parser_Eat(Parser, TOKEN_ID);

    if (Parser->current_token->type == TOKEN_LPAREN) {
        return Parser_Parse_Function_Call(Parser, Scope);
    }

    AST_T* ast_variable = Init_AST(AST_VARIABLE);
    ast_variable->variable_name = token_value;

    ast_variable->scope=Scope;
    return ast_variable;
};

AST_T* Parser_Parse_Table_Definition(Parser_T* Parser, Scope_T* Scope) {
    Parser_Eat(Parser, TOKEN_ID); // table
    char* table_def_name = Parser->current_token->value;
    Parser_Eat(Parser, TOKEN_ID); // table name
    Parser_Eat(Parser, TOKEN_EQUALS); // equals sign

    AST_T* table_def = Init_AST(AST_TABLE_DEFINITION);
    table_def->table_definition_name = table_def_name;
    table_def->scope=Scope;

    if (Parser->current_token->type == TOKEN_LSQUARE) {

        Parser_Eat(Parser, TOKEN_LSQUARE); // square bracket

        table_def->table_definition_value = calloc(1, sizeof(struct AST_STRUCT*));
        table_def->table_definition_value[0] = Parser_Parse_Expr(Parser, Scope);
        table_def->table_size = 1;

        while (Parser->current_token->type == TOKEN_COMMA) {
            Parser_Eat(Parser, TOKEN_COMMA);
            table_def->table_size += 1;
            table_def->table_definition_value = realloc(
                table_def->table_definition_value,
                table_def->table_size * sizeof(struct AST_STRUCT*)
            );
            table_def->table_definition_value[table_def->table_size - 1] = Parser_Parse_Expr(Parser, Scope);
        }

        Parser_Eat(Parser, TOKEN_RSQUARE); // square bracket

        Parser->table_names_size += 1;
        Parser->table_names = realloc(
            Parser->table_names,
            Parser->table_names_size * sizeof(char*)
        );
        Parser->table_names[Parser->table_names_size - 1] = table_def_name;
    } else if (Parser->current_token->type == TOKEN_ID) {

        AST_T* table = Parser_Parse_Id(Parser, Scope);

        if (table->type != AST_TABLE) {
            printf("Tripped on table assignment, not a table/invalid type (at line %d)\n", Parser->current_token->line);
            exit(1);
        };

        AST_T* true_val = Scope_Get_Table_Definition(Scope, table->table_name);

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
        class_def->init_args_size += 1;
        class_def->init_args[class_def->init_args_size - 1] = arg;

        while (Parser->current_token->type == TOKEN_COMMA) {
            Parser_Eat(Parser, TOKEN_COMMA);
            class_def->init_args_size += 1;
            class_def->init_args = realloc(
                class_def->init_args,
                class_def->init_args_size * sizeof(struct AST_STRUCT*)
            );
            AST_T* arg2 = Parser_Parse_Variable(Parser, Scope);
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
    class_def->class_definition_name = class_def_name;
    class_def->scope=Scope;

    class_def->class_definition_value = calloc(1, sizeof(struct AST_STRUCT*));
    class_def->class_definition_value_name = calloc(1, sizeof(struct AST_STRUCT*));
    class_def->class_size = 0;

    int first_is_init = (strcmp(Parser->current_token->value, "init") == 0);
    if (first_is_init) {
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
        return Parser_Parse_Function_Call(Parser, Scope);
    }

    if (Parser->current_token->type == TOKEN_ID) {
        // ClassName instance_name; -- declare a new independent instance of this class
        char* instance_name = Parser->current_token->value;
        Parser_Eat(Parser, TOKEN_ID);

        AST_T* ast_instantiation = Init_AST(AST_CLASS_INSTANTIATION);
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
    ast_class->class_name = token_value;
    ast_class->type = AST_CLASS;

    ast_class->scope=Scope;
    return ast_class;
};

AST_T* Parser_Parse_Function_Definition(Parser_T* Parser, Scope_T* Scope) {
    AST_T* ast = Init_AST(AST_FUNCTION_DEFINITION);
    Parser_Eat(Parser, TOKEN_ID); // function
    char* func_name = Parser->current_token->value;
    ast->function_definition_name = calloc(
        strlen(func_name)+1,
        sizeof(char)
    );
    strcpy(ast->function_definition_name, func_name);
    Parser_Eat(Parser, TOKEN_ID); // function name
    Parser_Eat(Parser, TOKEN_LPAREN);

    if (Parser->current_token->type != TOKEN_RPAREN) {

        ast->function_definition_args = calloc(1, sizeof(struct AST_STRUCT*));
            AST_T* arg = Parser_Parse_Variable(Parser, Scope);
            ast->function_definition_args_size += 1;
            ast->function_definition_args[ast->function_definition_args_size-1] = arg;

        while (Parser->current_token->type == TOKEN_COMMA) {
            Parser_Eat(Parser, TOKEN_COMMA);
            ast->function_definition_args_size += 1;

            ast->function_definition_args = realloc(
                ast->function_definition_args,
                ast->function_definition_args_size * sizeof(struct AST_STRUCT*)
            );
            AST_T* arg = Parser_Parse_Variable(Parser, Scope);
            ast->function_definition_args[ast->function_definition_args_size-1] = arg;
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
    function_call->function_call_name = Parser->previous_token->value;
    Parser_Eat(Parser, TOKEN_LPAREN);

    function_call->function_call_arguments = NULL;
    function_call->function_call_arguments_size = 0;

    if (Parser->current_token->type != TOKEN_RPAREN) {
        function_call->function_call_arguments = calloc(1, sizeof(struct AST_STRUCT*));
        function_call->function_call_arguments_size += 1;

        AST_T* ast_expr = Parser_Parse_Expr(Parser, Scope);
        function_call->function_call_arguments[0] = ast_expr;

        while (Parser->current_token->type == TOKEN_COMMA) {
            Parser_Eat(Parser, TOKEN_COMMA);

            AST_T* ast_expr2 = Parser_Parse_Expr(Parser, Scope);
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

AST_T* Parser_Parse_If(Parser_T* Parser, Scope_T* Scope) {
    AST_T* ast = Init_AST(AST_IF);
    Parser_Eat(Parser, TOKEN_ID); // if
    Parser_Eat(Parser, TOKEN_LPAREN);

    ast->if_condition = Parser_Parse_Expr(Parser, Scope);

    Parser_Eat(Parser, TOKEN_RPAREN);
    Parser_Eat(Parser, TOKEN_LCURLY); // if compound

    ast->if_body = Parser_Parse_Statements(Parser, Scope);

    Parser_Eat(Parser, TOKEN_RCURLY); // if compound
    return ast;
};
AST_T* Parser_Parse_If_Else(Parser_T* Parser, Scope_T* Scope) {
    AST_T* ast = Init_AST(AST_IF_ELSE);
    Parser_Eat(Parser, TOKEN_ID); // if
    Parser_Eat(Parser, TOKEN_LPAREN);

    ast->if_condition = Parser_Parse_Expr(Parser, Scope);

    Parser_Eat(Parser, TOKEN_RPAREN);
    Parser_Eat(Parser, TOKEN_LCURLY); // if compound

    ast->if_body = Parser_Parse_Statements(Parser, Scope);

    Parser_Eat(Parser, TOKEN_RCURLY); // if compound

    Parser_Eat(Parser, TOKEN_ID); // else
    Parser_Eat(Parser, TOKEN_LCURLY);

    ast->if_else_body = Parser_Parse_Statements(Parser, Scope);

    Parser_Eat(Parser, TOKEN_RCURLY); // else compound
    return ast;
};

AST_T* Parser_Parse_Checks(Parser_T* Parser, Scope_T* Scope) {
    AST_T* ast = Init_AST(AST_CHECKS);
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
    ast->scope = Scope;
    Parser_Eat(Parser, TOKEN_ID); // for
    Parser_Eat(Parser, TOKEN_LPAREN);

    ast->for_variable = Parser_Parse_Expr(Parser, Scope);
    Parser_Eat(Parser, TOKEN_COLON);

    ast->for_condition = Parser_Parse_Expr(Parser, Scope);
    Parser_Eat(Parser, TOKEN_COLON);

    ast->for_does_at_end = Parser_Parse_Expr(Parser, Scope);

    Parser_Eat(Parser, TOKEN_RPAREN);
    Parser_Eat(Parser, TOKEN_LCURLY);

    ast->for_body = Parser_Parse_Statements(Parser, Scope);

    Parser_Eat(Parser, TOKEN_RCURLY);
    return ast;
};
AST_T* Parser_Parse_While(Parser_T* Parser, Scope_T* Scope) {
    AST_T* ast = Init_AST(AST_WHILE);
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
    ast->return_value = Parser_Parse_Expr(Parser, Scope);
    ast->scope = Scope;
    return ast;
};

AST_T* Parser_Parse_Include(Parser_T* Parser, Scope_T* Scope) {
    Parser_Eat(Parser, TOKEN_ID); // include
    AST_T* path_node = Parser_Parse_String(Parser, Scope);
    char* requested_path = path_node->string_value;

    char* path = join_path(Parser->current_dir, requested_path);

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
        printf("Tripped on include, could not read file '%s' (at line %d)\n", path, Parser->current_token->line);
        exit(1);
    }

    Lexer_T* saved_lexer = Parser->lexer;
    Token_T* saved_current = Parser->current_token;
    Token_T* saved_previous = Parser->previous_token;
    int saved_line = Parser->current_token->line;
    int saved_col = Parser->current_token->col;
    char* saved_path = Parser->current_dir;

    Lexer_T* include_lexer = Init_Lexer(contents);
    Parser->lexer = include_lexer;
    current_line = 1;
    current_col = 1;
    Parser->current_token = Lexer_Get_Next_Token(include_lexer);
    Parser->previous_token = Parser->current_token;
    Parser->current_dir = get_directory(path);

    AST_T* included_statements = Parser_Parse_Statements(Parser, Scope);

    if (Parser->current_token->type != TOKEN_EOF) {
        printf("Tripped on include, unexpected trailing token in included file '%s', at line %d\n", path, Parser->current_token->line);
        exit(1);
    };

    Parser->lexer = saved_lexer;
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

    Parser_Eat(Parser, TOKEN_STRING);

    ast_string->scope=Scope;

    return ast_string;
};
AST_T* Parser_Parse_Number(Parser_T* Parser, Scope_T* Scope){
    AST_T* ast_number = Init_AST(AST_NUMBER);
    ast_number->number_value = strtof(Parser->current_token->value, NULL);

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
    } else if (strcmp(Parser->current_token->value, "func") == 0) {
        return Parser_Parse_Function_Definition(Parser, Scope);
    } else if (strcmp(Parser->current_token->value, "for") == 0) {
        return Parser_Parse_For(Parser, Scope);
    } else if (strcmp(Parser->current_token->value, "while") == 0) {
        return Parser_Parse_While(Parser, Scope);
    } else if (strcmp(Parser->current_token->value, "if") == 0) {
        return Parser_Parse_If(Parser, Scope);
    } else if (strcmp(Parser->current_token->value, "ifelse") == 0) {
        return Parser_Parse_If_Else(Parser, Scope);
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
