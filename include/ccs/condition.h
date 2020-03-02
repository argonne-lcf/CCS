#ifndef _CCS_CONDITION_H
#define _CCS_CONDITION_H

enum ccs_expression_type_e {
	CCS_EQUALS,
	CCS_LESS_THAN,
	CCS_GREATER_THAN,
	CCS_IN,
	CCS_NOT,
	CCS_AND,
	CCS_OR,
	CCS_CONDITION_TYPE_MAX,
	CCS_CONDITION_FORCE_32BIT = INT_MAX
};

typedef enum ccs_expression_type_e ccs_expression_type_t;

// Expressions
extern ccs_error_t
ccs_create_binary_expression(ccs_expression_type_t  expresion_type,
                             ccs_datum_t            value_left,
                             ccs_datum_t            value_right,
                             ccs_expression_t      *expression_ret);
extern ccs_error_t
ccs_create_equals_expression(ccs_datum_t       value_left,
                             ccs_datum_t       value_right,
                             ccs_expression_t *expression_ret);

extern ccs_error_t
ccs_create_less_than_expression(ccs_datum_t       value_left,
                                ccs_datum_t       value_right,
                                ccs_expression_t *expression_ret);

extern ccs_error_t
ccs_create_greater_than_expression(ccs_datum_t       value_left,
                                   ccs_datum_t       value_right,
                                   ccs_expression_t *expression_ret);

extern ccs_error_t
ccs_create_in_expression(size_t            num_values,
                         ccs_datum_t      *values,
                         ccs_expression_t *expression_ret);
extern ccs_error_t
ccs_create_not_expression(ccs_expression_t  expression,
                          ccs_expression_t *expression_ret);

extern ccs_error_t
ccs_create_conjunction_expression(ccs_expression_type_t expresion_type,
                                  size_t                num_expressions,
                                  ccs_expression_t     *expressions,
                                  ccs_expression_t     *expressions_ret);

extern ccs_error_t
ccs_create_and_expression(size_t            num_expressions,
                          ccs_expression_t *expressions,
                          ccs_expression_t *expressions_ret);

extern ccs_error_t
ccs_create_or_expression(size_t            num_expressions,
                         ccs_expression_t *expressions,
                         ccs_expression_t *expressions_ret);

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

#endif //_CCS_CONDITION_H
