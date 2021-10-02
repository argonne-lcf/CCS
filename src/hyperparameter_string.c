#include "cconfigspace_internal.h"
#include "hyperparameter_internal.h"
#include <string.h>

struct _ccs_hyperparameter_string_data_s {
	_ccs_hyperparameter_common_data_t common_data;
};
typedef struct _ccs_hyperparameter_string_data_s _ccs_hyperparameter_string_data_t;

static ccs_result_t
_ccs_hyperparameter_string_del(ccs_object_t o) {
	(void)o;
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_hyperparameter_string_check_values(_ccs_hyperparameter_data_t *data,
                                        size_t                num_values,
                                        const ccs_datum_t    *values,
                                        ccs_datum_t          *values_ret,
                                        ccs_bool_t           *results) {
	(void)data;
	for(size_t i = 0; i < num_values; i++)
		if (values[i].type != CCS_STRING)
			results[i] = CCS_FALSE;
		else
			results[i] = CCS_TRUE;
	if (values_ret) {
		for (size_t i = 0; i < num_values; i++)
			if (results[i] == CCS_TRUE) {
				values_ret[i] = values[i];
			} else {
				values_ret[i] = ccs_inactive;
			}
	}
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_hyperparameter_string_samples(_ccs_hyperparameter_data_t *data,
                                   ccs_distribution_t          distribution,
                                   ccs_rng_t                   rng,
                                   size_t                      num_values,
                                   ccs_datum_t                *values) {
	(void)data;
	(void)distribution;
	(void)rng;
	(void)num_values;
	(void)values;
	return -CCS_UNSUPPORTED_OPERATION;
}

static ccs_result_t
_ccs_hyperparameter_string_get_default_distribution(
		_ccs_hyperparameter_data_t *data,
		ccs_distribution_t         *distribution) {
	(void)data;
	(void)distribution;
	return -CCS_UNSUPPORTED_OPERATION;
}

static ccs_result_t
_ccs_hyperparameter_string_convert_samples(
		_ccs_hyperparameter_data_t *data,
		ccs_bool_t                  oversampling,
		size_t                      num_values,
		const ccs_numeric_t        *values,
		ccs_datum_t                *results) {
	(void)data;
	(void)oversampling;
	(void)num_values;
	(void)values;
	(void)results;
	return -CCS_UNSUPPORTED_OPERATION;
}

static _ccs_hyperparameter_ops_t _ccs_hyperparameter_string_ops = {
	{ &_ccs_hyperparameter_string_del },
	&_ccs_hyperparameter_string_check_values,
	&_ccs_hyperparameter_string_samples,
	&_ccs_hyperparameter_string_get_default_distribution,
	&_ccs_hyperparameter_string_convert_samples
};

extern ccs_result_t
ccs_create_string_hyperparameter(const char           *name,
                                 void                 *user_data,
                                 ccs_hyperparameter_t *hyperparameter_ret) {
	CCS_CHECK_PTR(name);
	CCS_CHECK_PTR(hyperparameter_ret);
	uintptr_t mem = (uintptr_t)calloc(1, sizeof(struct _ccs_hyperparameter_s) + sizeof(_ccs_hyperparameter_string_data_t) + strlen(name) + 1);
	if (!mem)
		return -CCS_OUT_OF_MEMORY;

	ccs_hyperparameter_t hyperparam = (ccs_hyperparameter_t)mem;
	_ccs_object_init(&(hyperparam->obj), CCS_HYPERPARAMETER, (_ccs_object_ops_t *)&_ccs_hyperparameter_string_ops);
	_ccs_hyperparameter_string_data_t *hyperparam_data = (_ccs_hyperparameter_string_data_t *)(mem + sizeof(struct _ccs_hyperparameter_s));
	hyperparam_data->common_data.type = CCS_HYPERPARAMETER_TYPE_STRING;
	hyperparam_data->common_data.name = (char *)(mem + sizeof(struct _ccs_hyperparameter_s) + sizeof(_ccs_hyperparameter_string_data_t));
	strcpy((char *)hyperparam_data->common_data.name, name);
	hyperparam_data->common_data.user_data = user_data;
	hyperparam->data = (_ccs_hyperparameter_data_t *)hyperparam_data;
	*hyperparameter_ret = hyperparam;

	return CCS_SUCCESS;
}
