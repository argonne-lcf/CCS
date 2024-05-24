#ifndef _CCS_CONDITION_H
#define _CCS_CONDITION_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file expression.h
 * An expression in CCS is a combination of constants, variables (parameters),
 * and operators. Expressions are usually evaluated in the context of a binding
 * where parameters are associated values. For convinience CCS suggests a
 * grammar that can be used to create an expression parser.
 */

/**
 * Supported expression types.
 */
enum ccs_expression_type_e {
	/** Or boolean operator */
	CCS_EXPRESSION_TYPE_OR = 0,
	/** And boolean operator */
	CCS_EXPRESSION_TYPE_AND,
	/** Equality test operator */
	CCS_EXPRESSION_TYPE_EQUAL,
	/** Inequality test operator */
	CCS_EXPRESSION_TYPE_NOT_EQUAL,
	/** Lesser than comparison operator */
	CCS_EXPRESSION_TYPE_LESS,
	/** Greater than comparison operator */
	CCS_EXPRESSION_TYPE_GREATER,
	/** Lesser than or equal comparison operator */
	CCS_EXPRESSION_TYPE_LESS_OR_EQUAL,
	/** Greater than or equal comparison operator */
	CCS_EXPRESSION_TYPE_GREATER_OR_EQUAL,
	/** Addition operator */
	CCS_EXPRESSION_TYPE_ADD,
	/** Substraction operator */
	CCS_EXPRESSION_TYPE_SUBSTRACT,
	/** Multiplication operator */
	CCS_EXPRESSION_TYPE_MULTIPLY,
	/** Division operator */
	CCS_EXPRESSION_TYPE_DIVIDE,
	/** Modulo operator */
	CCS_EXPRESSION_TYPE_MODULO,
	/** Unary plus operator */
	CCS_EXPRESSION_TYPE_POSITIVE,
	/** Unary minus operator */
	CCS_EXPRESSION_TYPE_NEGATIVE,
	/** Not boolean operator */
	CCS_EXPRESSION_TYPE_NOT,
	/** List inclusion test operator */
	CCS_EXPRESSION_TYPE_IN,
	/** List */
	CCS_EXPRESSION_TYPE_LIST,
	/** Literal constant */
	CCS_EXPRESSION_TYPE_LITERAL,
	/** Variable */
	CCS_EXPRESSION_TYPE_VARIABLE,
	/** Guard */
	CCS_EXPRESSION_TYPE_MAX,
	/** Try forcing 32 bits value for bindings */
	CCS_EXPRESSION_FORCE_32BIT = INT32_MAX
};

/**
 * A commodity type to represent an expression type.
 */
typedef enum ccs_expression_type_e ccs_expression_type_t;

/**
 * An array of precedence of operators as defined by CCS grammar:
 *  - 0 : OR
 *  - 1 : AND
 *  - 2 : EQUAL, NOT_EQUAL
 *  - 3 : LESS, GREATER, LESS_OR_EQUAL, GREATER_OR_EQUAL
 *  - 4 : ADD, SUBSTRACT
 *  - 5 : MULTIPLY, DIVIDE, MODULO
 *  - 6 : POSITIVE, NEGATIVE, NOT
 *  - 7 : IN
 *  - max - 1: LIST
 *  - max : LITERAL, VARIABLE
 *
 * Those are similar to C's precedence
 */
extern const int                   ccs_expression_precedence[];

/**
 * Associativity of CCS operators:
 */
enum ccs_associativity_type_e {
	/** No associativity */
	CCS_ASSOCIATIVITY_TYPE_NONE = 0,
	/** left to right associativity */
	CCS_ASSOCIATIVITY_TYPE_LEFT_TO_RIGHT,
	/** right to left associativity */
	CCS_ASSOCIATIVITY_TYPE_RIGHT_TO_LEFT,
	/** Guard */
	CCS_ASSOCIATIVITY_TYPE_MAX,
	/** Try forcing 32 bits value for bindings */
	CCS_ASSOCIATIVITY_TYPE_FORCE_32BIT = INT32_MAX
};

