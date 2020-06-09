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
ccs_create_binary_expression(ccs_expression_type_t  type,
                             ccs_datum_t            node_left,
                             ccs_datum_t            node_right,
                             ccs_expression_t      *expression_ret);

extern ccs_error_t
ccs_create_unary_expression(ccs_expression_type_t  type,
                            ccs_datum_t            node,
                            ccs_expression_t      *expression_ret);

extern ccs_error_t
ccs_create_expression(ccs_expression_type_t  type,
	              size_t                 num_nodes,
                      ccs_datum_t           *nodes,
                      ccs_expression_t      *expression_ret);

extern ccs_error_t
ccs_expression_get_type(ccs_expression_t       expression,
                        ccs_expression_type_t *type_ret);

extern ccs_error_t
ccs_expression_get_num_nodes(ccs_expression_t  expression,
                             size_t           *num_nodes_ret);

extern ccs_error_t
ccs_expression_get_nodes(ccs_expression_t  expression,
                         size_t            num_nodes,
                         ccs_datum_t      *nodes,
                         size_t           *num_nodes_ret);

extern ccs_error_t
ccs_expression_eval(ccs_expression_t  expression,
                    ccs_context_t     context,
                    ccs_datum_t      *values,
                    ccs_datum_t      *result);

extern ccs_error_t
ccs_expression_list_eval_node(ccs_expression_t  expression,
                              ccs_context_t     context,
                              ccs_datum_t      *values,
                              size_t            index,
                              ccs_datum_t      *result);

//uniq and sorted list of hyperparameters handle
extern ccs_error_t
ccs_expression_get_hyperparameters(ccs_expression_t      expression,
                                   size_t                num_hyperparameters,
                                   ccs_hyperparameter_t *hyperparameters,
                                   size_t               *num_hyperparameters_ret);

extern ccs_error_t
ccs_expression_check_context(ccs_expression_t expression,
                             ccs_context_t    context);
#ifdef __cplusplus
}
#endif

#endif //_CCS_CONDITION_H
