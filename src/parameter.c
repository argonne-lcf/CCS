#include "cconfigspace_internal.h"
#include "parameter_internal.h"

static inline _ccs_parameter_ops_t *
ccs_parameter_get_ops(ccs_parameter_t parameter) {
	return (_ccs_parameter_ops_t *)parameter->obj.ops;
}

ccs_error_t
ccs_parameter_get_type(ccs_parameter_t       parameter,
                            ccs_parameter_type_t *type_ret) {
	CCS_CHECK_OBJ(parameter, CCS_PARAMETER);
	CCS_CHECK_PTR(type_ret);
	*type_ret = ((_ccs_parameter_common_data_t *)(parameter->data))->type;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_parameter_get_default_value(ccs_parameter_t  parameter,
                                     ccs_datum_t          *value_ret) {
	CCS_CHECK_OBJ(parameter, CCS_PARAMETER);
	CCS_CHECK_PTR(value_ret);
	*value_ret = ((_ccs_parameter_common_data_t *)(parameter->data))->default_value;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_parameter_get_name(ccs_parameter_t   parameter,
                            const char           **name_ret) {
	CCS_CHECK_OBJ(parameter, CCS_PARAMETER);
	CCS_CHECK_PTR(name_ret);
	*name_ret = ((_ccs_parameter_common_data_t *)(parameter->data))->name;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_parameter_get_default_distribution(ccs_parameter_t  parameter,
                                            ccs_distribution_t   *distribution_ret) {
	CCS_CHECK_OBJ(parameter, CCS_PARAMETER);
	CCS_CHECK_PTR(distribution_ret);
	_ccs_parameter_ops_t *ops = ccs_parameter_get_ops(parameter);
	CCS_VALIDATE(ops->get_default_distribution( parameter->data, distribution_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_parameter_check_value(ccs_parameter_t  parameter,
                               ccs_datum_t           value,
                               ccs_bool_t           *result_ret) {
	CCS_CHECK_OBJ(parameter, CCS_PARAMETER);
	CCS_CHECK_PTR(result_ret);
	_ccs_parameter_ops_t *ops = ccs_parameter_get_ops(parameter);
	CCS_VALIDATE(ops->check_values(parameter->data, 1, &value, NULL, result_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_parameter_check_values(ccs_parameter_t  parameter,
                                size_t                num_values,
                                const ccs_datum_t    *values,
                                ccs_bool_t           *results) {
	CCS_CHECK_OBJ(parameter, CCS_PARAMETER);
	CCS_CHECK_ARY(num_values, values);
	CCS_CHECK_ARY(num_values, results);
	_ccs_parameter_ops_t *ops = ccs_parameter_get_ops(parameter);
	CCS_VALIDATE(ops->check_values(parameter->data, num_values, values, NULL, results));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_parameter_validate_value(ccs_parameter_t  parameter,
                                  ccs_datum_t           value,
                                  ccs_datum_t          *value_ret,
                                  ccs_bool_t           *result_ret) {
	CCS_CHECK_OBJ(parameter, CCS_PARAMETER);
	CCS_CHECK_PTR(value_ret);
	CCS_CHECK_PTR(result_ret);
	_ccs_parameter_ops_t *ops = ccs_parameter_get_ops(parameter);
	CCS_VALIDATE(ops->check_values(parameter->data, 1, &value, value_ret, result_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_parameter_validate_values(ccs_parameter_t  parameter,
                                   size_t                num_values,
                                   const ccs_datum_t    *values,
                                   ccs_datum_t          *values_ret,
                                   ccs_bool_t           *results) {
	CCS_CHECK_OBJ(parameter, CCS_PARAMETER);
	CCS_CHECK_ARY(num_values, values);
	CCS_CHECK_ARY(num_values, values_ret);
	CCS_CHECK_ARY(num_values, results);
	_ccs_parameter_ops_t *ops = ccs_parameter_get_ops(parameter);
	CCS_VALIDATE(ops->check_values(parameter->data, num_values, values, values_ret, results));
	return CCS_SUCCESS;
}


ccs_error_t
ccs_parameter_sample(ccs_parameter_t  parameter,
                          ccs_distribution_t    distribution,
                          ccs_rng_t             rng,
                          ccs_datum_t          *value_ret) {
	CCS_CHECK_OBJ(parameter, CCS_PARAMETER);
	CCS_CHECK_OBJ(distribution, CCS_DISTRIBUTION);
	CCS_CHECK_OBJ(rng, CCS_RNG);
	CCS_CHECK_PTR(value_ret);
	_ccs_parameter_ops_t *ops = ccs_parameter_get_ops(parameter);
	CCS_VALIDATE(ops->samples(parameter->data, distribution, rng, 1, value_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_parameter_samples(ccs_parameter_t  parameter,
                           ccs_distribution_t    distribution,
                           ccs_rng_t             rng,
                           size_t                num_values,
                           ccs_datum_t          *values) {
	CCS_CHECK_OBJ(parameter, CCS_PARAMETER);
	CCS_CHECK_OBJ(distribution, CCS_DISTRIBUTION);
	CCS_CHECK_OBJ(rng, CCS_RNG);
	CCS_CHECK_ARY(num_values, values);
	if (!num_values)
		return CCS_SUCCESS;
	_ccs_parameter_ops_t *ops = ccs_parameter_get_ops(parameter);
	CCS_VALIDATE(ops->samples(parameter->data, distribution, rng, num_values, values));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_parameter_convert_samples(ccs_parameter_t  parameter,
                                   ccs_bool_t            oversampling,
                                   size_t                num_values,
                                   const ccs_numeric_t  *values,
                                   ccs_datum_t          *results) {
	CCS_CHECK_OBJ(parameter, CCS_PARAMETER);
	CCS_CHECK_ARY(num_values, values);
	CCS_CHECK_ARY(num_values, results);
	if (!num_values)
		return CCS_SUCCESS;
	_ccs_parameter_ops_t *ops = ccs_parameter_get_ops(parameter);
	CCS_VALIDATE(ops->convert_samples(parameter->data, oversampling, num_values, values, results));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_parameter_sampling_interval(ccs_parameter_t  parameter,
                                     ccs_interval_t       *interval_ret) {
	CCS_CHECK_OBJ(parameter, CCS_PARAMETER);
	CCS_CHECK_PTR(interval_ret);
	*interval_ret = ((_ccs_parameter_common_data_t *)(parameter->data))->interval;
	return CCS_SUCCESS;
}
