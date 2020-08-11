#include "cconfigspace_internal.h"
#include "distribution_internal.h"

static inline _ccs_distribution_ops_t *
ccs_distribution_get_ops(ccs_distribution_t distribution) {
	return (_ccs_distribution_ops_t *)distribution->obj.ops;
}


ccs_result_t
ccs_distribution_get_type(ccs_distribution_t       distribution,
                          ccs_distribution_type_t *type_ret) {
	CCS_CHECK_OBJ(distribution, CCS_DISTRIBUTION);
	CCS_CHECK_PTR(type_ret);
	*type_ret = ((_ccs_distribution_common_data_t *)(distribution->data))->type;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_distribution_get_data_types(ccs_distribution_t       distribution,
                                ccs_numeric_type_t      *data_types_ret) {
	CCS_CHECK_OBJ(distribution, CCS_DISTRIBUTION);
	CCS_CHECK_PTR(data_types_ret);
        _ccs_distribution_common_data_t *d =
		(_ccs_distribution_common_data_t *)(distribution->data);
	for(size_t i = 0; i < d->dimension; i++)
		data_types_ret[i] = d->data_types[i];
	return CCS_SUCCESS;
}

ccs_result_t
ccs_distribution_get_dimension(ccs_distribution_t  distribution,
                               size_t             *dimension_ret) {
	CCS_CHECK_OBJ(distribution, CCS_DISTRIBUTION);
	CCS_CHECK_PTR(dimension_ret);
	*dimension_ret = ((_ccs_distribution_common_data_t *)(distribution->data))->dimension;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_distribution_get_bounds(ccs_distribution_t  distribution,
                            ccs_interval_t     *interval_ret) {
	CCS_CHECK_OBJ(distribution, CCS_DISTRIBUTION);
	CCS_CHECK_PTR(interval_ret);
	_ccs_distribution_ops_t *ops = ccs_distribution_get_ops(distribution);
	return ops->get_bounds(distribution->data, interval_ret);
}

ccs_result_t
ccs_distribution_check_oversampling(ccs_distribution_t  distribution,
                                    ccs_interval_t     *interval,
                                    ccs_bool_t         *oversampling_ret) {
	CCS_CHECK_OBJ(distribution, CCS_DISTRIBUTION);
	CCS_CHECK_PTR(interval);
	CCS_CHECK_PTR(oversampling_ret);
	ccs_result_t err;
	ccs_interval_t d_interval;

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

ccs_result_t
ccs_distribution_sample(ccs_distribution_t  distribution,
                        ccs_rng_t           rng,
                        ccs_numeric_t      *value_ret) {
	CCS_CHECK_OBJ(distribution, CCS_DISTRIBUTION);
	CCS_CHECK_PTR(value_ret);
	_ccs_distribution_ops_t *ops = ccs_distribution_get_ops(distribution);
	return ops->samples(distribution->data, rng, 1, value_ret);
}

ccs_result_t
ccs_distribution_samples(ccs_distribution_t  distribution,
                         ccs_rng_t           rng,
                         size_t              num_values,
                         ccs_numeric_t      *values) {
	CCS_CHECK_OBJ(distribution, CCS_DISTRIBUTION);
	if (!num_values)
		return CCS_SUCCESS;
	CCS_CHECK_ARY(num_values, values);
	_ccs_distribution_ops_t *ops = ccs_distribution_get_ops(distribution);
	return ops->samples(distribution->data, rng, num_values, values);
}

ccs_result_t
ccs_distribution_strided_samples(ccs_distribution_t  distribution,
                                 ccs_rng_t           rng,
                                 size_t              num_values,
                                 size_t              stride,
                                 ccs_numeric_t      *values) {
	CCS_CHECK_OBJ(distribution, CCS_DISTRIBUTION);
	if (stride < ((_ccs_distribution_common_data_t *)(distribution->data))->dimension)
		return -CCS_INVALID_VALUE;
	if (!num_values)
		return CCS_SUCCESS;
	CCS_CHECK_ARY(num_values, values);
	_ccs_distribution_ops_t *ops = ccs_distribution_get_ops(distribution);
	return ops->strided_samples(distribution->data, rng, num_values, stride, values);
}

ccs_result_t
ccs_distribution_soa_samples(ccs_distribution_t   distribution,
                             ccs_rng_t            rng,
                             size_t               num_values,
                             ccs_numeric_t      **values) {
	CCS_CHECK_OBJ(distribution, CCS_DISTRIBUTION);
	if (!num_values)
		return CCS_SUCCESS;
	CCS_CHECK_ARY(num_values, values);
	_ccs_distribution_ops_t *ops = ccs_distribution_get_ops(distribution);
	return ops->soa_samples(distribution->data, rng, num_values, values);
}

ccs_result_t
ccs_create_normal_float_distribution(ccs_float_t         mu,
                                     ccs_float_t         sigma,
                                     ccs_scale_type_t    scale,
                                     ccs_float_t         quantization,
                                     ccs_distribution_t *distribution_ret) {
	return ccs_create_normal_distribution(CCS_NUM_FLOAT, mu, sigma, scale,
                                       CCSF(quantization), distribution_ret);
}

ccs_result_t
ccs_create_normal_int_distribution(ccs_float_t         mu,
                                   ccs_float_t         sigma,
                                   ccs_scale_type_t    scale,
                                   ccs_int_t           quantization,
                                   ccs_distribution_t *distribution_ret) {
	return ccs_create_normal_distribution(CCS_NUM_INTEGER, mu, sigma, scale,
	                                      CCSI(quantization), distribution_ret);
}

ccs_result_t
ccs_create_uniform_float_distribution(ccs_float_t         lower,
                                      ccs_float_t         upper,
                                      ccs_scale_type_t    scale,
                                      ccs_float_t         quantization,
                                      ccs_distribution_t *distribution_ret) {
	return ccs_create_uniform_distribution(CCS_NUM_FLOAT, CCSF(lower), CCSF(upper),
	                                       scale, CCSF(quantization),
	                                       distribution_ret);
}

ccs_result_t
ccs_create_uniform_int_distribution(ccs_int_t           lower,
                                    ccs_int_t           upper,
                                    ccs_scale_type_t    scale,
                                    ccs_int_t           quantization,
                                    ccs_distribution_t *distribution_ret) {
	return ccs_create_uniform_distribution(CCS_NUM_INTEGER, CCSI(lower), CCSI(upper),
	                                       scale, CCSI(quantization),
	                                       distribution_ret);
}

