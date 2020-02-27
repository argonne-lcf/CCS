enum ccs_expression_type_e {
	CCS_EQUALS,
	CCS_LESS_THAN,
	CCS_GREATER_THAN,
	CCS_IN,
	CCS_LEAF,
	CCS_NEGATION,
	CCS_AND,
	CCS_OR,
	CCS_CONDITION_TYPE_MAX,
	CCS_CONDITION_FORCE_32BIT = MAX_INT
};

typedef ccs_expression_type_e ccs_expression_type_t;

extern ccs_error_t
ccs_create_equals_expression(ccs_hyperparameter_t  hyperparameter,
                             ccs_datum_t           value
                             ccs_expression_t     *expression);
