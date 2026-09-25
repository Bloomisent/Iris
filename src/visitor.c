#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>

#include "include/scope.h"
#include "include/visitor.h"
#include "include/builtin.h"
#include "include/token.h"

extern int current_line;

static int is_truthy(AST_T* var){
    int equal = 0;
    switch (var->type) {
        case AST_NUMBER: equal = (var->number_value != 0); break;
        case AST_STRING: equal = (var->string_value != (void*)0 && var->string_value[0] != '\0'); break;
        case AST_BOOL: equal = (var->bool_value != 0); break;
        default:
            printf("Tripped on comparison, unsupported type %d for equality (at line %d)\n", var->type, current_line);
            exit(1);
    };
    return equal;
};

For_Tuple* get_for_data(Visitor_T* visitor, AST_T* node) {
    For_Tuple* Tuple = Init_For_Tuple();
    Tuple->condition_result = Visitor_Visit(visitor, node->for_condition);
    Tuple->for_var = Scope_Get_Variable_Definition(node->scope, node->for_variable->variable_name);
    Tuple->for_var2 = Visitor_Visit(visitor, node->for_variable);
    Tuple->for_end_do = Visitor_Visit(visitor, node->for_does_at_end);
    return Tuple;
};

While_Tuple* get_while_data(Visitor_T* visitor, AST_T* node) {
    While_Tuple* Tuple = Init_While_Tuple();
    Tuple->condition_result = Visitor_Visit(visitor, node->while_condition);
    return Tuple;
};

Visitor_T* Init_Visitor(){
    Visitor_T* visitor = calloc(1, sizeof(struct VISITOR_STRUCT));
    visitor->returning = 0;
    visitor->call_depth = 0;
    return visitor;
};

For_Tuple* Init_For_Tuple(){
    For_Tuple* tuple = calloc(1, sizeof(struct FOR_TUPLE_STRUCT));
    return tuple;
};

While_Tuple* Init_While_Tuple(){
    While_Tuple* tuple = calloc(1, sizeof(struct WHILE_TUPLE_STRUCT));
    return tuple;
};

