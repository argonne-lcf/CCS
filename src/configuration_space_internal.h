#ifndef _CONFIGURATION_SPACE_INTERNAL_H
#define _CONFIGURATION_SPACE_INTERNAL_H
#include "context_internal.h"

struct _ccs_distribution_wrapper_s;
typedef struct _ccs_distribution_wrapper_s _ccs_distribution_wrapper_t;

struct _ccs_configuration_space_data_s;
typedef struct _ccs_configuration_space_data_s _ccs_configuration_space_data_t;

struct _ccs_configuration_space_ops_s {
	_ccs_context_ops_t ops;
};
typedef struct _ccs_configuration_space_ops_s _ccs_configuration_space_ops_t;

struct _ccs_configuration_space_s {
	_ccs_object_internal_t           obj;
	_ccs_configuration_space_data_t *data;
};

struct _ccs_configuration_space_data_s {
	const char                  *name;
	size_t                       num_parameters;
	ccs_parameter_t             *parameters;
	_ccs_parameter_index_hash_t *hash_elems;
	_ccs_parameter_index_hash_t *name_hash;
	_ccs_parameter_index_hash_t *handle_hash;
	ccs_expression_t            *conditions;
	UT_array                   **parents;
	UT_array                   **children;
	size_t                      *sorted_indexes;
	size_t                       num_forbidden_clauses;
	ccs_expression_t            *forbidden_clauses;
	ccs_rng_t                    rng;
	ccs_feature_space_t          feature_space;
	ccs_distribution_space_t     default_distribution_space;
	size_t                       num_contexts;
	ccs_context_t                contexts[2];
};

#include "configuration_internal.h"

static inline ccs_result_t
_test_forbidden(
	ccs_configuration_space_t configuration_space,
	ccs_configuration_t       configuration,
	ccs_bool_t               *is_valid)
{
	ccs_expression_t *forbidden_clauses =
		configuration_space->data->forbidden_clauses;
	*is_valid = CCS_FALSE;
	for (size_t i = 0; i < configuration_space->data->num_forbidden_clauses;
	     i++) {
		ccs_datum_t result;
		CCS_VALIDATE(ccs_expression_eval(
			forbidden_clauses[i], configuration->data->num_bindings,
			configuration->data->bindings, &result));
		if (result.type == CCS_DATA_TYPE_INACTIVE)
			continue;
		if (result.type == CCS_DATA_TYPE_BOOL &&
		    result.value.i == CCS_TRUE)
			return CCS_RESULT_SUCCESS;
	}
	*is_valid = CCS_TRUE;
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_check_configuration(
	ccs_configuration_space_t configuration_space,
	ccs_configuration_t       configuration,
	ccs_bool_t               *is_valid_ret)
{
	size_t           *indexes = configuration_space->data->sorted_indexes;
	ccs_parameter_t  *parameters = configuration_space->data->parameters;
	ccs_expression_t *conditions = configuration_space->data->conditions;
	ccs_datum_t      *values     = configuration->data->values;

	for (size_t i = 0; i < configuration_space->data->num_parameters; i++) {
		ccs_bool_t active = CCS_TRUE;
		size_t     index  = indexes[i];
		if (conditions[index]) {
			ccs_datum_t result;
			CCS_VALIDATE(ccs_expression_eval(
				conditions[index],
				configuration->data->num_bindings,
				configuration->data->bindings, &result));
			if (!(result.type == CCS_DATA_TYPE_BOOL &&
			      result.value.i == CCS_TRUE))
				active = CCS_FALSE;
		}
		if (active != (values[index].type == CCS_DATA_TYPE_INACTIVE ?
				       CCS_FALSE :
				       CCS_TRUE)) {
			*is_valid_ret = CCS_FALSE;
			return CCS_RESULT_SUCCESS;
		}
		if (active) {
			CCS_VALIDATE(ccs_parameter_check_value(
				parameters[index], values[index],
				is_valid_ret));
			if (*is_valid_ret == CCS_FALSE)
				return CCS_RESULT_SUCCESS;
		}
	}
	CCS_VALIDATE(_test_forbidden(
		configuration_space, configuration, is_valid_ret));
	return CCS_RESULT_SUCCESS;
}

#endif //_CONFIGURATION_SPACE_INTERNAL_H
