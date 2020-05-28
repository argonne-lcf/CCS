#include "cconfigspace_internal.h"
#include "hyperparameter_internal.h"

static inline _ccs_hyperparameter_ops_t *
ccs_hyperparameter_get_ops(ccs_hyperparameter_t hyperparameter) {
	return (_ccs_hyperparameter_ops_t *)hyperparameter->obj.ops;
}

ccs_error_t
ccs_hyperparameter_get_type(ccs_hyperparameter_t       hyperparameter,
                            ccs_hyperparameter_type_t *type_ret) {
	if (!hyperparameter || !hyperparameter->data)
		return -CCS_INVALID_OBJECT;
	if (!type_ret)
		return -CCS_INVALID_VALUE;
	*type_ret = ((_ccs_hyperparameter_common_data_t *)(hyperparameter->data))->type;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_hyperparameter_get_default_value(ccs_hyperparameter_t  hyperparameter,
                                     ccs_datum_t          *value_ret) {
	if (!hyperparameter || !hyperparameter->data)
		return -CCS_INVALID_OBJECT;
	if (!value_ret)
		return -CCS_INVALID_VALUE;
	*value_ret = ((_ccs_hyperparameter_common_data_t *)(hyperparameter->data))->default_value;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_hyperparameter_get_name(ccs_hyperparameter_t   hyperparameter,
                            const char           **name_ret) {
	if (!hyperparameter || !hyperparameter->data)
		return -CCS_INVALID_OBJECT;
	if (!name_ret)
		return -CCS_INVALID_OBJECT;
	*name_ret = ((_ccs_hyperparameter_common_data_t *)(hyperparameter->data))->name;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_hyperparameter_get_user_data(ccs_hyperparameter_t   hyperparameter,
                                 void                 **user_data_ret) {
	if (!hyperparameter || !hyperparameter->data)
		return -CCS_INVALID_OBJECT;
	if (!user_data_ret)
		return -CCS_INVALID_OBJECT;
	*user_data_ret = ((_ccs_hyperparameter_common_data_t *)(hyperparameter->data))->user_data;
	return CCS_SUCCESS;
}


ccs_error_t
ccs_hyperparameter_get_default_distribution(ccs_hyperparameter_t  hyperparameter,
                                            ccs_distribution_t   *distribution) {
	if (!hyperparameter || !hyperparameter->data)
		return -CCS_INVALID_OBJECT;
	if (!distribution)
		return -CCS_INVALID_VALUE;
	_ccs_hyperparameter_ops_t *ops = ccs_hyperparameter_get_ops(hyperparameter);
	return ops->get_default_distribution( hyperparameter->data, distribution);
}

ccs_error_t
ccs_hyperparameter_check_value(ccs_hyperparameter_t  hyperparameter,
                               ccs_datum_t           value,
                               ccs_bool_t           *result_ret) {
	if (!hyperparameter || !hyperparameter->data)
		return -CCS_INVALID_OBJECT;
	if (!result_ret)
		return -CCS_INVALID_VALUE;
	_ccs_hyperparameter_ops_t *ops = ccs_hyperparameter_get_ops(hyperparameter);
	return ops->check_values(hyperparameter->data, 1, &value, result_ret);
}

ccs_error_t
ccs_hyperparameter_check_values(ccs_hyperparameter_t  hyperparameter,
                                size_t                num_values,
                                const ccs_datum_t    *values,
                                ccs_bool_t           *results) {
	if (!hyperparameter || !hyperparameter->data)
		return -CCS_INVALID_OBJECT;
	if (num_values && (!values || !results ))
		return -CCS_INVALID_VALUE;
	_ccs_hyperparameter_ops_t *ops = ccs_hyperparameter_get_ops(hyperparameter);
	return ops->check_values(hyperparameter->data, num_values, values, results);
}

ccs_error_t
ccs_hyperparameter_sample(ccs_hyperparameter_t  hyperparameter,
                          ccs_distribution_t    distribution,
                          ccs_rng_t             rng,
                          ccs_datum_t          *value) {
	if (!hyperparameter || !distribution || !hyperparameter->data)
		return -CCS_INVALID_OBJECT;
	if (!value)
		return -CCS_INVALID_VALUE;
	_ccs_hyperparameter_ops_t *ops = ccs_hyperparameter_get_ops(hyperparameter);
	return ops->samples(hyperparameter->data, distribution, rng, 1, value);
}

ccs_error_t
ccs_hyperparameter_samples(ccs_hyperparameter_t  hyperparameter,
                           ccs_distribution_t    distribution,
                           ccs_rng_t             rng,
                           size_t                num_values,
                           ccs_datum_t          *values) {
	if (!hyperparameter || !distribution || !hyperparameter->data)
		return -CCS_INVALID_OBJECT;
	if (!num_values || !values)
		return -CCS_INVALID_VALUE;
	_ccs_hyperparameter_ops_t *ops = ccs_hyperparameter_get_ops(hyperparameter);
	return ops->samples(hyperparameter->data, distribution, rng, num_values, values);
}

