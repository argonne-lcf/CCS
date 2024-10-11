#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <math.h>
#include <string.h>
#include "cconfigspace_internal.h"
#include "distribution_internal.h"
#include "rng_internal.h"

struct _ccs_distribution_mixture_data_s {
	_ccs_distribution_common_data_t common_data;
	size_t                          num_distributions;
	ccs_distribution_t             *distributions;
	ccs_interval_t                 *bounds;
	ccs_float_t                    *weights;
};
typedef struct _ccs_distribution_mixture_data_s _ccs_distribution_mixture_data_t;

static ccs_result_t
_ccs_distribution_mixture_del(ccs_object_t o)
{
	struct _ccs_distribution_mixture_data_s *data =
		(struct _ccs_distribution_mixture_data_s
			 *)(((ccs_distribution_t)o)->data);
	for (size_t i = 0; i < data->num_distributions; i++) {
		ccs_release_object(data->distributions[i]);
	}
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_distribution_mixture_data(
	_ccs_distribution_mixture_data_t *data,
	size_t                           *cum_size,
	_ccs_object_serialize_options_t  *opts)
{
	*cum_size += _ccs_serialize_bin_size_ccs_distribution_common_data(
		&data->common_data);
	*cum_size += _ccs_serialize_bin_size_size(data->num_distributions);
	for (size_t i = 0; i < data->num_distributions; i++) {
		*cum_size += _ccs_serialize_bin_size_ccs_float(
			data->weights[i + 1] - data->weights[i]);
		CCS_VALIDATE(_ccs_object_serialize_size_with_opts(
			data->distributions[i], CCS_SERIALIZE_FORMAT_BINARY,
			cum_size, opts));
	}
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_distribution_mixture_data(
	_ccs_distribution_mixture_data_t *data,
	size_t                           *buffer_size,
	char                            **buffer,
	_ccs_object_serialize_options_t  *opts)
{
	CCS_VALIDATE(_ccs_serialize_bin_ccs_distribution_common_data(
		&data->common_data, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_size(
		data->num_distributions, buffer_size, buffer));
	for (size_t i = 0; i < data->num_distributions; i++) {
		CCS_VALIDATE(_ccs_serialize_bin_ccs_float(
			data->weights[i + 1] - data->weights[i], buffer_size,
			buffer));
		CCS_VALIDATE(_ccs_object_serialize_with_opts(
			data->distributions[i], CCS_SERIALIZE_FORMAT_BINARY,
			buffer_size, buffer, opts));
	}
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_distribution_mixture(
	ccs_distribution_t               distribution,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	_ccs_distribution_mixture_data_t *data =
		(_ccs_distribution_mixture_data_t *)(distribution->data);
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_distribution_mixture_data(
		data, cum_size, opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_distribution_mixture(
	ccs_distribution_t               distribution,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	_ccs_distribution_mixture_data_t *data =
		(_ccs_distribution_mixture_data_t *)(distribution->data);
	CCS_VALIDATE(_ccs_serialize_bin_ccs_distribution_mixture_data(
		data, buffer_size, buffer, opts));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_distribution_mixture_serialize_size(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_distribution_mixture(
			(ccs_distribution_t)object, cum_size, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_distribution_mixture_serialize(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_distribution_mixture(
			(ccs_distribution_t)object, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_distribution_mixture_get_bounds(
	_ccs_distribution_data_t *data,
	ccs_interval_t           *interval_ret);

static ccs_result_t
_ccs_distribution_mixture_samples(
	_ccs_distribution_data_t *data,
	ccs_rng_t                 rng,
	size_t                    num_values,
	ccs_numeric_t            *values);

static ccs_result_t
_ccs_distribution_mixture_strided_samples(
	_ccs_distribution_data_t *data,
	ccs_rng_t                 rng,
	size_t                    num_values,
	size_t                    stride,
	ccs_numeric_t            *values);

static ccs_result_t
_ccs_distribution_mixture_soa_samples(
	_ccs_distribution_data_t *data,
	ccs_rng_t                 rng,
	size_t                    num_values,
	ccs_numeric_t           **values);

static _ccs_distribution_ops_t _ccs_distribution_mixture_ops = {
	{&_ccs_distribution_mixture_del,
	 &_ccs_distribution_mixture_serialize_size,
	 &_ccs_distribution_mixture_serialize},
	&_ccs_distribution_mixture_samples,
	&_ccs_distribution_mixture_get_bounds,
	&_ccs_distribution_mixture_strided_samples,
	&_ccs_distribution_mixture_soa_samples};

ccs_result_t
ccs_create_mixture_distribution(
	size_t              num_distributions,
	ccs_distribution_t *distributions,
	ccs_float_t        *weights,
	ccs_distribution_t *distribution_ret)
{
	CCS_CHECK_ARY(num_distributions, distributions);
	CCS_CHECK_ARY(num_distributions, weights);
	CCS_CHECK_PTR(distribution_ret);
	CCS_REFUTE(
		!num_distributions || num_distributions > INT64_MAX,
		CCS_RESULT_ERROR_INVALID_VALUE);
	ccs_float_t  weights_sum = 0.0;

	size_t       dimension, i, j;
	ccs_result_t err;
	CCS_VALIDATE(
		ccs_distribution_get_dimension(distributions[0], &dimension));
	for (i = 1; i < num_distributions; i++) {
		size_t other_dimension;
		CCS_VALIDATE(ccs_distribution_get_dimension(
			distributions[i], &other_dimension));
		CCS_REFUTE(
			dimension != other_dimension,
			CCS_RESULT_ERROR_INVALID_DISTRIBUTION);
	}

	for (i = 0; i < num_distributions; i++) {
		CCS_REFUTE(weights[i] < 0.0, CCS_RESULT_ERROR_INVALID_VALUE);
		weights_sum += weights[i];
	}
	CCS_REFUTE(weights_sum == 0.0, CCS_RESULT_ERROR_INVALID_VALUE);
	ccs_float_t weights_sum_inverse = 1.0 / weights_sum;
	CCS_REFUTE(
		isnan(weights_sum_inverse) || !isfinite(weights_sum_inverse),
		CCS_RESULT_ERROR_INVALID_VALUE);

	uintptr_t                         mem;
	uintptr_t                         cur_mem;
	uintptr_t                         tmp_mem;
	ccs_interval_t                   *bounds_tmp;
	ccs_numeric_type_t               *data_types_tmp;
	ccs_distribution_t                distrib;
	_ccs_distribution_mixture_data_t *distrib_data;
	mem = (uintptr_t)calloc(
		1, sizeof(struct _ccs_distribution_s) +
			   sizeof(_ccs_distribution_mixture_data_t) +
			   sizeof(ccs_distribution_t) * num_distributions +
			   sizeof(ccs_interval_t) * dimension +
			   sizeof(ccs_float_t) * (num_distributions + 1) +
			   sizeof(ccs_numeric_type_t) * dimension);

	CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	cur_mem = mem;

	tmp_mem = (uintptr_t)calloc(
		1, sizeof(ccs_interval_t) * num_distributions +
			   sizeof(ccs_numeric_type_t) * dimension);
	CCS_REFUTE_ERR_GOTO(
		err, !tmp_mem, CCS_RESULT_ERROR_OUT_OF_MEMORY, memory);
	bounds_tmp = (ccs_interval_t *)tmp_mem;
	data_types_tmp =
		(ccs_numeric_type_t
			 *)(tmp_mem + sizeof(ccs_interval_t) * num_distributions);

	distrib = (ccs_distribution_t)cur_mem;
	cur_mem += sizeof(struct _ccs_distribution_s);
	_ccs_object_init(
		&(distrib->obj), CCS_OBJECT_TYPE_DISTRIBUTION,
		(_ccs_object_ops_t *)&_ccs_distribution_mixture_ops);
	distrib_data = (_ccs_distribution_mixture_data_t *)(cur_mem);
	cur_mem += sizeof(_ccs_distribution_mixture_data_t);
	distrib_data->common_data.type      = CCS_DISTRIBUTION_TYPE_MIXTURE;
	distrib_data->common_data.dimension = dimension;
	distrib_data->num_distributions     = num_distributions;
	distrib_data->distributions         = (ccs_distribution_t *)(cur_mem);
	cur_mem += sizeof(ccs_distribution_t) * num_distributions;
	distrib_data->bounds = (ccs_interval_t *)(cur_mem);
	cur_mem += sizeof(ccs_interval_t) * dimension;
	distrib_data->weights = (ccs_float_t *)(cur_mem);
	cur_mem += sizeof(ccs_float_t) * (num_distributions + 1);
	distrib_data->common_data.data_types = (ccs_numeric_type_t *)(cur_mem);
	cur_mem += sizeof(ccs_numeric_type_t) * dimension;

	CCS_VALIDATE_ERR_GOTO(
		err,
		ccs_distribution_get_data_types(
			distributions[0], distrib_data->common_data.data_types),
		tmpmemory);
	for (i = 1; i < num_distributions; i++) {
		CCS_VALIDATE_ERR_GOTO(
			err,
			ccs_distribution_get_data_types(
				distributions[i], data_types_tmp),
			tmpmemory);
		for (j = 0; j < dimension; j++)
			CCS_REFUTE_ERR_GOTO(
				err,
				distrib_data->common_data.data_types[j] !=
					data_types_tmp[j],
				CCS_RESULT_ERROR_INVALID_DISTRIBUTION,
				tmpmemory);
	}

	CCS_VALIDATE_ERR_GOTO(
		err,
		ccs_distribution_get_bounds(
			distributions[0], distrib_data->bounds),
		tmpmemory);
	for (i = 1; i < num_distributions; i++) {
		CCS_VALIDATE_ERR_GOTO(
			err,
			ccs_distribution_get_bounds(
				distributions[i], bounds_tmp),
			tmpmemory);
		for (j = 0; j < dimension; j++) {
			CCS_VALIDATE_ERR_GOTO(
				err,
				ccs_interval_union(
					distrib_data->bounds + j,
					bounds_tmp + j,
					distrib_data->bounds + j),
				tmpmemory);
		}
	}

	distrib_data->weights[0] = 0.0;
	for (i = 1; i <= num_distributions; i++)
		distrib_data->weights[i] = distrib_data->weights[i - 1] +
					   weights[i - 1] * weights_sum_inverse;
	distrib_data->weights[num_distributions] = 1.0;
	CCS_REFUTE_ERR_GOTO(
		err, 1.0 < distrib_data->weights[num_distributions - 1],
		CCS_RESULT_ERROR_INVALID_VALUE, memory);
	for (i = 0; i < num_distributions; i++) {
		CCS_VALIDATE_ERR_GOTO(
			err, ccs_retain_object(distributions[i]), distrib);
		distrib_data->distributions[i] = distributions[i];
	}
	distrib->data     = (_ccs_distribution_data_t *)distrib_data;
	*distribution_ret = distrib;
	free((void *)tmp_mem);
	return CCS_RESULT_SUCCESS;
distrib:
	for (i = 0; i < num_distributions; i++) {
		if (distrib_data->distributions[i])
			ccs_release_object(distributions[i]);
	}
tmpmemory:
	_ccs_object_deinit(&(distrib->obj));
	free((void *)tmp_mem);
memory:
	free((void *)mem);
	return err;
}

static ccs_result_t
_ccs_distribution_mixture_get_bounds(
	_ccs_distribution_data_t *data,
	ccs_interval_t           *interval_ret)
{
	_ccs_distribution_mixture_data_t *d =
		(_ccs_distribution_mixture_data_t *)data;
	memcpy(interval_ret, d->bounds,
	       d->common_data.dimension * sizeof(ccs_interval_t));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_distribution_mixture_samples(
	_ccs_distribution_data_t *data,
	ccs_rng_t                 rng,
	size_t                    num_values,
	ccs_numeric_t            *values)
{
	_ccs_distribution_mixture_data_t *d =
		(_ccs_distribution_mixture_data_t *)data;
	size_t   dim  = d->common_data.dimension;
	gsl_rng *grng = rng->data->rng;

	for (size_t i = 0; i < num_values; i++) {
		ccs_float_t rnd   = gsl_rng_uniform(grng);
		ccs_int_t   index = _ccs_dichotomic_search(
                        d->num_distributions, d->weights, rnd);
		CCS_VALIDATE(_ccs_distribution_get_ops(d->distributions[index])
				     ->samples(
					     d->distributions[index]->data, rng,
					     1, values + i * dim));
	}
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_distribution_mixture_strided_samples(
	_ccs_distribution_data_t *data,
	ccs_rng_t                 rng,
	size_t                    num_values,
	size_t                    stride,
	ccs_numeric_t            *values)
{
	_ccs_distribution_mixture_data_t *d =
		(_ccs_distribution_mixture_data_t *)data;
	gsl_rng *grng = rng->data->rng;

	for (size_t i = 0; i < num_values; i++) {
		ccs_float_t rnd   = gsl_rng_uniform(grng);
		ccs_int_t   index = _ccs_dichotomic_search(
                        d->num_distributions, d->weights, rnd);
		CCS_VALIDATE(_ccs_distribution_get_ops(d->distributions[index])
				     ->samples(
					     d->distributions[index]->data, rng,
					     1, values + i * stride));
	}
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_distribution_mixture_soa_samples(
	_ccs_distribution_data_t *data,
	ccs_rng_t                 rng,
	size_t                    num_values,
	ccs_numeric_t           **values)
{
	_ccs_distribution_mixture_data_t *d =
		(_ccs_distribution_mixture_data_t *)data;
	size_t          dim    = d->common_data.dimension;
	int             needed = 0;

	ccs_numeric_t **p_values =
		(ccs_numeric_t **)alloca(dim * sizeof(ccs_numeric_t *));
	for (size_t i = 0; i < dim; i++)
		if (values[i]) {
			needed = 1;
			break;
		}
	if (!needed)
		return CCS_RESULT_SUCCESS;

	gsl_rng *grng = rng->data->rng;

	for (size_t i = 0; i < num_values; i++) {
		ccs_float_t rnd   = gsl_rng_uniform(grng);
		ccs_int_t   index = _ccs_dichotomic_search(
                        d->num_distributions, d->weights, rnd);
		for (size_t j = 0; j < dim; j++)
			if (values[j])
				p_values[j] = values[j] + i;
		CCS_VALIDATE(_ccs_distribution_get_ops(d->distributions[index])
				     ->soa_samples(
					     d->distributions[index]->data, rng,
					     1, p_values));
	}
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_mixture_distribution_get_distributions(
	ccs_distribution_t  distribution,
	size_t              num_distributions,
	ccs_distribution_t *distributions,
	size_t             *num_distributions_ret)
{
	CCS_CHECK_DISTRIBUTION(distribution, CCS_DISTRIBUTION_TYPE_MIXTURE);
	CCS_CHECK_ARY(num_distributions, distributions);
	CCS_REFUTE(
		!distributions && !num_distributions_ret,
		CCS_RESULT_ERROR_INVALID_VALUE);
	_ccs_distribution_mixture_data_t *data =
		(_ccs_distribution_mixture_data_t *)distribution->data;
	if (distributions) {
		CCS_REFUTE(
			num_distributions < data->num_distributions,
			CCS_RESULT_ERROR_INVALID_VALUE);
		for (size_t i = 0; i < data->num_distributions; i++)
			distributions[i] = data->distributions[i];
		for (size_t i = data->num_distributions; i < num_distributions;
		     i++)
			distributions[i] = NULL;
	}
	if (num_distributions_ret)
		*num_distributions_ret = data->num_distributions;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_mixture_distribution_get_weights(
	ccs_distribution_t distribution,
	size_t             num_weights,
	ccs_float_t       *weights,
	size_t            *num_weights_ret)
{
	CCS_CHECK_DISTRIBUTION(distribution, CCS_DISTRIBUTION_TYPE_MIXTURE);
	CCS_CHECK_ARY(num_weights, weights);
	CCS_REFUTE(
		!weights && !num_weights_ret, CCS_RESULT_ERROR_INVALID_VALUE);
	_ccs_distribution_mixture_data_t *data =
		(_ccs_distribution_mixture_data_t *)distribution->data;
	if (weights) {
		CCS_REFUTE(
			num_weights < data->num_distributions,
			CCS_RESULT_ERROR_INVALID_VALUE);
		for (size_t i = 0; i < data->num_distributions; i++)
			weights[i] = data->weights[i + 1] - data->weights[i];
		for (size_t i = data->num_distributions; i < num_weights; i++)
			weights[i] = 0.0;
	}
	if (num_weights_ret)
		*num_weights_ret = data->num_distributions;
	return CCS_RESULT_SUCCESS;
}
