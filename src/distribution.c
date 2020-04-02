#include "cconfigspace_internal.h"
#include "distribution_internal.h"

static inline _ccs_distribution_ops_t *
ccs_distribution_get_ops(ccs_distribution_t distribution) {
	return (_ccs_distribution_ops_t *)distribution->obj.ops;
}


ccs_error_t
ccs_distribution_get_type(ccs_distribution_t       distribution,
                          ccs_distribution_type_t *type_ret) {
	if (!distribution || !distribution->data)
		return -CCS_INVALID_OBJECT;
	if (!type_ret)
		return -CCS_INVALID_VALUE;
	*type_ret = ((_ccs_distribution_common_data_t *)(distribution->data))->type;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_distribution_get_data_type(ccs_distribution_t       distribution,
                               ccs_data_type_t         *data_type_ret) {
	if (!distribution || !distribution->data)
		return -CCS_INVALID_OBJECT;
	if (!data_type_ret)
		return -CCS_INVALID_VALUE;
	*data_type_ret = ((_ccs_distribution_common_data_t *)(distribution->data))->data_type;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_distribution_get_scale_type(ccs_distribution_t  distribution,
                                ccs_scale_type_t   *scale_type_ret) {
	if (!distribution || !distribution->data)
		return -CCS_INVALID_OBJECT;
	if (!scale_type_ret)
		return -CCS_INVALID_VALUE;
	*scale_type_ret = ((_ccs_distribution_common_data_t *)(distribution->data))->scale_type;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_distribution_get_quantization(ccs_distribution_t  distribution,
                                  ccs_datum_t        *quantization_ret) {
	if (!distribution || !distribution->data)
		return -CCS_INVALID_OBJECT;
	if (!quantization_ret)
		return -CCS_INVALID_VALUE;
	quantization_ret->value = ((_ccs_distribution_common_data_t *)(distribution->data))->quantization;
	quantization_ret->type = ((_ccs_distribution_common_data_t *)(distribution->data))->data_type;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_distribution_get_num_parameters(ccs_distribution_t  distribution,
                                    size_t             *num_parameters_ret) {
	if (!distribution || !distribution->data)
		return -CCS_INVALID_OBJECT;
	if (!num_parameters_ret)
		return -CCS_INVALID_VALUE;
	_ccs_distribution_ops_t *ops = ccs_distribution_get_ops(distribution);
	return ops->get_num_parameters(distribution->data, num_parameters_ret);
}

ccs_error_t
ccs_distribution_get_parameters(ccs_distribution_t  distribution,
                                size_t              num_parameters,
                                ccs_datum_t        *parameters,
                                size_t             *num_parameters_ret) {
	if (!distribution || !distribution->data)
		return -CCS_INVALID_OBJECT;
	if (num_parameters > 0 && !parameters)
		return -CCS_INVALID_VALUE;
	_ccs_distribution_ops_t *ops = ccs_distribution_get_ops(distribution);
	return ops->get_parameters(distribution->data, num_parameters, parameters, num_parameters_ret);
}

ccs_error_t
ccs_distribution_get_bounds(ccs_distribution_t  distribution,
                            ccs_datum_t        *lower,
                            ccs_bool_t         *lower_included,
                            ccs_datum_t        *upper,
                            ccs_bool_t         *upper_included) {
	if (!distribution || !distribution->data)
		return -CCS_INVALID_OBJECT;
	if (!lower && !lower_included && !upper && !upper_included)
		return -CCS_INVALID_VALUE;
	_ccs_distribution_ops_t *ops = ccs_distribution_get_ops(distribution);
	return ops->get_bounds(distribution->data, lower, lower_included, upper, upper_included);
}

ccs_error_t
ccs_distribution_sample(ccs_distribution_t  distribution,
                        ccs_rng_t           rng,
                        ccs_value_t        *value) {
	if (!distribution || !distribution->data)
		return -CCS_INVALID_OBJECT;
	if (!value)
		return -CCS_INVALID_VALUE;
	_ccs_distribution_ops_t *ops = ccs_distribution_get_ops(distribution);
	return ops->samples(distribution->data, rng, 1, value);
}

ccs_error_t
ccs_distribution_samples(ccs_distribution_t  distribution,
                         ccs_rng_t           rng,
                         size_t              num_values,
                         ccs_value_t        *values) {
	if (!distribution || !distribution->data)
		return -CCS_INVALID_OBJECT;
	if (!num_values || !values)
		return -CCS_INVALID_VALUE;
	_ccs_distribution_ops_t *ops = ccs_distribution_get_ops(distribution);
	return ops->samples(distribution->data, rng, num_values, values);
}