/**
 * A commodity type to represent an associativity.
 */
typedef enum ccs_associativity_type_e ccs_associativity_type_t;

/**
 * An array of associativity of operators as defined by CCS grammar:
 *  - left: OR
 *  - left: AND
 *  - left: EQUAL, NOT_EQUAL
 *  - left: LESS, GREATER, LESS_OR_EQUAL, GREATER_OR_EQUAL
 *  - left: ADD, SUBSTRACT
 *  - left: MULTIPLY, DIVIDE, MODULO
 *  - right: POSITIVE, NEGATIVE, NOT
 *  - left: IN
 *  - left: LIST
 *  - none: LITERAL, VARIABLE
 */
extern const ccs_associativity_type_t ccs_expression_associativity[];

/**
 * An array of suggested symbols (NULL terminated strings) for CCS operators.
 *  - OR: ||
 *  - AND: &&
 *  - EQUAL: ==
 *  - NOT_EQUAL: !=
 *  - LESS: <
 *  - GREATER: >
 *  - LESS_OR_EQUAL: <=
 *  - GREATER_OR_EQUAL: >=
 *  - ADD: +
 *  - SUBSTRACT: -
 *  - MULTIPLY: *
 *  - DIVIDE: /
 *  - MODULO: %
 *  - POSITIVE: +
 *  - NEGATIVE: -
 *  - NOT: !
 *  - IN: #
 *  - LIST: NULL
 *  - LITERAL: NULL
 *  - VARIABLE: NULL
 */
extern const char                    *ccs_expression_symbols[];

/**
 * An array of arity of CCS operators
 *  - 2: OR
 *  - 2: AND
 *  - 2: EQUAL, NOT_EQUAL
 *  - 2: LESS, GREATER, LESS_OR_EQUAL, GREATER_OR_EQUAL
 *  - 2: ADD, SUBSTRACT
 *  - 2: MULTIPLY, DIVIDE, MODULO
 *  - 1: POSITIVE, NEGATIVE, NOT
 *  - 2: IN
 *  - -1: LIST
 *  - 0: LITERAL, VARIABLE
 */
extern const int                      ccs_expression_arity[];

/**
 * The different terminal types of ccs expressions.
 */
enum ccs_terminal_type_e {
	/** The #CCS_NONE_VAL value */
	CCS_TERMINAL_TYPE_NONE = 0,
	/** The #CCS_TRUE_VAL value */
	CCS_TERMINAL_TYPE_TRUE,
	/** The #CCS_FALSE_VAL value */
	CCS_TERMINAL_TYPE_FALSE,
	/** A #CCS_DATA_TYPE_STRING value */
	CCS_TERMINAL_TYPE_STRING,
	/** An identifer (name of a parameter) */
	CCS_TERMINAL_TYPE_IDENTIFIER,
	/** A #CCS_DATA_TYPE_INT value */
	CCS_TERMINAL_TYPE_INTEGER,
	/** A #CCS_DATA_TYPE_FLOAT value */
	CCS_TERMINAL_TYPE_FLOAT,
	/** Guard */
	CCS_TERMINAL_TYPE_MAX,
	/** Try forcing 32 bits value for bindings */
	CCS_TERMINAL_FORCE_32BIT = INT32_MAX
};

/**
 * A commodity type to represend CCS terminal types
 */
typedef enum ccs_terminal_type_e ccs_terminal_type_t;

/**
 * An array of integers defining terminal precedence in order to disambiguate
 * NONE, TRUE and FALSE from identifiers:
 *  - 0: STRING, IDENTIFIER, INTEGER, FLOAT
 *  - 1: NONE, TRUE, FALSE
 */
extern const int                 ccs_terminal_precedence[];

/**
 * An array of regexp that define terminals:
 *  - NONE: /none/
 *  - TRUE: /true/
 *  - FALSE: /false/
 *  - STRING:
 *  /"([^\0\t\n\r\f"\\\\]|\\\\[0tnrf"\\])+"|'([^\0\\t\\n\\r\\f'\\\\]|\\\\[0tnrf'\\\\])+'/
 *  - IDENTIFIER: /[a-zA-Z_][a-zA-Z_0-9]/
 *  - INTEGER: /-?[0-9]+/
 *  - FLOAT: /-?[0-9]+([eE][+-]?[0-9]+|\\.[0-9]+([eE][+-]?[0-9]+)?)/
 */
