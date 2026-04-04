#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <math.h>
#include "cconfigspace_internal.h"
#include "distribution_internal.h"
#include "cconfigspace_json.h"
#include "rng_internal.h"

struct _ccs_distribution_roulette_data_s {
	_ccs_distribution_common_data_t common_data;
	size_t                          num_areas;
	ccs_float_t                    *areas;
};
typedef struct _ccs_distribution_roulette_data_s
	_ccs_distribution_roulette_data_t;

static ccs_result_t
_ccs_distribution_roulette_del(ccs_object_t o)
{
	(void)o;
	return CCS_RESULT_SUCCESS;
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

static inline ccs_result_t
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
	return CCS_RESULT_SUCCESS;
}

static inline size_t
_ccs_serialize_bin_size_ccs_distribution_roulette(
	ccs_distribution_t distribution)
{
	_ccs_distribution_roulette_data_t *data =
		(_ccs_distribution_roulette_data_t *)(distribution->data);
	return _ccs_serialize_bin_size_ccs_distribution_roulette_data(data);
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_distribution_roulette(
	ccs_distribution_t distribution,
	size_t            *buffer_size,
	char             **buffer)
{
	_ccs_distribution_roulette_data_t *data =
		(_ccs_distribution_roulette_data_t *)(distribution->data);
	CCS_VALIDATE(_ccs_serialize_bin_ccs_distribution_roulette_data(
		data, buffer_size, buffer));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_json_ccs_distribution_roulette(
	ccs_distribution_t distribution,
	cJSON             *json)
{
	_ccs_distribution_roulette_data_t *data =
		(_ccs_distribution_roulette_data_t *)(distribution->data);
	cJSON *areas;
	CCS_VALIDATE(
		_ccs_json_add_string(json, "distribution_type", "roulette"));
	CCS_VALIDATE(_ccs_json_add_array(json, "areas", &areas));
	for (size_t i = 0; i < data->num_areas; i++)
		CCS_VALIDATE(_ccs_json_add_float_to_array(
			areas, data->areas[i + 1] - data->areas[i]));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_distribution_roulette_serialize_size(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	(void)opts;
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		*cum_size += _ccs_serialize_bin_size_ccs_distribution_roulette(
			(ccs_distribution_t)object);
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_distribution_roulette_serialize(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	(void)opts;
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_distribution_roulette(
			(ccs_distribution_t)object, buffer_size, buffer));
		break;
	case CCS_SERIALIZE_FORMAT_JSON:
		CCS_VALIDATE(_ccs_serialize_json_ccs_distribution_roulette(
			(ccs_distribution_t)object, *(cJSON **)buffer));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_distribution_roulette_get_bounds(
	_ccs_distribution_data_t *data,
	ccs_interval_t           *interval_ret);

static ccs_result_t
_ccs_distribution_roulette_samples(
	_ccs_distribution_data_t *data,
	ccs_rng_t                 rng,
	size_t                    num_values,
	ccs_numeric_t            *values);

static ccs_result_t
_ccs_distribution_roulette_strided_samples(
	_ccs_distribution_data_t *data,
	ccs_rng_t                 rng,
	size_t                    num_values,
	size_t                    stride,
	ccs_numeric_t            *values);

static ccs_result_t
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

static ccs_result_t
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
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_distribution_roulette_samples(
	_ccs_distribution_data_t *data,
	ccs_rng_t                 rng,
	size_t                    num_values,
	ccs_numeric_t            *values)
{
	_ccs_distribution_roulette_data_t *d =
		(_ccs_distribution_roulette_data_t *)data;

	gsl_rng *grng = rng->data->rng;

	for (size_t i = 0; i < num_values; i++) {
		ccs_float_t rnd = gsl_rng_uniform(grng);
		ccs_int_t   index =
			_ccs_dichotomic_search(d->num_areas, d->areas, rnd);
		values[i].i = index;
	}
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_distribution_roulette_strided_samples(
	_ccs_distribution_data_t *data,
	ccs_rng_t                 rng,
	size_t                    num_values,
	size_t                    stride,
	ccs_numeric_t            *values)
{
	_ccs_distribution_roulette_data_t *d =
		(_ccs_distribution_roulette_data_t *)data;

	gsl_rng *grng = rng->data->rng;

	for (size_t i = 0; i < num_values; i++) {
		ccs_float_t rnd = gsl_rng_uniform(grng);
		ccs_int_t   index =
			_ccs_dichotomic_search(d->num_areas, d->areas, rnd);
		values[i * stride].i = index;
	}
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_distribution_roulette_soa_samples(
	_ccs_distribution_data_t *data,
	ccs_rng_t                 rng,
	size_t                    num_values,
	ccs_numeric_t           **values)
{
	if (*values)
		return _ccs_distribution_roulette_samples(
			data, rng, num_values, *values);
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_create_roulette_distribution(
	size_t              num_areas,
	ccs_float_t        *areas,
	ccs_distribution_t *distribution_ret)
{
	CCS_CHECK_ARY(num_areas, areas);
	CCS_CHECK_PTR(distribution_ret);
	CCS_REFUTE(
		!num_areas || num_areas > INT64_MAX,
		CCS_RESULT_ERROR_INVALID_VALUE);
	ccs_float_t sum_areas_inverse;
	CCS_VALIDATE(_ccs_distribution_roulette_validate_areas(
		num_areas, areas, &sum_areas_inverse));

	size_t _sz;
	CCS_REFUTE(
		CCS_ALLOC_SIZE(
			&_sz, CCS_ALLOC_SIZE_TYPE(struct _ccs_distribution_s),
			CCS_ALLOC_SIZE_TYPE(_ccs_distribution_roulette_data_t),
			CCS_ALLOC_SIZE_ARRAY(num_areas + 1, ccs_float_t),
			CCS_ALLOC_SIZE_ARRAY(1, ccs_numeric_type_t)),
		CCS_RESULT_ERROR_OUT_OF_MEMORY);
	uintptr_t mem = (uintptr_t)calloc(1, _sz);
	CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);

	ccs_distribution_t distrib =
		CCS_ALLOC_CARVE_TYPE(mem, struct _ccs_distribution_s);
	_ccs_object_init(
		&(distrib->obj), CCS_OBJECT_TYPE_DISTRIBUTION,
		(_ccs_object_ops_t *)&_ccs_distribution_roulette_ops);
	_ccs_distribution_roulette_data_t *distrib_data =
		CCS_ALLOC_CARVE_TYPE(mem, _ccs_distribution_roulette_data_t);
	distrib_data->areas =
		CCS_ALLOC_CARVE_ARRAY(mem, num_areas + 1, ccs_float_t);
	distrib_data->common_data.data_types =
		CCS_ALLOC_CARVE_ARRAY(mem, 1, ccs_numeric_type_t);
	distrib_data->common_data.type      = CCS_DISTRIBUTION_TYPE_ROULETTE;
	distrib_data->common_data.dimension = 1;
	distrib_data->common_data.data_types[0] = CCS_NUMERIC_TYPE_INT;
	distrib_data->num_areas                 = num_areas;
	_ccs_distribution_roulette_normalize_areas(
		num_areas, areas, sum_areas_inverse, distrib_data->areas);
	distrib->data     = (_ccs_distribution_data_t *)distrib_data;
	*distribution_ret = distrib;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_roulette_distribution_get_areas(
	ccs_distribution_t distribution,
	size_t             num_areas,
	ccs_float_t       *areas,
	size_t            *num_areas_ret)
{
	CCS_CHECK_DISTRIBUTION(distribution, CCS_DISTRIBUTION_TYPE_ROULETTE);
	CCS_CHECK_ARY(num_areas, areas);
	CCS_REFUTE(!areas && !num_areas_ret, CCS_RESULT_ERROR_INVALID_VALUE);
	_ccs_distribution_roulette_data_t *data =
		(_ccs_distribution_roulette_data_t *)distribution->data;
	if (areas) {
		CCS_REFUTE(
			num_areas < data->num_areas,
			CCS_RESULT_ERROR_INVALID_VALUE);
		for (size_t i = 0; i < data->num_areas; i++)
			areas[i] = data->areas[i + 1] - data->areas[i];
		for (size_t i = data->num_areas; i < num_areas; i++)
			areas[i] = 0.0;
	}
	if (num_areas_ret)
		*num_areas_ret = data->num_areas;
	return CCS_RESULT_SUCCESS;
}
