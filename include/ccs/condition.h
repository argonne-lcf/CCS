#ifndef _CCS_CONDITION_H
#define _CCS_CONDITION_H

#ifdef __cplusplus
extern "C" {
#endif

enum ccs_expression_type_e {
	CCS_OR = 0,
	CCS_AND,
	CCS_EQUAL,
	CCS_NOT_EQUAL,
	CCS_LESS,
	CCS_GREATER,
	CCS_LESS_OR_EQUAL,
	CCS_GREATER_OR_EQUAL,
	CCS_IN,
	CCS_ADD,
	CCS_SUBSTRACT,
	CCS_MULTIPLY,
	CCS_DIVIDE,
	CCS_MODULO,
	CCS_POSITIVE,
	CCS_NEGATIVE,
	CCS_NOT,
	CCS_LIST,
	CCS_EXPRESSION_TYPE_MAX,
	CCS_EXPRESSION_FORCE_32BIT = INT_MAX
};

typedef enum ccs_expression_type_e ccs_expression_type_t;

// Precedence classes: least from most (taken from C)
// Needed to parentheses correctly
// 0 : OR
// 1 : AND
// 2 : EQUAL, NOT_EQUAL
// 3 : LESS, GREATER, LESS_OR_EQUAL, GREATER_OR_EQUAL
// 4 : IN
// 5 : ADD, SUBSTRACT
// 6 : MULTIPLY, DIVIDE, MODULO
// 7 : POSITIVE, NEGATIVE, NOT
// max - 1: LIST
// max : LITERAL, VARIABLE, HYPERPARAMETER

// One for each expression type:
extern const int ccs_expression_precedence[];

extern const char *ccs_expression_symbols[];

extern const int ccs_expression_arity[];

// Expressions
extern ccs_error_t
ccs_create_binary_expression(ccs_expression_type_t  expresion_type,
                             ccs_datum_t            node_left,
                             ccs_datum_t            node_right,
                             ccs_expression_t      *expression_ret);

extern ccs_error_t
ccs_create_unary_expression(ccs_expression_type_t  expression_type,
                            ccs_datum_t            node,
                            ccs_expression_t      *expression_ret);

extern ccs_error_t
ccs_create_epression(ccs_expression_type_t  expression_type,
	             size_t                 num_nodes,
                     ccs_datum_t           *nodes,
                     ccs_expression_t      *expression_ret);

extern ccs_error_t
ccs_eval_expression(ccs_expression_t           expression,
                    ccs_configuration_space_t  context,
                    ccs_datum_t               *values,
                    ccs_datum_t               *result);

// Conditions
extern ccs_error_t
ccs_create_condition(ccs_hyperparameter_t  hyperparameter,
                     ccs_expression_t      expression,
                     ccs_condition_t      *condition_ret);

// Forbidden Clause
extern ccs_error_t
ccs_create_forbidden_clause(ccs_hyperparameter_t    hyperparameter,
                            ccs_expression_t        expression,
                            ccs_forbidden_clause_t *forbidden_clause_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_CONDITION_H
