#include "cconfigspace_internal.h"
#include "distribution_internal.h"

static inline _ccs_distribution_ops_t *
_ccs_distribution_get_ops(ccs_distribution_t distribution)
{
	return (_ccs_distribution_ops_t *)distribution->obj.ops;
}

ccs_error_t
ccs_distribution_get_type(
	ccs_distribution_t       distribution,
	ccs_distribution_type_t *type_ret)
{
	CCS_CHECK_OBJ(distribution, CCS_DISTRIBUTION);
	CCS_CHECK_PTR(type_ret);
	*type_ret =
		((_ccs_distribution_common_data_t *)(distribution->data))->type;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_distribution_get_data_types(
	ccs_distribution_t  distribution,
	ccs_numeric_type_t *data_types_ret)
{
	CCS_CHECK_OBJ(distribution, CCS_DISTRIBUTION);
	CCS_CHECK_PTR(data_types_ret);
	_ccs_distribution_common_data_t *d =
		(_ccs_distribution_common_data_t *)(distribution->data);
	for (size_t i = 0; i < d->dimension; i++)
		data_types_ret[i] = d->data_types[i];
	return CCS_SUCCESS;
}

ccs_error_t
ccs_distribution_get_dimension(
	ccs_distribution_t distribution,
	size_t            *dimension_ret)
{
	CCS_CHECK_OBJ(distribution, CCS_DISTRIBUTION);
	CCS_CHECK_PTR(dimension_ret);
	*dimension_ret =
		((_ccs_distribution_common_data_t *)(distribution->data))
			->dimension;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_distribution_get_bounds(
	ccs_distribution_t distribution,
	ccs_interval_t    *interval_ret)
{
	CCS_CHECK_OBJ(distribution, CCS_DISTRIBUTION);
	CCS_CHECK_PTR(interval_ret);
	_ccs_distribution_ops_t *ops = _ccs_distribution_get_ops(distribution);
	CCS_VALIDATE(ops->get_bounds(distribution->data, interval_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_distribution_check_oversampling(
	ccs_distribution_t distribution,
	ccs_interval_t    *intervals,
	ccs_bool_t        *oversamplings)
{
	CCS_CHECK_OBJ(distribution, CCS_DISTRIBUTION);
	CCS_CHECK_PTR(intervals);
	CCS_CHECK_PTR(oversamplings);
	size_t dim = ((_ccs_distribution_common_data_t *)(distribution->data))
			     ->dimension;

	ccs_interval_t *d_intervals =
		(ccs_interval_t *)alloca(sizeof(ccs_interval_t) * dim);

	CCS_VALIDATE(ccs_distribution_get_bounds(distribution, d_intervals));

	for (size_t i = 0; i < dim; i++) {
		ccs_interval_t intersection;
		CCS_VALIDATE(ccs_interval_intersect(
			d_intervals + i, intervals + i, &intersection));

		ccs_bool_t eql;
		CCS_VALIDATE(ccs_interval_equal(
			d_intervals + i, &intersection, &eql));
		oversamplings[i] = eql ? CCS_FALSE : CCS_TRUE;
	}
	return CCS_SUCCESS;
}

ccs_error_t
ccs_distribution_sample(
	ccs_distribution_t distribution,
	ccs_rng_t          rng,
	ccs_numeric_t     *value_ret)
{
	CCS_CHECK_OBJ(distribution, CCS_DISTRIBUTION);
	CCS_CHECK_OBJ(rng, CCS_RNG);
	CCS_CHECK_PTR(value_ret);
	_ccs_distribution_ops_t *ops = _ccs_distribution_get_ops(distribution);
	CCS_VALIDATE(ops->samples(distribution->data, rng, 1, value_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_distribution_samples(
	ccs_distribution_t distribution,
	ccs_rng_t          rng,
	size_t             num_values,
	ccs_numeric_t     *values)
{
	CCS_CHECK_OBJ(distribution, CCS_DISTRIBUTION);
	CCS_CHECK_OBJ(rng, CCS_RNG);
	if (!num_values)
		return CCS_SUCCESS;
	CCS_CHECK_ARY(num_values, values);
	_ccs_distribution_ops_t *ops = _ccs_distribution_get_ops(distribution);
	CCS_VALIDATE(ops->samples(distribution->data, rng, num_values, values));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_distribution_strided_samples(
	ccs_distribution_t distribution,
	ccs_rng_t          rng,
	size_t             num_values,
	size_t             stride,
	ccs_numeric_t     *values)
{
	CCS_CHECK_OBJ(distribution, CCS_DISTRIBUTION);
	CCS_CHECK_OBJ(rng, CCS_RNG);
	CCS_REFUTE(
		stride <
			((_ccs_distribution_common_data_t *)(distribution->data))
				->dimension,
		CCS_INVALID_VALUE);
	if (!num_values)
		return CCS_SUCCESS;
	CCS_CHECK_ARY(num_values, values);
	_ccs_distribution_ops_t *ops = _ccs_distribution_get_ops(distribution);
	CCS_VALIDATE(ops->strided_samples(
		distribution->data, rng, num_values, stride, values));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_distribution_soa_samples(
	ccs_distribution_t distribution,
	ccs_rng_t          rng,
	size_t             num_values,
	ccs_numeric_t    **values)
{
	CCS_CHECK_OBJ(distribution, CCS_DISTRIBUTION);
	CCS_CHECK_OBJ(rng, CCS_RNG);
	if (!num_values)
		return CCS_SUCCESS;
	CCS_CHECK_ARY(num_values, values);
	_ccs_distribution_ops_t *ops = _ccs_distribution_get_ops(distribution);
	CCS_VALIDATE(
		ops->soa_samples(distribution->data, rng, num_values, values));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_distribution_parameters_samples(
	ccs_distribution_t distribution,
	ccs_rng_t          rng,
	ccs_parameter_t   *parameters,
	size_t             num_values,
	ccs_datum_t       *values)
{
	CCS_CHECK_OBJ(distribution, CCS_DISTRIBUTION);
	CCS_CHECK_OBJ(rng, CCS_RNG);
	if (!num_values)
		return CCS_SUCCESS;
	CCS_CHECK_ARY(num_values, parameters);
	CCS_CHECK_ARY(num_values, values);
	ccs_error_t err = CCS_SUCCESS;
	size_t dim = ((_ccs_distribution_common_data_t *)(distribution->data))
			     ->dimension;
	if (dim == 1) {
		CCS_VALIDATE(ccs_parameter_samples(
			parameters[0], distribution, rng, num_values, values));
		return CCS_SUCCESS;
	}

	ccs_bool_t  oversampling = CCS_FALSE;
	ccs_bool_t *oversamplings =
		(ccs_bool_t *)alloca(dim * sizeof(ccs_bool_t));
	ccs_interval_t *intervals =
		(ccs_interval_t *)alloca(dim * sizeof(ccs_interval_t));
	ccs_numeric_t **p_vs =
		(ccs_numeric_t **)alloca(dim * sizeof(ccs_numeric_t *));

	for (size_t i = 0; i < dim; i++)
		CCS_VALIDATE(ccs_parameter_sampling_interval(
			parameters[i], intervals + i));
	CCS_VALIDATE(ccs_distribution_check_oversampling(
		distribution, intervals, oversamplings));

	for (size_t i = 0; i < dim; i++)
		if (oversamplings[i]) {
			oversampling = CCS_TRUE;
			break;
		}
	uintptr_t mem = (uintptr_t)malloc(
		num_values * dim *
		(sizeof(ccs_numeric_t) + sizeof(ccs_datum_t)));
	CCS_REFUTE(!mem, CCS_OUT_OF_MEMORY);
	ccs_datum_t *ds = (ccs_datum_t *)mem;
	p_vs[0] =
		(ccs_numeric_t *)(mem + num_values * dim * sizeof(ccs_datum_t));
	for (size_t i = 1; i < dim; i++)
		p_vs[i] = p_vs[i - 1] + num_values;
	CCS_VALIDATE_ERR_GOTO(
		err,
		ccs_distribution_soa_samples(
			distribution, rng, num_values, p_vs),
		errmem);

	for (size_t i = 0; i < dim; i++)
		CCS_VALIDATE_ERR_GOTO(
			err,
			ccs_parameter_convert_samples(
				parameters[i], oversamplings[i], num_values,
				p_vs[i], ds + i * num_values),
			errmem);

	if (!oversampling) {
		for (size_t j = 0; j < num_values; j++)
			for (size_t i = 0; i < dim; i++)
				values[j * dim + i] = ds[i * num_values + j];
	} else {
		size_t found = 0;
		for (size_t j = 0; j < num_values; j++) {
			int discard = 0;
			for (size_t i = 0; i < dim; i++) {
				if (ds[i * num_values + j].type != CCS_INACTIVE)
					values[found * dim + i] =
						ds[i * num_values + j];
				else {
					discard = 1;
					break;
				}
			}
			if (!discard)
				found++;
		}
		size_t coeff = 2;
		while (found < num_values) {
			CCS_REFUTE_ERR_GOTO(
				err, coeff > 32, CCS_SAMPLING_UNSUCCESSFUL,
				errmem);
			size_t    buff_len = (num_values - found) * coeff;
			uintptr_t oldmem   = mem;
			mem                = (uintptr_t)realloc(
                                (void *)oldmem, buff_len * dim *
                                                        (sizeof(ccs_numeric_t) +
                                                         sizeof(ccs_datum_t)));
			if (CCS_UNLIKELY(!mem)) {
				if (oldmem)
					free((void *)oldmem);
				CCS_RAISE(
					CCS_OUT_OF_MEMORY,
					"Out of memory to reallocate array");
			}
			ccs_datum_t *ds = (ccs_datum_t *)mem;
			p_vs[0] =
				(ccs_numeric_t
					 *)(mem + buff_len * dim * sizeof(ccs_datum_t));
			for (size_t i = 1; i < dim; i++)
				p_vs[i] = p_vs[i - 1] + buff_len;
			CCS_VALIDATE_ERR_GOTO(
				err,
				ccs_distribution_soa_samples(
					distribution, rng, buff_len, p_vs),
				errmem);
			for (size_t i = 0; i < dim; i++) {
				CCS_VALIDATE_ERR_GOTO(
					err,
					ccs_parameter_convert_samples(
						parameters[i], oversamplings[i],
						buff_len, p_vs[i],
						ds + i * buff_len),
					errmem);
			}

			for (size_t j = 0; j < buff_len && found < num_values;
			     j++) {
				int discard = 0;
				for (size_t i = 0; i < dim; i++) {
					if (ds[i * buff_len + j].type !=
					    CCS_INACTIVE)
						values[found * dim + i] =
							ds[i * buff_len + j];
					else {
						discard = 1;
						break;
					}
				}
				if (!discard)
					found++;
			}
			coeff <<= 1;
		}
	}
errmem:
	free((void *)mem);
	return err;
}

ccs_error_t
ccs_distribution_parameters_sample(
	ccs_distribution_t distribution,
	ccs_rng_t          rng,
	ccs_parameter_t   *parameters,
	ccs_datum_t       *results)
{
	CCS_VALIDATE(ccs_distribution_parameters_samples(
		distribution, rng, parameters, 1, results));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_create_normal_float_distribution(
	ccs_float_t         mu,
	ccs_float_t         sigma,
	ccs_scale_type_t    scale,
	ccs_float_t         quantization,
	ccs_distribution_t *distribution_ret)
{
	CCS_VALIDATE(ccs_create_normal_distribution(
		CCS_NUM_FLOAT, mu, sigma, scale, CCSF(quantization),
		distribution_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_create_normal_int_distribution(
	ccs_float_t         mu,
	ccs_float_t         sigma,
	ccs_scale_type_t    scale,
	ccs_int_t           quantization,
	ccs_distribution_t *distribution_ret)
{
	CCS_VALIDATE(ccs_create_normal_distribution(
		CCS_NUM_INTEGER, mu, sigma, scale, CCSI(quantization),
		distribution_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_create_uniform_float_distribution(
	ccs_float_t         lower,
	ccs_float_t         upper,
	ccs_scale_type_t    scale,
	ccs_float_t         quantization,
	ccs_distribution_t *distribution_ret)
{
	CCS_VALIDATE(ccs_create_uniform_distribution(
		CCS_NUM_FLOAT, CCSF(lower), CCSF(upper), scale,
		CCSF(quantization), distribution_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_create_uniform_int_distribution(
	ccs_int_t           lower,
	ccs_int_t           upper,
	ccs_scale_type_t    scale,
	ccs_int_t           quantization,
	ccs_distribution_t *distribution_ret)
{
	CCS_VALIDATE(ccs_create_uniform_distribution(
		CCS_NUM_INTEGER, CCSI(lower), CCSI(upper), scale,
		CCSI(quantization), distribution_ret));
	return CCS_SUCCESS;
}