AST_T* Visitor_Visit(Visitor_T* visitor, AST_T* node){
    if (node == (void*)0) {
        return Init_AST(AST_NOOP);
    }
    switch (node->type) {
        case AST_VARIABLE_DEFINITION: return VV_Variable_Definition(visitor, node); break;
        case AST_TABLE_DEFINITION: return VV_Table_Definition(visitor, node); break;
        case AST_DICTIONARY_DEFINITION: return VV_Dict_Definition(visitor, node); break;
        case AST_CLASS_DEFINITION: return VV_Class_Definition(visitor, node); break;
        case AST_FUNCTION_DEFINITION: return VV_Function_Definition(visitor, node); break;
        case AST_IF: return VV_If(visitor, node); break;
        case AST_IF_ELSE: return VV_If_Else(visitor, node); break;
        case AST_CHECKS: return VV_Checks(visitor, node); break;
        case AST_FOR: return VV_For(visitor, node); break;
        case AST_WHILE: return VV_While(visitor, node); break;
        case AST_BINOP: return VV_BinOp(visitor, node); break;
        case AST_DOT: return VV_Dot(visitor, node); break;
        case AST_RETURN: return VV_Return(visitor, node); break;
        case AST_ASSIGNMENT: return VV_Assignment(visitor, node); break;
        case AST_CLASS_INSTANTIATION: return VV_Class_Instantiation(visitor, node); break;
        case AST_INIT_CALL: return VV_Init_Call(visitor, node); break;
        case AST_VARIABLE: return VV_Variable(visitor, node); break;
        case AST_STRING: return VV_String(visitor, node); break;
        case AST_NUMBER: return VV_Number(visitor, node); break;
        case AST_BOOL: return VV_Bool(visitor, node); break;
        case AST_TABLE: return VV_Table(visitor, node); break;
        case AST_DICTIONARY: return VV_Dict(visitor, node); break;
        case AST_CLASS: return VV_Class(visitor, node); break;
        case AST_FUNCTION_CALL: return VV_Function_Call(visitor, node); break;
        case AST_UNARY_NOT: return VV_Unary_Not(visitor, node); break;
        case AST_COMPOUND: return VV_Compound(visitor, node); break;
        case AST_NOOP: return node; break;
    };

    printf("Tripped on statement, enum type is '%d' (at line %d)\n", node->type, current_line);
    exit(1);
    return Init_AST(AST_NOOP);
};
AST_T* VV_Variable_Definition(Visitor_T* visitor, AST_T* node) {
    AST_T* evaluated_value = Visitor_Visit(visitor, node->variable_definition_value);

    AST_T* vdef = Init_AST(AST_VARIABLE_DEFINITION);
    vdef->variable_definition_variable_name = node->variable_definition_variable_name;
    vdef->variable_definition_value = evaluated_value;
    vdef->scope = node->scope;

    Scope_Add_Variable_Definition(node->scope, vdef);
    return node;
};
AST_T* VV_Variable(Visitor_T* visitor, AST_T* node) {
    if (strcmp(node->variable_name, "true") == 0) {
        AST_T* vdef = Init_AST(AST_BOOL);
        vdef->bool_value = true;
        return Visitor_Visit(visitor, vdef);
    } else if (strcmp(node->variable_name, "false") == 0) {
        AST_T* vdef = Init_AST(AST_BOOL);
        vdef->bool_value = false;
        return Visitor_Visit(visitor, vdef);
    } else {
        AST_T* vdef = Scope_Get_Variable_Definition(node->scope, node->variable_name);

        if (vdef != (void*)0) {
            return Visitor_Visit(visitor, vdef->variable_definition_value);
        } else {
            printf("Tripped on undefined variable '%s' (at line %d)\n", node->variable_name, current_line);
            exit(1);
        }
    }
    return node;
};
AST_T* VV_If(Visitor_T* visitor, AST_T* node) {
    AST_T* condition_result = Visitor_Visit(visitor, node->if_condition);

    int is_truthy = 0;
    switch (condition_result->type) {
        case AST_NUMBER: is_truthy = (condition_result->number_value != 0.0f); break;
        case AST_STRING:  is_truthy = (condition_result->string_value != (void*)0 &&
                                        strlen(condition_result->string_value) > 0); break;
        case AST_BOOL:  is_truthy = (condition_result->bool_value != false); break;
        default: is_truthy = 0; break;
    }

    if (is_truthy) {
        return Visitor_Visit(visitor, node->if_body);
    }
    return Init_AST(AST_NOOP);
};
AST_T* VV_If_Else(Visitor_T* visitor, AST_T* node) {
    AST_T* condition_result = Visitor_Visit(visitor, node->if_condition);

    int is_truthy = 0;
    switch (condition_result->type) {
        case AST_NUMBER: is_truthy = (condition_result->number_value != 0.0f); break;
        case AST_STRING:  is_truthy = (condition_result->string_value != (void*)0 &&
                                        strlen(condition_result->string_value) > 0); break;
        case AST_BOOL:  is_truthy = (condition_result->bool_value != false); break;
        default: is_truthy = 0; break;
    }

    if (is_truthy) {
        return Visitor_Visit(visitor, node->if_body);
    };
    return Visitor_Visit(visitor, node->if_else_body);
};
AST_T* VV_Checks(Visitor_T* visitor, AST_T* node) {
    AST_T* condition_var = Visitor_Visit(visitor, node->checks_base_var);

    int is_truthy = 0;

    for (int i=0;i<node->checks_condition_size;i++) {
        AST_T* condition_to_check = Visitor_Visit(visitor, node->checks_condition_body[i]);
        if (condition_var->type != condition_to_check->type) {
            printf("Tripped on checks, incompatible types (at line %d)\n", current_line);
            exit(1);
        }
        switch (condition_var->type) {
            case AST_NUMBER: is_truthy = (condition_to_check->number_value == condition_var->number_value); break;
            case AST_STRING:  is_truthy = (condition_to_check->string_value != (void*)0 &&
                                            strcmp(condition_to_check->string_value, condition_var->string_value) == 0); break;
            case AST_BOOL:  is_truthy = (condition_to_check->bool_value == condition_var->bool_value); break;
            default: is_truthy = 0; break;
        }
        if (is_truthy) {
            return Visitor_Visit(visitor, node->checks_do_body[i]);
        }
    };

    return Init_AST(AST_NOOP);
};
AST_T* VV_For(Visitor_T* visitor, AST_T* node) {
    char* loop_var_name;

    if (node->for_variable->type == AST_VARIABLE_DEFINITION) {
        AST_T* init_result = Visitor_Visit(visitor, node->for_variable);
        loop_var_name = init_result->variable_definition_variable_name;
    } else if (node->for_variable->type == AST_VARIABLE) {
        loop_var_name = node->for_variable->variable_name;
        if (Scope_Get_Variable_Definition(node->scope, loop_var_name) == (void*)0) {
            printf("Tripped on for loop, undefined variable '%s' (at line %d)\n", loop_var_name, current_line);
            exit(1);
        }
    } else {
        printf("Tripped on for loop, loop variable must be a 'var' definition or an existing variable (at line %d)\n", current_line);
        exit(1);
    }

    AST_T* end_do = Visitor_Visit(visitor, node->for_does_at_end);
    if (end_do->type != AST_STRING) {
        printf("Tripped on for loop, unexpected argument types received (at line %d)\n", current_line);
        exit(1);
    }

    bool minus;
    if (strcmp(end_do->string_value, "++") == 0) {
        minus = false;
    } else if (strcmp(end_do->string_value, "--") == 0) {
        minus = true;
    } else {
        printf("Tripped on for loop, unknown step operator '%s' (at line %d)\n", end_do->string_value, current_line);
        exit(1);
    }

    bool condition_true() {
        AST_T* cond = Visitor_Visit(visitor, node->for_condition);
        switch (cond->type) {
            case AST_NUMBER: return cond->number_value != 0.0f;
            case AST_STRING:  return cond->string_value != (void*)0 && strlen(cond->string_value) > 0;
            case AST_BOOL:    return cond->bool_value != false;
            default: return false;
        }
    };

    void set_loop_var(float value) {
        AST_T* vdef = Init_AST(AST_VARIABLE_DEFINITION);
        vdef->variable_definition_variable_name = loop_var_name;
        AST_T* num = Init_AST(AST_NUMBER);
        num->number_value = value;
        vdef->variable_definition_value = num;
        vdef->scope = node->scope;
        Scope_Add_Variable_Definition(node->scope, vdef);
    };

    AST_T* current = Scope_Get_Variable_Definition(node->scope, loop_var_name);
    float counter = current->variable_definition_value->number_value;

    AST_T* last_var = Init_AST(AST_NOOP);
    while (condition_true()) {
        last_var = Visitor_Visit(visitor, node->for_body);
        if (visitor->returning) {
            break;
        }
        counter += minus ? -1.0f : 1.0f;
        set_loop_var(counter);
    };

    return last_var;
};
AST_T* VV_While(Visitor_T* visitor, AST_T* node) {
    While_Tuple* ttt = get_while_data(visitor, node);

    bool get_res() {
        ttt->condition_result = Visitor_Visit(visitor, node->while_condition);
        bool is_truthy = false;
        switch (ttt->condition_result->type) {
            case AST_NUMBER: is_truthy = (ttt->condition_result->number_value != 0.0f); break;
            case AST_STRING:  is_truthy = (ttt->condition_result->string_value != (void*)0 &&
                                            strlen(ttt->condition_result->string_value) > 0); break;
            case AST_BOOL:  is_truthy = (ttt->condition_result->bool_value != false); break;
            default: is_truthy = false; break;
        };
        return is_truthy;
    };
    AST_T* last_var = Init_AST(AST_NOOP);

    while (get_res() == true) {
        last_var = Visitor_Visit(visitor, node->while_body);
        if (visitor->returning) {
            break; // a 'return' fired inside the loop body -- stop iterating
        }
    };
    free(ttt);
    return last_var;
};
AST_T* VV_BinOp(Visitor_T* visitor, AST_T* node) {
    int op = node->binop_op;

    if (op == TOKEN_AND || op == TOKEN_OR) {
        AST_T* left = Visitor_Visit(visitor, node->binop_left);
        int lv = is_truthy(left);

        AST_T* result = Init_AST(AST_BOOL);
        if (op == TOKEN_AND && !lv) { result->bool_value = 0; return result; }
        if (op == TOKEN_OR  &&  lv) { result->bool_value = 1; return result; }

        AST_T* right = Visitor_Visit(visitor, node->binop_right);
        result->bool_value = is_truthy(right);
        return result;
    }

    AST_T* left = Visitor_Visit(visitor, node->binop_left);
    AST_T* right = Visitor_Visit(visitor, node->binop_right);

    int is_comparison = (op == TOKEN_GT || op == TOKEN_LT || op == TOKEN_GTE ||
                         op == TOKEN_LTE || op == TOKEN_EQ || op == TOKEN_NEQ);

    if (is_comparison) {
        if (left->type != right->type) {
            printf("Tripped on comparison, incomparable types (%d, %d) (at line %d)\n", left->type, right->type, current_line);
            exit(1);
        }

        AST_T* result = Init_AST(AST_BOOL);

        if (op == TOKEN_EQ || op == TOKEN_NEQ) {
            int equal = 0;
            switch (left->type) {
                case AST_NUMBER: equal = (left->number_value == right->number_value); break;
                case AST_STRING: equal = (left->string_value != (void*)0 &&
                                          right->string_value != (void*)0 &&
                                          strcmp(left->string_value, right->string_value) == 0); break;
                case AST_BOOL:   equal = (left->bool_value == right->bool_value); break;
                default:
                    printf("Tripped on comparison, unsupported type %d for equality (at line %d)\n", left->type, current_line);
                    exit(1);
            }
            result->bool_value = (op == TOKEN_EQ) ? equal : !equal;
            return result;
        }

        if (left->type != AST_NUMBER) {
            printf("Tripped on comparison, ordering operators require numbers (got type %d), at line %d\n", left->type, current_line);
            exit(1);
        }
        switch (op) {
            case TOKEN_GT:  result->bool_value = (left->number_value >  right->number_value); break;
            case TOKEN_LT:  result->bool_value = (left->number_value <  right->number_value); break;
            case TOKEN_GTE: result->bool_value = (left->number_value >= right->number_value); break;
            case TOKEN_LTE: result->bool_value = (left->number_value <= right->number_value); break;
        }
        return result;
    };

    if (left->type == AST_NUMBER && right->type == AST_NUMBER) {
        AST_T* result = Init_AST(AST_NUMBER);
        result->number_value = 0;
        switch (node->binop_op) {
            case TOKEN_PLUS:  result->number_value = left->number_value + right->number_value; break;
            case TOKEN_MINUS: result->number_value = left->number_value - right->number_value; break;
            case TOKEN_MULTIPLY:   result->number_value = left->number_value * right->number_value; break;
            case TOKEN_DIVIDE:
                if (right->number_value == 0.0f) {
                    printf("Tripped on binary operation, division by zero, at line %d\n", current_line);
                    exit(1);
                }
                result->number_value = left->number_value / right->number_value;
                break;
            case TOKEN_ADD:  result->number_value = left->number_value + right->number_value; break;
            case TOKEN_SUB: result->number_value = left->number_value - right->number_value; break;
            case TOKEN_MULT:   result->number_value = left->number_value * right->number_value; break;
            case TOKEN_DIV:
                if (right->number_value == 0.0f) {
                    printf("Tripped on binary operation, division by zero, at line %d\n", current_line);
                    exit(1);
                }
                result->number_value = left->number_value / right->number_value;
                break;
            case TOKEN_MODULO:
                if (right->number_value == 0.0f) {
                    printf("Tripped on binary operation, modulo by zero, at line %d\n", current_line);
                    exit(1);
                }
                result->number_value = fmodf(left->number_value, right->number_value);
                break;
            case TOKEN_EXPONENT: result->number_value = powf(left->number_value, right->number_value); break;
            default:
                printf("Tripped on binary operation, unsupported operator %d, at line %d\n", node->binop_op, current_line);
                exit(1);
        }
        return result;
    } else if (left->type == AST_STRING && right->type == AST_STRING && (node->binop_op == TOKEN_PLUS || node->binop_op == TOKEN_ADD)) {
        AST_T* result = Init_AST(AST_STRING);
        char* combined = calloc(strlen(left->string_value) + strlen(right->string_value) + 1, sizeof(char));
        strcpy(combined, left->string_value);
        strcat(combined, right->string_value);
        result->string_value = combined;
        return result;
    } else {
        printf("Tripped on binary operation, incompatible operand types (%d, %d), at line %d\n", left->type, right->type, current_line);
        exit(1);
    }

    return Init_AST(AST_NOOP);
};
AST_T* VV_Unary_Not(Visitor_T* visitor, AST_T* node) {
    AST_T* operand = Visitor_Visit(visitor, node->unary_operand);
    AST_T* result = Init_AST(AST_BOOL);
    result->bool_value = !is_truthy(operand);
    return result;
};
AST_T* VV_Dot(Visitor_T* visitor, AST_T* node) {
    AST_T* left = Visitor_Visit(visitor, node->dot_left);
    AST_T* right = Visitor_Visit(visitor, node->dot_right);

    if (right->type != AST_STRING) {
        printf("Tripped on dot operation, key must be a string (got type %d), at line %d\n", right->type, current_line);
        exit(1);
    }

    if (left->type == AST_DICTIONARY_DEFINITION) {
        for (size_t i = 0; i < left->dictionary_size; i++) {
            AST_T* key = Visitor_Visit(visitor, left->dictionary_definition_value_name[i]);
            if (key->type == AST_STRING && strcmp(key->string_value, right->string_value) == 0) {
                return Visitor_Visit(visitor, left->dictionary_definition_value[i]);
            }
        }
        printf("Tripped on dot operation, key '%s' not found in dictionary, at line %d\n", right->string_value, current_line);
        exit(1);
    } else if (left->type == AST_CLASS_DEFINITION) {
        if (strcmp(right->string_value, "init") == 0) {
            if (left->init_step == 0) {
                printf("Tripped on dot operation, class '%s' has no init defined, at line %d\n", left->class_definition_name, current_line);
                exit(1);
            }
            return Visitor_Visit(visitor, left->init_value);
        }
        for (size_t i = 0; i < left->class_size; i++) {
            AST_T* key = Visitor_Visit(visitor, left->class_definition_value_name[i]);
            if (key->type == AST_STRING && strcmp(key->string_value, right->string_value) == 0) {
                return Visitor_Visit(visitor, left->class_definition_value[i]);
            }
        }
        printf("Tripped on dot operation, key '%s' not found in class, at line %d\n", right->string_value, current_line);
        exit(1);
    } else {
        printf("Tripped on dot operation, invalid type accessed (%d), at line %d\n", left->type, current_line);
        exit(1);
    }

    return Init_AST(AST_NOOP);
};
AST_T* VV_Class_Instantiation(Visitor_T* visitor, AST_T* node) {
    AST_T* template_def = Scope_Get_Class_Definition(node->scope, node->instance_class_name);
    if (template_def == (void*)0) {
        printf("Tripped on class instantiation, undefined class '%s', at line %d\n", node->instance_class_name, current_line);
        exit(1);
    }

    AST_T* instance = Init_AST(AST_CLASS_DEFINITION);
    instance->class_definition_name = node->instance_variable_name;
    instance->scope = node->scope;
    instance->class_size = template_def->class_size;
    instance->init_step = template_def->init_step;
    instance->init_value = template_def->init_value;
    instance->init_args = template_def->init_args;
    instance->init_args_size = template_def->init_args_size;

    if (instance->class_size > 0) {
        instance->class_definition_value = calloc(instance->class_size, sizeof(struct AST_STRUCT*));
        instance->class_definition_value_name = calloc(instance->class_size, sizeof(struct AST_STRUCT*));
        for (size_t i = 0; i < instance->class_size; i++) {
            instance->class_definition_value[i] = template_def->class_definition_value[i];
            instance->class_definition_value_name[i] = template_def->class_definition_value_name[i];
        }
    }

    Scope_Add_Class_Definition(node->scope, instance);
    return instance;
};
AST_T* VV_Init_Call(Visitor_T* visitor, AST_T* node) {
    AST_T* instance = Visitor_Visit(visitor, node->init_call_instance);

    if (instance->type != AST_CLASS_DEFINITION) {
        printf("Tripped on init call, target is not a class instance (type %d), at line %d\n", instance->type, current_line);
        exit(1);
    }
    if (!instance->init_step || instance->init_value == (void*)0) {
        printf("Tripped on init call, class '%s' has no 'init' defined, at line %d\n", instance->class_definition_name, current_line);
        exit(1);
    }
    if (instance->init_args_size < 1) {
        printf("Tripped on init call, 'init' must declare at least a 'self' parameter, at line %d\n", current_line);
        exit(1);
    }

    size_t declared_param_count = instance->init_args_size;
    size_t user_arg_count = (size_t) node->function_call_arguments_size;

    if (user_arg_count != declared_param_count - 1) {
        printf(
            "Tripped on init call, expected %zu argument(s) but got %zu, at line %d\n",
            declared_param_count - 1,
            user_arg_count,
            current_line
        );
        exit(1);
    }

    Scope_T* call_scope = instance->scope;
    size_t saved_scope_size = call_scope->variable_definitions_size;

    // First declared parameter is 'self' by convention -- bind it to the instance itself.
    AST_T* self_param = instance->init_args[0];
    AST_T* self_vdef = Init_AST(AST_VARIABLE_DEFINITION);
    self_vdef->variable_definition_variable_name = self_param->variable_name;
    self_vdef->variable_definition_value = instance;
    Scope_Add_Variable_Definition(call_scope, self_vdef);

    // Remaining declared parameters bind to the evaluated call arguments, in order.
    for (size_t i = 1; i < declared_param_count; i++) {
        AST_T* param = instance->init_args[i];
        AST_T* arg_expr = node->function_call_arguments[i - 1];
        AST_T* evaluated = Visitor_Visit(visitor, arg_expr);

        AST_T* vdef = Init_AST(AST_VARIABLE_DEFINITION);
        vdef->variable_definition_variable_name = param->variable_name;
        vdef->variable_definition_value = evaluated;
        Scope_Add_Variable_Definition(call_scope, vdef);
    }

    AST_T* result = Visitor_Visit(visitor, instance->init_value);
    visitor->returning = 0;

    call_scope->variable_definitions_size = saved_scope_size;

    return result;
};
AST_T* VV_Return(Visitor_T* visitor, AST_T* node) {
    AST_T* result = Visitor_Visit(visitor, node->return_value);
    visitor->returning = 1;
    return result;
};
AST_T* VV_Assignment(Visitor_T* visitor, AST_T* node) {
    AST_T* target = node->assignment_target; // raw AST_DOT: unevaluated so we can reach its left/right

    if (target->type == AST_VARIABLE) {
        AST_T* value = Visitor_Visit(visitor, node->assignment_value);
        AST_T* variable = Scope_Get_Variable_Definition(target->scope, target->variable_name);

        if (variable == NULL){
            printf("Tripped on assignment, unknown variable '%s', at line %d\n", target->variable_name, current_line);
            exit(1);
        };

        variable->variable_definition_value = value;
        return value;
    };

    AST_T* left = Visitor_Visit(visitor, target->dot_left);   // the dict/class instance -- same live pointer stored in scope
    AST_T* key = Visitor_Visit(visitor, target->dot_right);

    if (key->type != AST_STRING) {
        printf("Tripped on assignment, key must be a string (got type %d), at line %d\n", key->type, current_line);
        exit(1);
    }

    AST_T* value = Visitor_Visit(visitor, node->assignment_value);

    if (left->type == AST_DICTIONARY_DEFINITION) {
        for (size_t i = 0; i < left->dictionary_size; i++) {
            AST_T* existing_key = Visitor_Visit(visitor, left->dictionary_definition_value_name[i]);
            if (existing_key->type == AST_STRING && strcmp(existing_key->string_value, key->string_value) == 0) {
                left->dictionary_definition_value[i] = value;
                return value;
            }
        }
        // key not found -- append a new field
        left->dictionary_size += 1;
        left->dictionary_definition_value_name = realloc(
            left->dictionary_definition_value_name,
            left->dictionary_size * sizeof(struct AST_STRUCT*)
        );
        left->dictionary_definition_value = realloc(
            left->dictionary_definition_value,
            left->dictionary_size * sizeof(struct AST_STRUCT*)
        );
        left->dictionary_definition_value_name[left->dictionary_size - 1] = key;
        left->dictionary_definition_value[left->dictionary_size - 1] = value;
        return value;
    } else if (left->type == AST_CLASS_DEFINITION) {
        for (size_t i = 0; i < left->class_size; i++) {
            AST_T* existing_key = Visitor_Visit(visitor, left->class_definition_value_name[i]);
            if (existing_key->type == AST_STRING && strcmp(existing_key->string_value, key->string_value) == 0) {
                left->class_definition_value[i] = value;
                return value;
            }
        }
        left->class_size += 1;
        left->class_definition_value_name = realloc(
            left->class_definition_value_name,
            left->class_size * sizeof(struct AST_STRUCT*)
        );
        left->class_definition_value = realloc(
            left->class_definition_value,
            left->class_size * sizeof(struct AST_STRUCT*)
        );
        left->class_definition_value_name[left->class_size - 1] = key;
        left->class_definition_value[left->class_size - 1] = value;
        return value;
    } else {
        printf("Tripped on assignment, invalid target type (%d) -- expected a dict or class instance, at line %d\n", left->type, current_line);
        exit(1);
    }

    return Init_AST(AST_NOOP);
};

