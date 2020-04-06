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
                               ccs_numeric_type_t      *data_type_ret) {
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
                                  ccs_numeric_t      *quantization_ret) {
	if (!distribution || !distribution->data)
		return -CCS_INVALID_OBJECT;
	if (!quantization_ret)
		return -CCS_INVALID_VALUE;
	*quantization_ret = ((_ccs_distribution_common_data_t *)(distribution->data))->quantization;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_distribution_get_bounds(ccs_distribution_t  distribution,
                            ccs_interval_t     *interval_ret) {
	if (!distribution || !distribution->data)
		return -CCS_INVALID_OBJECT;
	if (!interval_ret)
		return -CCS_INVALID_VALUE;
	_ccs_distribution_ops_t *ops = ccs_distribution_get_ops(distribution);
	return ops->get_bounds(distribution->data, interval_ret);
}

ccs_error_t
ccs_distribution_check_oversampling(ccs_distribution_t  distribution,
                                    ccs_interval_t     *interval,
                                    ccs_bool_t         *oversampling_ret) {
	ccs_error_t err;
	ccs_interval_t d_interval;
	if (!interval || !oversampling_ret)
		return -CCS_INVALID_VALUE;

	err = ccs_distribution_get_bounds(distribution, &d_interval);
	if (err)
		return err;

	ccs_interval_t intersection;
	err = ccs_interval_intersect(&d_interval, interval, &intersection);
	if (err)
		return err;

	ccs_bool_t eql;
	err = ccs_interval_equal(&d_interval, &intersection, &eql);
	if (err)
		return err;
	*oversampling_ret = (eql ? CCS_FALSE : CCS_TRUE);

	return CCS_SUCCESS;
}

ccs_error_t
ccs_distribution_sample(ccs_distribution_t  distribution,
                        ccs_rng_t           rng,
                        ccs_numeric_t      *value) {
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
                         ccs_numeric_t      *values) {
	if (!distribution || !distribution->data)
		return -CCS_INVALID_OBJECT;
	if (!num_values || !values)
		return -CCS_INVALID_VALUE;
	_ccs_distribution_ops_t *ops = ccs_distribution_get_ops(distribution);
	return ops->samples(distribution->data, rng, num_values, values);
}

ccs_error_t
ccs_create_normal_float_distribution(ccs_float_t         mu,
                                     ccs_float_t         sigma,
                                     ccs_scale_type_t    scale,
                                     ccs_float_t         quantization,
                                     ccs_distribution_t *distribution_ret) {
	return ccs_create_normal_distribution(CCS_NUM_FLOAT, mu, sigma, scale,
                                       CCSF(quantization), distribution_ret);
}

ccs_error_t
ccs_create_normal_int_distribution(ccs_float_t         mu,
                                   ccs_float_t         sigma,
                                   ccs_scale_type_t    scale,
                                   ccs_int_t           quantization,
                                   ccs_distribution_t *distribution_ret) {
	return ccs_create_normal_distribution(CCS_NUM_INTEGER, mu, sigma, scale,
	                                      CCSI(quantization), distribution_ret);
}

ccs_error_t
ccs_create_uniform_float_distribution(ccs_float_t         lower,
                                      ccs_float_t         upper,
                                      ccs_scale_type_t    scale,
                                      ccs_float_t         quantization,
                                      ccs_distribution_t *distribution_ret) {
	return ccs_create_uniform_distribution(CCS_NUM_FLOAT, CCSF(lower), CCSF(upper),
	                                       scale, CCSF(quantization),
	                                       distribution_ret);
}

ccs_error_t
ccs_create_uniform_int_distribution(ccs_int_t           lower,
                                    ccs_int_t           upper,
                                    ccs_scale_type_t    scale,
                                    ccs_int_t           quantization,
                                    ccs_distribution_t *distribution_ret) {
	return ccs_create_uniform_distribution(CCS_NUM_INTEGER, CCSI(lower), CCSI(upper),
	                                       scale, CCSI(quantization),
	                                       distribution_ret);
}

