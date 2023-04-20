#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <math.h>
#include "cconfigspace_internal.h"
#include "distribution_internal.h"

struct _ccs_distribution_roulette_data_s {
	_ccs_distribution_common_data_t common_data;
	size_t                          num_areas;
	ccs_float_t                    *areas;
};
typedef struct _ccs_distribution_roulette_data_s
	_ccs_distribution_roulette_data_t;

static ccs_error_t
_ccs_distribution_roulette_del(ccs_object_t o)
{
	(void)o;
	return CCS_SUCCESS;
}

static inline size_t
_ccs_serialize_bin_size_ccs_distribution_roulette_data(
	_ccs_distribution_roulette_data_t *data)
{
	size_t sz = _ccs_serialize_bin_size_ccs_distribution_common_data(
		&data->common_data);
	sz += _ccs_serialize_bin_size_size(data->num_areas);
	for (size_t i = 0; i < data->num_areas; i++)
		sz += _ccs_serialize_bin_size_ccs_float(
			data->areas[i + 1] - data->areas[i]);
	return sz;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_distribution_roulette_data(
	_ccs_distribution_roulette_data_t *data,
	size_t                            *buffer_size,
	char                             **buffer)
{
	CCS_VALIDATE(_ccs_serialize_bin_ccs_distribution_common_data(
		&data->common_data, buffer_size, buffer));
	CCS_VALIDATE(
		_ccs_serialize_bin_size(data->num_areas, buffer_size, buffer));
	for (size_t i = 0; i < data->num_areas; i++)
		CCS_VALIDATE(_ccs_serialize_bin_ccs_float(
			data->areas[i + 1] - data->areas[i], buffer_size,
			buffer));
	return CCS_SUCCESS;
}

static inline size_t
_ccs_serialize_bin_size_ccs_distribution_roulette(
	ccs_distribution_t distribution)
{
	_ccs_distribution_roulette_data_t *data =
		(_ccs_distribution_roulette_data_t *)(distribution->data);
	return _ccs_serialize_bin_size_ccs_object_internal(
		       (_ccs_object_internal_t *)distribution) +
	       _ccs_serialize_bin_size_ccs_distribution_roulette_data(data);
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_distribution_roulette(
	ccs_distribution_t distribution,
	size_t            *buffer_size,
	char             **buffer)
{
	_ccs_distribution_roulette_data_t *data =
		(_ccs_distribution_roulette_data_t *)(distribution->data);
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)distribution, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_distribution_roulette_data(
		data, buffer_size, buffer));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_distribution_roulette_serialize_size(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		*cum_size += _ccs_serialize_bin_size_ccs_distribution_roulette(
			(ccs_distribution_t)object);
		break;
	default:
		CCS_RAISE(
			CCS_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data_size(
		object, format, cum_size, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_distribution_roulette_serialize(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_distribution_roulette(
			(ccs_distribution_t)object, buffer_size, buffer));
		break;
	default:
		CCS_RAISE(
			CCS_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data(
		object, format, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_distribution_roulette_get_bounds(
	_ccs_distribution_data_t *data,
	ccs_interval_t           *interval_ret);

static ccs_error_t
_ccs_distribution_roulette_samples(
	_ccs_distribution_data_t *data,
	ccs_rng_t                 rng,
	size_t                    num_values,
	ccs_numeric_t            *values);

static ccs_error_t
_ccs_distribution_roulette_strided_samples(
	_ccs_distribution_data_t *data,
	ccs_rng_t                 rng,
	size_t                    num_values,
	size_t                    stride,
	ccs_numeric_t            *values);

static ccs_error_t
_ccs_distribution_roulette_soa_samples(
	_ccs_distribution_data_t *data,
	ccs_rng_t                 rng,
	size_t                    num_values,
	ccs_numeric_t           **values);

static _ccs_distribution_ops_t _ccs_distribution_roulette_ops = {
	{&_ccs_distribution_roulette_del,
	 &_ccs_distribution_roulette_serialize_size,
	 &_ccs_distribution_roulette_serialize},
	&_ccs_distribution_roulette_samples,
	&_ccs_distribution_roulette_get_bounds,
	&_ccs_distribution_roulette_strided_samples,
	&_ccs_distribution_roulette_soa_samples};

static ccs_error_t
_ccs_distribution_roulette_get_bounds(
	_ccs_distribution_data_t *data,
	ccs_interval_t           *interval_ret)
{
	_ccs_distribution_roulette_data_t *d =
		(_ccs_distribution_roulette_data_t *)data;

	interval_ret->type           = CCS_NUMERIC_TYPE_INT;
	interval_ret->lower          = CCSI(INT64_C(0));
	interval_ret->upper          = CCSI((ccs_int_t)(d->num_areas));
	interval_ret->lower_included = CCS_TRUE;
	interval_ret->upper_included = CCS_FALSE;
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_distribution_roulette_samples(
	_ccs_distribution_data_t *data,
	ccs_rng_t                 rng,
	size_t                    num_values,
	ccs_numeric_t            *values)
{
	_ccs_distribution_roulette_data_t *d =
		(_ccs_distribution_roulette_data_t *)data;

	gsl_rng *grng;
	CCS_VALIDATE(ccs_rng_get_gsl_rng(rng, &grng));

	for (size_t i = 0; i < num_values; i++) {
		ccs_float_t rnd = gsl_rng_uniform(grng);
		ccs_int_t   index =
			ccs_dichotomic_search(d->num_areas, d->areas, rnd);
		values[i].i = index;
	}
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_distribution_roulette_strided_samples(
	_ccs_distribution_data_t *data,
	ccs_rng_t                 rng,
	size_t                    num_values,
	size_t                    stride,
	ccs_numeric_t            *values)
{
	_ccs_distribution_roulette_data_t *d =
		(_ccs_distribution_roulette_data_t *)data;

	gsl_rng *grng;
	CCS_VALIDATE(ccs_rng_get_gsl_rng(rng, &grng));

	for (size_t i = 0; i < num_values; i++) {
		ccs_float_t rnd = gsl_rng_uniform(grng);
		ccs_int_t   index =
			ccs_dichotomic_search(d->num_areas, d->areas, rnd);
		values[i * stride].i = index;
	}
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_distribution_roulette_soa_samples(
	_ccs_distribution_data_t *data,
	ccs_rng_t                 rng,
	size_t                    num_values,
	ccs_numeric_t           **values)
{
	if (*values)
		return _ccs_distribution_roulette_samples(
			data, rng, num_values, *values);
	return CCS_SUCCESS;
}

ccs_error_t
ccs_create_roulette_distribution(
	size_t              num_areas,
	ccs_float_t        *areas,
	ccs_distribution_t *distribution_ret)
{
	CCS_CHECK_ARY(num_areas, areas);
	CCS_CHECK_PTR(distribution_ret);
	CCS_REFUTE(!num_areas || num_areas > INT64_MAX, CCS_INVALID_VALUE);
	ccs_float_t sum_areas = 0.0;

	for (size_t i = 0; i < num_areas; i++) {
		CCS_REFUTE(areas[i] < 0.0, CCS_INVALID_VALUE);
		sum_areas += areas[i];
	}
	CCS_REFUTE(sum_areas == 0.0, CCS_INVALID_VALUE);
	ccs_float_t sum_areas_inverse = 1.0 / sum_areas;
	CCS_REFUTE(
		isnan(sum_areas_inverse) || !isfinite(sum_areas_inverse),
		CCS_INVALID_VALUE);

	uintptr_t mem = (uintptr_t)calloc(
		1, sizeof(struct _ccs_distribution_s) +
			   sizeof(_ccs_distribution_roulette_data_t) +
			   sizeof(ccs_float_t) * (num_areas + 1) +
			   sizeof(ccs_numeric_type_t));
	CCS_REFUTE(!mem, CCS_OUT_OF_MEMORY);

	ccs_distribution_t distrib = (ccs_distribution_t)mem;
	_ccs_object_init(
		&(distrib->obj), CCS_OBJECT_TYPE_DISTRIBUTION,
		(_ccs_object_ops_t *)&_ccs_distribution_roulette_ops);
	_ccs_distribution_roulette_data_t *distrib_data =
		(_ccs_distribution_roulette_data_t
			 *)(mem + sizeof(struct _ccs_distribution_s));
	distrib_data->common_data.data_types =
		(ccs_numeric_type_t
			 *)(mem + sizeof(struct _ccs_distribution_s) + sizeof(_ccs_distribution_roulette_data_t) + sizeof(ccs_float_t) * (num_areas + 1));
	distrib_data->common_data.type      = CCS_DISTRIBUTION_TYPE_ROULETTE;
	distrib_data->common_data.dimension = 1;
	distrib_data->common_data.data_types[0] = CCS_NUMERIC_TYPE_INT;
	distrib_data->num_areas                 = num_areas;
	distrib_data->areas =
		(ccs_float_t
			 *)(mem + sizeof(struct _ccs_distribution_s) + sizeof(_ccs_distribution_roulette_data_t));

	distrib_data->areas[0] = 0.0;
	for (size_t i = 1; i <= num_areas; i++)
		distrib_data->areas[i] =
			distrib_data->areas[i - 1] + areas[i - 1];
	for (size_t i = 1; i <= num_areas; i++)
		distrib_data->areas[i] *= sum_areas_inverse;
	distrib_data->areas[num_areas] = 1.0;
	distrib->data     = (_ccs_distribution_data_t *)distrib_data;
	*distribution_ret = distrib;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_roulette_distribution_get_num_areas(
	ccs_distribution_t distribution,
	size_t            *num_areas_ret)
{
	CCS_CHECK_DISTRIBUTION(distribution, CCS_DISTRIBUTION_TYPE_ROULETTE);
	CCS_CHECK_PTR(num_areas_ret);
	_ccs_distribution_roulette_data_t *data =
		(_ccs_distribution_roulette_data_t *)distribution->data;
	*num_areas_ret = data->num_areas;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_roulette_distribution_get_areas(
	ccs_distribution_t distribution,
	size_t             num_areas,
	ccs_float_t       *areas,
	size_t            *num_areas_ret)
{
	CCS_CHECK_DISTRIBUTION(distribution, CCS_DISTRIBUTION_TYPE_ROULETTE);
	CCS_CHECK_ARY(num_areas, areas);
	CCS_REFUTE(!areas && !num_areas_ret, CCS_INVALID_VALUE);
	_ccs_distribution_roulette_data_t *data =
		(_ccs_distribution_roulette_data_t *)distribution->data;
	if (areas) {
		CCS_REFUTE(num_areas < data->num_areas, CCS_INVALID_VALUE);
		for (size_t i = 0; i < data->num_areas; i++)
			areas[i] = data->areas[i + 1] - data->areas[i];
		for (size_t i = data->num_areas; i < num_areas; i++)
			areas[i] = 0.0;
	}
	if (num_areas_ret)
		*num_areas_ret = data->num_areas;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_roulette_distribution_set_areas(
	ccs_distribution_t distribution,
	size_t             num_areas,
	ccs_float_t       *areas)
{
	CCS_CHECK_DISTRIBUTION(distribution, CCS_DISTRIBUTION_TYPE_ROULETTE);
	_ccs_distribution_roulette_data_t *distrib_data =
		(_ccs_distribution_roulette_data_t *)distribution->data;
	CCS_REFUTE(num_areas != distrib_data->num_areas, CCS_INVALID_VALUE);
	CCS_CHECK_PTR(areas);

	ccs_float_t sum_areas = 0.0;
	for (size_t i = 0; i < num_areas; i++) {
		CCS_REFUTE(areas[i] < 0.0, CCS_INVALID_VALUE);
		sum_areas += areas[i];
	}
	CCS_REFUTE(sum_areas == 0.0, CCS_INVALID_VALUE);
	ccs_float_t sum_areas_inverse = 1.0 / sum_areas;
	distrib_data->areas[0]        = 0.0;
	for (size_t i = 1; i <= num_areas; i++)
		distrib_data->areas[i] =
			distrib_data->areas[i - 1] + areas[i - 1];
	for (size_t i = 1; i <= num_areas; i++)
		distrib_data->areas[i] *= sum_areas_inverse;
	distrib_data->areas[num_areas] = 1.0;
	return CCS_SUCCESS;
}