extern const char               *ccs_terminal_regexp[];

/**
 * An array of symbols (NULL terminated strings) for terminals that define them:
 *  - NONE: "none"
 *  - TRUE: "true"
 *  - FALSE: "false"
 *  - STRING: NULL
 *  - IDENTIFIER: NULL
 *  - INTEGER: NULL
 *  - FLOAT: NULL
 */
extern const char               *ccs_terminal_symbols[];

/**
 * Create a new expression.
 * @param[in] type the type of the expression
 * @param[in] num_nodes the number of the expression children nodes. Must be
 *                      compatible with the arity of the expression
 * @param[in] nodes an array of \p num_nodes expressions
 * @param[out] expression_ret a pointer to the variable that will hold the newly
 *                            created expression
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p type is not a valid CCS
 * expression type; or if \p num_nodes is not compatible with the arity of \p
 * type; or if one the nodes given is of type #CCS_DATA_TYPE_OBJECT but is
 * neither a #CCS_OBJECT_TYPE_PARAMETER nor a #CCS_OBJECT_TYPE_EXPRESSION; or
 * if one the nodes given node is not a type #CCS_DATA_TYPE_OBJECT,
 * #CCS_DATA_TYPE_NONE, #CCS_DATA_TYPE_INT, #CCS_DATA_TYPE_FLOAT,
 * #CCS_DATA_TYPE_BOOL, or #CCS_DATA_TYPE_STRING; or if \p expression_ret is
 * NULL
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if one the nodes given is of type
 * #CCS_DATA_TYPE_OBJECT but the object is not a valid CCS object
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was a lack of memory to
 * allocate the new expression
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_create_expression(
	ccs_expression_type_t type,
	size_t                num_nodes,
	ccs_datum_t          *nodes,
	ccs_expression_t     *expression_ret);

/**
 * Create a new binary expression. Convenience wrapper around
 * #ccs_create_expression for binary operators.
 * @param[in] type the type of the expression
 * @param[in] node_left left child node
 * @param[in] node_right right child node
 * @param[out] expression_ret a pointer to the variable that will hold the newly
 *             created expression
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p type is not a valid CCS
 * expression type; or if \p type arity is not 2; or if \p node_left or \p
 * node_right are of type #CCS_DATA_TYPE_OBJECT but are neither a
 * #CCS_OBJECT_TYPE_PARAMETER nor a #CCS_OBJECT_TYPE_EXPRESSION; or if \p
 * node_left or \p node_right are not of type #CCS_DATA_TYPE_OBJECT,
 * #CCS_DATA_TYPE_NONE, #CCS_DATA_TYPE_INT, #CCS_DATA_TYPE_FLOAT,
 * #CCS_DATA_TYPE_BOOL, or #CCS_DATA_TYPE_STRING; or if \p expression_ret is
 * NULL
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p node_left or \p node_right are
 * of type #CCS_DATA_TYPE_OBJECT but the object is not a valid CCS object
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was a lack of memory to
 * allocate the new expression
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_create_binary_expression(
	ccs_expression_type_t type,
	ccs_datum_t           node_left,
	ccs_datum_t           node_right,
	ccs_expression_t     *expression_ret);

/**
 * Create a new unary expression. Convenience wrapper around
 * #ccs_create_expression for unary expressions.
 * @param[in] type the type of the expression
 * @param[in] node child node
 * @param[out] expression_ret a pointer to the variable that will hold the newly
 *                            created expression
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p type is not a valid CCS
 * expression type; or if \p type arity is not 1; or if \p node is of type
 * #CCS_DATA_TYPE_OBJECT but is neither a #CCS_OBJECT_TYPE_PARAMETER nor a
 * #CCS_OBJECT_TYPE_EXPRESSION; or if \p node is not of type
 * #CCS_DATA_TYPE_OBJECT, #CCS_DATA_TYPE_NONE, #CCS_DATA_TYPE_INT,
 * #CCS_DATA_TYPE_FLOAT, #CCS_DATA_TYPE_BOOL, or #CCS_DATA_TYPE_STRING; or if
 * \p expression_ret is NULL
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p node is of type
 * #CCS_DATA_TYPE_OBJECT but the object is not a valid CCS object
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was a lack of memory to
 * allocate the new expression
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_create_unary_expression(
	ccs_expression_type_t type,
	ccs_datum_t           node,
	ccs_expression_t     *expression_ret);

/**
 * Create a new literal expression.
 * @param[in] value the value of the literal
 * @param[out] expression_ret a pointer to the variable that will hold the newly
 *             created expression. If value is of type #CCS_DATA_TYPE_STRING,
 *             the string value is memoized
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p value is not of type
 * #CCS_DATA_TYPE_NONE, #CCS_DATA_TYPE_INT, #CCS_DATA_TYPE_FLOAT,
 * #CCS_DATA_TYPE_BOOL, or #CCS_DATA_TYPE_STRING; or if \p expression_ret is
 * NULL
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was a lack of memory to
 * allocate the new expression
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_create_literal(ccs_datum_t value, ccs_expression_t *expression_ret);

/**
 * Create a new variable expression.
 * @param[in] parameter parameter to use as a variable
 * @param[out] expression_ret a pointer to the variable that will hold the newly
 *             created expression
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p expression_ret is NULL
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p parameter is not a valid CCS
 * parameter
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was a lack of memory to
 * allocate the new expression
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_create_variable(ccs_parameter_t parameter, ccs_expression_t *expression_ret);

/**
 * Get the type of an expression.
 * @param[in] expression
 * @param[out] type_ret a pointer to the variable that will contain the type of
 *                      the expression
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p type_ret is NULL
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p expression is not a valid CCS
 * expression
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_expression_get_type(
	ccs_expression_t       expression,
	ccs_expression_type_t *type_ret);

/**
 * Get the child nodes of an expression.
 * @param[in] expression
 * @param[in] num_nodes the size of the \p nodes array
 * @param[out] nodes an array of size \p num_nodes to hold the returned values
 *                   or NULL. If the array is too big, extra values are set NULL
 * @param[out] num_nodes_ret a pointer to a variable that will contain the
 *                           number of nodes that are or would be returned. Can
 *                           be NULL
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p expression is not a valid CCS
 * expression
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p nodes is NULL and \p num_nodes
 * is greater than 0; or if \p nodes is NULL and num_nodes_ret is NULL; or if
 * num_values is less than the number of values that would be returned
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_expression_get_nodes(
	ccs_expression_t  expression,
	size_t            num_nodes,
	ccs_expression_t *nodes,
	size_t           *num_nodes_ret);

/**
 * Get the value of a literal expression.
 * @param[in] expression
 * @param[out] value_ret a pointer to a variable that will contain the value of
 *                       the literal
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p expression is not a valid CCS
 * expression
 * @return #CCS_RESULT_ERROR_INVALID_EXPRESSION if \p expression is not a
 * #CCS_EXPRESSION_TYPE_LITERAL
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p value_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_literal_get_value(ccs_expression_t expression, ccs_datum_t *value_ret);

/**
 * Get the parameter of a variable expression.
 * @param[in] expression
 * @param[out] parameter_ret a pointer to a variable that will contain the
 *                           parameter
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p expression is not a valid CCS
 * expression
 * @return #CCS_RESULT_ERROR_INVALID_EXPRESSION if \p expression is not a
 * #CCS_EXPRESSION_TYPE_VARIABLE
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p parameter_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_variable_get_parameter(
	ccs_expression_t expression,
	ccs_parameter_t *parameter_ret);

/**
 * Get the value of an expression, in a given list of bindings.
 * @param[in] expression
 * @param[in] num_bindings the number of bindings in \p bindings
 * @param[in] bindings an array of \p num_bindings bindings
 * @param[out] result_ret a pointer to a variable that will contain the result
 *                        of the evaluation of the expression. Result can be
 *                        #ccs_inactive when the result depend on an inactive
 *                        parameter
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p expression is not a
 * valid CCS variable expression; of if one of the provided bindings is
 * not a valid CCS binding; or if no binding is provided and expression
 * must evaluate a variable
 * @return #CCS_RESULT_ERROR_INVALID_PARAMETER if a parameter was not
 * found in the provided bindings
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p result_ret is NULL; or
 * if \p num_bindings is greater than 0 and \p bindings is NULL; or if an
 * illegal arithmetic or comparison operation would have occurred; or if
 * a non boolean value is used in a boolean operation
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_expression_eval(
	ccs_expression_t expression,
	size_t           num_bindings,
	ccs_binding_t   *bindings,
	ccs_datum_t     *result_ret);

/**
 * Evaluate the entry of a list at a given index, in a given context, provided a
 * list of values for the context parameters.
 * @param[in] expression
 * @param[in] num_bindings the number of bindings in \p bindings
 * @param[in] bindings an array of \p num_bindings bindings
 * @param[in] index index of the child node to evaluate
 * @param[out] result_ret a pointer to a variable that will contain the result
 *                        of the evaluation of the expression. Result can be
 *                        #ccs_inactive when the result depend on an inactive
 *                        parameter
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p expression is not a
 * valid CCS variable expression; of if one of the provided bindings is
 * not a valid CCS binding; or if no binding is provided and expression
 * must evaluate a variable
 * @return #CCS_RESULT_ERROR_INVALID_EXPRESSION if \p expression is not a
 * #CCS_EXPRESSION_TYPE_LIST
 * @return #CCS_RESULT_ERROR_OUT_OF_BOUNDS if \p index is greater than the
 * number of child nodes in the list
 * @return #CCS_RESULT_ERROR_INVALID_PARAMETER if a parameter was not
 * found in the provided bindings
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p result_ret is NULL; or
 * if \p num_bindings is greater than 0 and \p bindings is NULL; or if an
 * illegal arithmetic or comparison operation would have occurred; or if
 * a non boolean value is used in a boolean operation
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_expression_list_eval_node(
	ccs_expression_t expression,
	size_t           num_bindings,
	ccs_binding_t   *bindings,
	size_t           index,
	ccs_datum_t     *result_ret);

/**
 * Get the parameters used in an expression.
 * @param[in] expression
 * @param[in] num_parameters the size of the \p parameters array
 * @param[in] parameters an array of size \p num_parameters to hold the
 *                       returned values, or NULL. If the array is too big,
 *                       extra values are set to NULL
 * @param[out] num_parameters_ret a pointer to a variable that will contain the
 *                                number of parameters that are or would be
 *                                returned. Can be NULL
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p expression is not a valid CCS
 * expression
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p parameters is NULL and \p
 * num_parameters is greater than 0; or if \p parameters is NULL and \p
 * num_parameters_ret is NULL; or if \p num_parameters is less than the number
 * of parameters that would be returned
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was not enough memory to
 * allocate temporary storage
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_expression_get_parameters(
	ccs_expression_t expression,
	size_t           num_parameters,
	ccs_parameter_t *parameters,
	size_t          *num_parameters_ret);

/**
 * Validate that an expression can be evaluated in the given context.
 * @param[in] expression
 * @param[in] num_contexts the number of contexts in \p contexts
 * @param[in] contexts an array of \p num_contexts contexts
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p expression is not a valid CCS
 * expression; or if one of the provided ocntexts in \p contexts is not a
 * valid CCS context
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if the expression depends on a
 * parameter and \p contexts is NULL
 * @return #CCS_RESULT_ERROR_INVALID_PARAMETER if one of the parameters
 * referenced by the expression cannot be found in at least one the contexts in
 * \p contexts
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was not enough memory to
 * allocate temporary storage
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_expression_check_contexts(
	ccs_expression_t expression,
	size_t           num_contexts,
	ccs_context_t   *contexts);
#ifdef __cplusplus
}
#endif

#endif //_CCS_CONDITION_H
