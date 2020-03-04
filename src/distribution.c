#include "cconfigspace_internal.h"
#include "distribution_internal.h"

static inline _ccs_distribution_ops_t *
ccs_distribution_get_ops(ccs_distribution_t distribution) {
	return (_ccs_distribution_ops_t *)distribution->obj.ops;
}


ccs_error_t
ccs_distribution_get_type(ccs_distribution_t       distribution,
                          ccs_distribution_type_t *type_ret) {
	if (!distribution)
		return -CCS_INVALID_OBJECT;
	_ccs_distribution_ops_t *ops = ccs_distribution_get_ops(distribution);
	return ops->get_type(distribution->data, type_ret);
}

ccs_error_t
ccs_distribution_get_data_type(ccs_distribution_t       distribution,
                               ccs_data_type_t         *data_type_ret) {
	if (!distribution)
		return -CCS_INVALID_OBJECT;
	_ccs_distribution_ops_t *ops = ccs_distribution_get_ops(distribution);
	return ops->get_data_type(distribution->data, data_type_ret);
}

ccs_error_t
ccs_distribution_get_scale_type(ccs_distribution_t  distribution,
                                ccs_scale_type_t   *scale_type_ret) {
	if (!distribution)
		return -CCS_INVALID_OBJECT;
	_ccs_distribution_ops_t *ops = ccs_distribution_get_ops(distribution);
	return ops->get_scale_type(distribution->data, scale_type_ret);
}

ccs_error_t
ccs_distribution_get_num_parameters(ccs_distribution_t  distribution,
                                    size_t             *num_parameters_ret) {
	if (!distribution)
		return -CCS_INVALID_OBJECT;
	_ccs_distribution_ops_t *ops = ccs_distribution_get_ops(distribution);
	return ops->get_num_parameters(distribution->data, num_parameters_ret);
}

ccs_error_t
ccs_distribution_get_parameters(ccs_distribution_t  distribution,
                                size_t              num_parameters,
                                ccs_datum_t        *parameters,
                                size_t             *num_parameters_ret) {
	if (!distribution)
		return -CCS_INVALID_OBJECT;
	_ccs_distribution_ops_t *ops = ccs_distribution_get_ops(distribution);
	return ops->get_parameters(distribution->data, num_parameters, parameters, num_parameters_ret);
}

ccs_error_t
ccs_distribution_sample(ccs_distribution_t  distribution,
                        ccs_rng_t           rng,
                        ccs_datum_t        *value) {
	if (!distribution)
		return -CCS_INVALID_OBJECT;
	_ccs_distribution_ops_t *ops = ccs_distribution_get_ops(distribution);
	return ops->samples(distribution->data, rng, 1, value);
}

ccs_error_t
ccs_distribution_samples(ccs_distribution_t  distribution,
                         ccs_rng_t           rng,
                         size_t              num_values,
                         ccs_datum_t        *values) {
	if (!distribution)
		return -CCS_INVALID_OBJECT;
	_ccs_distribution_ops_t *ops = ccs_distribution_get_ops(distribution);
	return ops->samples(distribution->data, rng, num_values, values);
}
