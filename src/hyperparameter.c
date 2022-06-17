#include "cconfigspace_internal.h"
#include "hyperparameter_internal.h"

static inline _ccs_hyperparameter_ops_t *
ccs_hyperparameter_get_ops(ccs_hyperparameter_t hyperparameter) {
	return (_ccs_hyperparameter_ops_t *)hyperparameter->obj.ops;
}

ccs_result_t
ccs_hyperparameter_get_type(ccs_hyperparameter_t       hyperparameter,
                            ccs_hyperparameter_type_t *type_ret) {
	CCS_CHECK_OBJ(hyperparameter, CCS_HYPERPARAMETER);
	CCS_CHECK_PTR(type_ret);
	*type_ret = ((_ccs_hyperparameter_common_data_t *)(hyperparameter->data))->type;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_hyperparameter_get_default_value(ccs_hyperparameter_t  hyperparameter,
                                     ccs_datum_t          *value_ret) {
	CCS_CHECK_OBJ(hyperparameter, CCS_HYPERPARAMETER);
	CCS_CHECK_PTR(value_ret);
	*value_ret = ((_ccs_hyperparameter_common_data_t *)(hyperparameter->data))->default_value;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_hyperparameter_get_name(ccs_hyperparameter_t   hyperparameter,
                            const char           **name_ret) {
	CCS_CHECK_OBJ(hyperparameter, CCS_HYPERPARAMETER);
	CCS_CHECK_PTR(name_ret);
	*name_ret = ((_ccs_hyperparameter_common_data_t *)(hyperparameter->data))->name;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_hyperparameter_get_default_distribution(ccs_hyperparameter_t  hyperparameter,
                                            ccs_distribution_t   *distribution_ret) {
	CCS_CHECK_OBJ(hyperparameter, CCS_HYPERPARAMETER);
	CCS_CHECK_PTR(distribution_ret);
	_ccs_hyperparameter_ops_t *ops = ccs_hyperparameter_get_ops(hyperparameter);
	CCS_VALIDATE(ops->get_default_distribution( hyperparameter->data, distribution_ret));
	return CCS_SUCCESS;
}

ccs_result_t
ccs_hyperparameter_check_value(ccs_hyperparameter_t  hyperparameter,
                               ccs_datum_t           value,
                               ccs_bool_t           *result_ret) {
	CCS_CHECK_OBJ(hyperparameter, CCS_HYPERPARAMETER);
	CCS_CHECK_PTR(result_ret);
	_ccs_hyperparameter_ops_t *ops = ccs_hyperparameter_get_ops(hyperparameter);
	CCS_VALIDATE(ops->check_values(hyperparameter->data, 1, &value, NULL, result_ret));
	return CCS_SUCCESS;
}

ccs_result_t
ccs_hyperparameter_check_values(ccs_hyperparameter_t  hyperparameter,
                                size_t                num_values,
                                const ccs_datum_t    *values,
                                ccs_bool_t           *results) {
	CCS_CHECK_OBJ(hyperparameter, CCS_HYPERPARAMETER);
	CCS_CHECK_ARY(num_values, values);
	CCS_CHECK_ARY(num_values, results);
	_ccs_hyperparameter_ops_t *ops = ccs_hyperparameter_get_ops(hyperparameter);
	CCS_VALIDATE(ops->check_values(hyperparameter->data, num_values, values, NULL, results));
	return CCS_SUCCESS;
}

ccs_result_t
ccs_hyperparameter_validate_value(ccs_hyperparameter_t  hyperparameter,
                                  ccs_datum_t           value,
                                  ccs_datum_t          *value_ret,
                                  ccs_bool_t           *result_ret) {
	CCS_CHECK_OBJ(hyperparameter, CCS_HYPERPARAMETER);
	CCS_CHECK_PTR(value_ret);
	CCS_CHECK_PTR(result_ret);
	_ccs_hyperparameter_ops_t *ops = ccs_hyperparameter_get_ops(hyperparameter);
	CCS_VALIDATE(ops->check_values(hyperparameter->data, 1, &value, value_ret, result_ret));
	return CCS_SUCCESS;
}

ccs_result_t
ccs_hyperparameter_validate_values(ccs_hyperparameter_t  hyperparameter,
                                   size_t                num_values,
                                   const ccs_datum_t    *values,
                                   ccs_datum_t          *values_ret,
                                   ccs_bool_t           *results) {
	CCS_CHECK_OBJ(hyperparameter, CCS_HYPERPARAMETER);
	CCS_CHECK_ARY(num_values, values);
	CCS_CHECK_ARY(num_values, values_ret);
	CCS_CHECK_ARY(num_values, results);
	_ccs_hyperparameter_ops_t *ops = ccs_hyperparameter_get_ops(hyperparameter);
	CCS_VALIDATE(ops->check_values(hyperparameter->data, num_values, values, values_ret, results));
	return CCS_SUCCESS;
}


ccs_result_t
ccs_hyperparameter_sample(ccs_hyperparameter_t  hyperparameter,
                          ccs_distribution_t    distribution,
                          ccs_rng_t             rng,
                          ccs_datum_t          *value_ret) {
	CCS_CHECK_OBJ(hyperparameter, CCS_HYPERPARAMETER);
	CCS_CHECK_OBJ(distribution, CCS_DISTRIBUTION);
	CCS_CHECK_OBJ(rng, CCS_RNG);
	CCS_CHECK_PTR(value_ret);
	_ccs_hyperparameter_ops_t *ops = ccs_hyperparameter_get_ops(hyperparameter);
	CCS_VALIDATE(ops->samples(hyperparameter->data, distribution, rng, 1, value_ret));
	return CCS_SUCCESS;
}

ccs_result_t
ccs_hyperparameter_samples(ccs_hyperparameter_t  hyperparameter,
                           ccs_distribution_t    distribution,
                           ccs_rng_t             rng,
                           size_t                num_values,
                           ccs_datum_t          *values) {
	CCS_CHECK_OBJ(hyperparameter, CCS_HYPERPARAMETER);
	CCS_CHECK_OBJ(distribution, CCS_DISTRIBUTION);
	CCS_CHECK_OBJ(rng, CCS_RNG);
	CCS_CHECK_ARY(num_values, values);
	if (!num_values)
		return CCS_SUCCESS;
	_ccs_hyperparameter_ops_t *ops = ccs_hyperparameter_get_ops(hyperparameter);
	CCS_VALIDATE(ops->samples(hyperparameter->data, distribution, rng, num_values, values));
	return CCS_SUCCESS;
}

ccs_result_t
ccs_hyperparameter_convert_samples(ccs_hyperparameter_t  hyperparameter,
                                   ccs_bool_t            oversampling,
                                   size_t                num_values,
                                   const ccs_numeric_t  *values,
                                   ccs_datum_t          *results) {
	CCS_CHECK_OBJ(hyperparameter, CCS_HYPERPARAMETER);
	CCS_CHECK_ARY(num_values, values);
	CCS_CHECK_ARY(num_values, results);
	if (!num_values)
		return CCS_SUCCESS;
	_ccs_hyperparameter_ops_t *ops = ccs_hyperparameter_get_ops(hyperparameter);
	CCS_VALIDATE(ops->convert_samples(hyperparameter->data, oversampling, num_values, values, results));
	return CCS_SUCCESS;
}

ccs_result_t
ccs_hyperparameter_sampling_interval(ccs_hyperparameter_t  hyperparameter,
                                     ccs_interval_t       *interval_ret) {
	CCS_CHECK_OBJ(hyperparameter, CCS_HYPERPARAMETER);
	CCS_CHECK_PTR(interval_ret);
	*interval_ret = ((_ccs_hyperparameter_common_data_t *)(hyperparameter->data))->interval;
	return CCS_SUCCESS;
}