AST_T* VV_Function_Definition(Visitor_T* visitor, AST_T* node) {
    Scope_Add_Function_Definition(node->scope, node);
    return node;
};
AST_T* VV_Function_Call(Visitor_T* visitor, AST_T* node) {
    if (strcmp(node->function_call_name, "print") == 0) {
        return builtin_function_print(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "gc") == 0) { // garbage collector
        return builtin_function_gc(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "gcStats") == 0) {
        return builtin_function_gc_stats(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "type") == 0) {
        return builtin_function_type(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "dict_get_index") == 0) {
        return builtin_function_dict_get_index(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "dict_get_from_index") == 0) {
        return builtin_function_dict_get_from_index(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "dict_set_index") == 0) {
        return builtin_function_dict_set_index(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "table_get_index") == 0) {
        return builtin_function_table_get_index(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "table_get_from_index") == 0) {
        return builtin_function_table_get_from_index(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "table_set_index") == 0) {
        return builtin_function_table_set_index(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "charAt") == 0) {
        return builtin_function_char_at(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "stredit") == 0) {
        return builtin_function_str_edit(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "ForEach") == 0) {
        return builtin_function_for_each(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "toNumber") == 0) {
        return builtin_function_to_number(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "toString") == 0) {
        return builtin_function_to_string(visitor, node->function_call_arguments, node->function_call_arguments_size);
     } else if (strcmp(node->function_call_name, "readFile") == 0) {
        return builtin_function_read_file(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "writeFile") == 0) {
        return builtin_function_write_file(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "input") == 0) {
        return builtin_function_input(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "floor") == 0) {
        return builtin_function_floor(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "windowCreate") == 0) {
        return builtin_function_window_create(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "windowGetTime") == 0) {
        return builtin_function_window_get_time(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "windowShouldClose") == 0) {
        return builtin_function_window_should_close(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "windowClear") == 0) {
        return builtin_function_window_clear(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "windowDrawRect") == 0) {
        return builtin_function_window_draw_rect(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "windowDrawText") == 0) {
        return builtin_function_window_draw_text(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "windowPresent") == 0) {
        return builtin_function_window_present(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "windowMouseX") == 0) {
        return builtin_function_window_mouse_x(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "windowMouseY") == 0) {
        return builtin_function_window_mouse_y(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "windowGetWidth") == 0) {
        return builtin_function_window_width(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "windowGetHeight") == 0) {
        return builtin_function_window_height(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "windowMousePressed") == 0) {
        return builtin_function_window_mouse_pressed(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "windowKeyPressed") == 0) {
        return builtin_function_window_key_pressed(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else if (strcmp(node->function_call_name, "windowClose") == 0) {
        return builtin_function_window_close(visitor, node->function_call_arguments, node->function_call_arguments_size);
    } else {
        AST_T* fdef = Scope_Get_Function_Definition(node->scope, node->function_call_name);

        if (fdef == (void*)0) {
            printf("Tripped on undefined method '%s', at line %d\n", node->function_call_name, current_line);
            exit(1);
        }
        if (node->function_call_arguments_size != fdef->function_definition_args_size) {
            printf("Tripped on function call '%s', expected %zu args, got %zu (at line %d)\n",
                   node->function_call_name, fdef->function_definition_args_size, node->function_call_arguments_size, current_line);
            exit(1);
        }
        Scope_T* call_scope = fdef->function_definition_body->scope;
        size_t saved_scope_size = call_scope->variable_definitions_size;

        AST_T** evaluated_args = (AST_T**) calloc(node->function_call_arguments_size, sizeof(struct AST_STRUCT*));
        for (int i=0;i<(int)node->function_call_arguments_size;i++){
            evaluated_args[i] = Visitor_Visit(visitor, node->function_call_arguments[i]);
        }

        for (int i=0;i<(int)node->function_call_arguments_size;i++){
            AST_T* ast_var = (AST_T*) fdef->function_definition_args[i];

            AST_T* ast_vardef = Init_AST(AST_VARIABLE_DEFINITION);
            ast_vardef->variable_definition_value = evaluated_args[i];
            ast_vardef->variable_definition_variable_name = (char*) calloc(strlen(ast_var->variable_name) + 1, sizeof(char));
            strcpy(ast_vardef->variable_definition_variable_name, ast_var->variable_name);
            Scope_Add_Variable_Definition(call_scope, ast_vardef);
        }
        free(evaluated_args);

        AST_T* result = Visitor_Visit(visitor, fdef->function_definition_body);
        visitor->returning = 0;

        call_scope->variable_definitions_size = saved_scope_size;

        return result;
    }
};
AST_T* VV_Table_Definition(Visitor_T* visitor, AST_T* node) {
    Scope_Add_Table_Definition(node->scope, node);
    return node;
};
AST_T* VV_Table(Visitor_T* visitor, AST_T* node) {
    AST_T* tdef = Scope_Get_Table_Definition(node->scope, node->table_name);

    if (tdef != (void*)0) {
        return tdef;
    } else {
        printf("Tripped on undefined table '%s', at line %d\n", node->table_name, current_line);
        exit(1);
    }
    return node;
};
AST_T* VV_Dict_Definition(Visitor_T* visitor, AST_T* node) {
    Scope_Add_Dictionary_Definition(node->scope, node);
    return node;
};
AST_T* VV_Dict(Visitor_T* visitor, AST_T* node) {
    AST_T* ddef = Scope_Get_Dictionary_Definition(node->scope, node->dictionary_name);

    if (ddef != (void*)0) {
        return ddef;
    } else {
        printf("Tripped on undefined dictionary '%s', at line %d\n", node->dictionary_name, current_line);
        exit(1);
    }
    return node;
};
AST_T* VV_Class_Definition(Visitor_T* visitor, AST_T* node) {
    Scope_Add_Class_Definition(node->scope, node);
    return node;
};
AST_T* VV_Class(Visitor_T* visitor, AST_T* node) {
    AST_T* tdef = Scope_Get_Class_Definition(node->scope, node->class_name);

    if (tdef != (void*)0) {
        return tdef;
    } else {
        printf("Tripped on undefined class '%s', at line %d\n", node->class_name, current_line);
        exit(1);
    }
    return node;
};

AST_T* VV_String(Visitor_T* visitor, AST_T* node) {
    return node;
};
AST_T* VV_Number(Visitor_T* visitor, AST_T* node) {
    return node;
};
AST_T* VV_Bool(Visitor_T* visitor, AST_T* node) {
    return node;
};
AST_T* VV_Compound(Visitor_T* visitor, AST_T* node) {
    AST_T* result = Init_AST(AST_NOOP);
    for (int i=0;i<node->compound_size;i++) {
        result = Visitor_Visit(visitor, node->compound_value[i]);
        if (visitor->returning) {
            return result;   // a return fired somewhere in this statement (directly or nested in if/ifelse) - stop
        }
    };
    return Init_AST(AST_NOOP);
};


void Visitor_Clean(Visitor_T* visitor, AST_T* root){
    free(visitor);
    free(root);
}
