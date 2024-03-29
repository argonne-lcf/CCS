#include "cconfigspace_internal.h"
#include "parameter_internal.h"
#include "datum_uthash.h"
#include "datum_hash.h"
#include <string.h>

struct _ccs_parameter_categorical_data_s {
	_ccs_parameter_common_data_t common_data;
	size_t                       num_possible_values;
	_ccs_hash_datum_t           *possible_values;
	_ccs_hash_datum_t           *hash;
};
typedef struct _ccs_parameter_categorical_data_s
	_ccs_parameter_categorical_data_t;

static inline size_t
_ccs_serialize_bin_size_ccs_parameter_categorical_data(
	_ccs_parameter_categorical_data_t *data)
{
	size_t sz = _ccs_serialize_bin_size_ccs_parameter_common_data(
		&data->common_data);
	sz += _ccs_serialize_bin_size_size(data->num_possible_values);
	for (size_t i = 0; i < data->num_possible_values; i++)
		sz += _ccs_serialize_bin_size_ccs_datum(
			data->possible_values[i].d);
	return sz;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_parameter_categorical_data(
	_ccs_parameter_categorical_data_t *data,
	size_t                            *buffer_size,
	char                             **buffer)
{
	CCS_VALIDATE(_ccs_serialize_bin_ccs_parameter_common_data(
		&data->common_data, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_size(
		data->num_possible_values, buffer_size, buffer));
	for (size_t i = 0; i < data->num_possible_values; i++)
		CCS_VALIDATE(_ccs_serialize_bin_ccs_datum(
			data->possible_values[i].d, buffer_size, buffer));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_parameter_categorical_del(ccs_object_t o)
{
	ccs_parameter_t                    d = (ccs_parameter_t)o;
	_ccs_parameter_categorical_data_t *data =
		(_ccs_parameter_categorical_data_t *)(d->data);
	HASH_CLEAR(hh, data->hash);
	return CCS_RESULT_SUCCESS;
}

static inline size_t
_ccs_serialize_bin_size_ccs_parameter_categorical(ccs_parameter_t parameter)
{
	_ccs_parameter_categorical_data_t *data =
		(_ccs_parameter_categorical_data_t *)(parameter->data);
	return _ccs_serialize_bin_size_ccs_object_internal(
		       (_ccs_object_internal_t *)parameter) +
	       _ccs_serialize_bin_size_ccs_parameter_categorical_data(data);
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_parameter_categorical(
	ccs_parameter_t parameter,
	size_t         *buffer_size,
	char          **buffer)
{
	_ccs_parameter_categorical_data_t *data =
		(_ccs_parameter_categorical_data_t *)(parameter->data);
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)parameter, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_parameter_categorical_data(
		data, buffer_size, buffer));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_parameter_categorical_serialize_size(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		*cum_size += _ccs_serialize_bin_size_ccs_parameter_categorical(
			(ccs_parameter_t)object);
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data_size(
		object, format, cum_size, opts));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_parameter_categorical_serialize(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_parameter_categorical(
			(ccs_parameter_t)object, buffer_size, buffer));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data(
		object, format, buffer_size, buffer, opts));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_parameter_categorical_check_values(
	_ccs_parameter_data_t *data,
	size_t                 num_values,
	const ccs_datum_t     *values,
	ccs_datum_t           *values_ret,
	ccs_bool_t            *results)
{
	_ccs_parameter_categorical_data_t *d =
		(_ccs_parameter_categorical_data_t *)data;
	for (size_t i = 0; i < num_values; i++) {
		_ccs_hash_datum_t *p;
		ccs_bool_t         found;
		HASH_FIND(hh, d->hash, values + i, sizeof(ccs_datum_t), p);
		found      = (p ? CCS_TRUE : CCS_FALSE);
		results[i] = found;
		if (values_ret) {
			if (found) {
				values_ret[i] = p->d;
			} else {
				values_ret[i] = ccs_inactive;
			}
		}
	}
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_parameter_categorical_samples(
	_ccs_parameter_data_t *data,
	ccs_distribution_t     distribution,
	ccs_rng_t              rng,
	size_t                 num_values,
	ccs_datum_t           *values)
{
	_ccs_parameter_categorical_data_t *d =
		(_ccs_parameter_categorical_data_t *)data;
	ccs_result_t err;
	ccs_int_t   *vs = (ccs_int_t *)values + num_values;
	ccs_bool_t   oversampling;
	CCS_VALIDATE(ccs_distribution_check_oversampling(
		distribution, &(d->common_data.interval), &oversampling));
	CCS_VALIDATE(ccs_distribution_samples(
		distribution, rng, num_values, (ccs_numeric_t *)vs));
	if (!oversampling) {
		for (size_t i = 0; i < num_values; i++)
			values[i] = d->possible_values[vs[i]].d;
	} else {
		size_t found = 0;
		for (size_t i = 0; i < num_values; i++)
			if (vs[i] >= 0 &&
			    (size_t)vs[i] < d->num_possible_values)
				values[found++] = d->possible_values[vs[i]].d;
		vs           = NULL;
		size_t coeff = 2;
		while (found < num_values) {
			CCS_REFUTE(
				coeff > 32,
				CCS_RESULT_ERROR_SAMPLING_UNSUCCESSFUL);
			size_t     buff_sz = (num_values - found) * coeff;
			ccs_int_t *oldvs   = vs;
			vs                 = (ccs_int_t *)realloc(
                                oldvs, sizeof(ccs_int_t) * buff_sz);
			if (CCS_UNLIKELY(!vs)) {
				if (oldvs)
					free(oldvs);
				CCS_RAISE(
					CCS_RESULT_ERROR_OUT_OF_MEMORY,
					"Could not reallocate array");
			}
			CCS_VALIDATE_ERR_GOTO(
				err,
				ccs_distribution_samples(
					distribution, rng, buff_sz,
					(ccs_numeric_t *)vs),
				errmem);
			for (size_t i = 0; i < buff_sz && found < num_values;
			     i++)
				if (vs[i] >= 0 &&
				    (size_t)vs[i] < d->num_possible_values)
					values[found++] =
						d->possible_values[vs[i]].d;
			coeff <<= 1;
		}
		if (vs)
			free(vs);
	}
	return CCS_RESULT_SUCCESS;
errmem:
	free(vs);
	return err;
}

static ccs_result_t
_ccs_parameter_categorical_get_default_distribution(
	_ccs_parameter_data_t *data,
	ccs_distribution_t    *distribution)
{
	_ccs_parameter_categorical_data_t *d =
		(_ccs_parameter_categorical_data_t *)data;
	ccs_interval_t *interval = &(d->common_data.interval);
	CCS_VALIDATE(ccs_create_uniform_distribution(
		interval->type, interval->lower, interval->upper,
		CCS_SCALE_TYPE_LINEAR, CCSI(0), distribution));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_parameter_categorical_convert_samples(
	_ccs_parameter_data_t *data,
	ccs_bool_t             oversampling,
	size_t                 num_values,
	const ccs_numeric_t   *values,
	ccs_datum_t           *results)
{
	_ccs_parameter_categorical_data_t *d =
		(_ccs_parameter_categorical_data_t *)data;

	if (!oversampling) {
		for (size_t i = 0; i < num_values; i++)
			results[i] = d->possible_values[values[i].i].d;
	} else {
		for (size_t i = 0; i < num_values; i++) {
			ccs_int_t index = (size_t)values[i].i;
			if (index >= 0 &&
			    (size_t)index < d->num_possible_values)
				results[i] = d->possible_values[index].d;
			else
				results[i] = ccs_inactive;
		}
	}
	return CCS_RESULT_SUCCESS;
}

static _ccs_parameter_ops_t _ccs_parameter_categorical_ops = {
	{&_ccs_parameter_categorical_del,
	 &_ccs_parameter_categorical_serialize_size,
	 &_ccs_parameter_categorical_serialize},
	&_ccs_parameter_categorical_check_values,
	&_ccs_parameter_categorical_samples,
	&_ccs_parameter_categorical_get_default_distribution,
	&_ccs_parameter_categorical_convert_samples};

#undef uthash_nonfatal_oom
#define uthash_nonfatal_oom(elt)                                               \
	{                                                                      \
		HASH_CLEAR(hh, parameter_data->hash);                          \
		free((void *)mem);                                             \
		CCS_RAISE(                                                     \
			CCS_RESULT_ERROR_OUT_OF_MEMORY,                        \
			"Not enough memory to allocate hash");                 \
	}

static inline ccs_result_t
_ccs_create_categorical_parameter(
	ccs_parameter_type_t type,
	const char          *name,
	size_t               num_possible_values,
	ccs_datum_t         *possible_values,
	size_t               default_value_index,
	ccs_parameter_t     *parameter_ret)
{
	CCS_CHECK_PTR(name);
	CCS_CHECK_PTR(parameter_ret);
	CCS_CHECK_ARY(num_possible_values, possible_values);
	CCS_REFUTE(
		!num_possible_values ||
			num_possible_values <= default_value_index,
		CCS_RESULT_ERROR_INVALID_VALUE);
	CCS_REFUTE(
		num_possible_values > CCS_INT_MAX,
		CCS_RESULT_ERROR_INVALID_VALUE);
	if (type == CCS_PARAMETER_TYPE_DISCRETE)
		for (size_t i = 0; i < num_possible_values; i++)
			CCS_REFUTE(
				possible_values[i].type !=
						CCS_DATA_TYPE_FLOAT &&
					possible_values[i].type !=
						CCS_DATA_TYPE_INT,
				CCS_RESULT_ERROR_INVALID_VALUE);
	size_t size_strs = 0;
	if (type != CCS_PARAMETER_TYPE_DISCRETE)
		for (size_t i = 0; i < num_possible_values; i++) {
			CCS_REFUTE(
				possible_values[i].type > CCS_DATA_TYPE_STRING,
				CCS_RESULT_ERROR_INVALID_VALUE);
			if (possible_values[i].type == CCS_DATA_TYPE_STRING) {
				CCS_REFUTE(
					!possible_values[i].value.s,
					CCS_RESULT_ERROR_INVALID_VALUE);
				size_strs +=
					strlen(possible_values[i].value.s) + 1;
			}
		}

	uintptr_t mem = (uintptr_t)calloc(
		1, sizeof(struct _ccs_parameter_s) +
			   sizeof(_ccs_parameter_categorical_data_t) +
			   sizeof(_ccs_hash_datum_t) * num_possible_values +
			   strlen(name) + 1 + size_strs);
	CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);

	ccs_interval_t interval;
	interval.type             = CCS_NUMERIC_TYPE_INT;
	interval.lower.i          = 0;
	interval.upper.i          = (ccs_int_t)num_possible_values;
	interval.lower_included   = CCS_TRUE;
	interval.upper_included   = CCS_FALSE;

	ccs_parameter_t parameter = (ccs_parameter_t)mem;
	_ccs_object_init(
		&(parameter->obj), CCS_OBJECT_TYPE_PARAMETER,
		(_ccs_object_ops_t *)&_ccs_parameter_categorical_ops);
	_ccs_parameter_categorical_data_t *parameter_data =
		(_ccs_parameter_categorical_data_t
			 *)(mem + sizeof(struct _ccs_parameter_s));
	parameter_data->common_data.type = type;
	parameter_data->common_data.name =
		(char *)(mem + sizeof(struct _ccs_parameter_s) + sizeof(_ccs_parameter_categorical_data_t) + sizeof(_ccs_hash_datum_t) * num_possible_values);
	strcpy((char *)parameter_data->common_data.name, name);
	parameter_data->common_data.interval = interval;
	parameter_data->num_possible_values  = num_possible_values;
	_ccs_hash_datum_t *pvs =
		(_ccs_hash_datum_t
			 *)(mem + sizeof(struct _ccs_parameter_s) + sizeof(_ccs_parameter_categorical_data_t));
	parameter_data->possible_values = pvs;
	parameter_data->hash            = NULL;

	char *str_pool =
		(char *)(parameter_data->common_data.name) + strlen(name) + 1;
	for (size_t i = 0; i < num_possible_values; i++) {
		_ccs_hash_datum_t *p = NULL;
		HASH_FIND(
			hh, parameter_data->hash, possible_values + i,
			sizeof(ccs_datum_t), p);
		if (p) {
			_ccs_hash_datum_t *tmp;
			HASH_ITER(hh, parameter_data->hash, p, tmp)
			{
				HASH_DELETE(hh, parameter_data->hash, p);
			}
			free((void *)mem);
			CCS_RAISE(
				CCS_RESULT_ERROR_INVALID_VALUE,
				"Duplicate possible value");
		}
		if (possible_values[i].type == CCS_DATA_TYPE_STRING) {
			pvs[i].d = ccs_string(str_pool);
			strcpy(str_pool, possible_values[i].value.s);
			str_pool += strlen(possible_values[i].value.s) + 1;
		} else {
			pvs[i].d       = possible_values[i];
			pvs[i].d.flags = CCS_DATUM_FLAG_DEFAULT;
		}
		HASH_ADD(
			hh, parameter_data->hash, d, sizeof(ccs_datum_t),
			pvs + i);
	}
	parameter_data->common_data.default_value = pvs[default_value_index].d;
	parameter->data = (_ccs_parameter_data_t *)parameter_data;
	*parameter_ret  = parameter;
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_categorical_parameter_get_values(
	ccs_parameter_t parameter,
	size_t          num_possible_values,
	ccs_datum_t    *possible_values,
	size_t         *num_possible_values_ret)
{
	CCS_CHECK_ARY(num_possible_values, possible_values);
	CCS_REFUTE(
		!possible_values && !num_possible_values_ret,
		CCS_RESULT_ERROR_INVALID_VALUE);
	_ccs_parameter_categorical_data_t *d =
		(_ccs_parameter_categorical_data_t *)parameter->data;
	if (possible_values) {
		CCS_REFUTE(
			num_possible_values < d->num_possible_values,
			CCS_RESULT_ERROR_INVALID_VALUE);
		for (size_t i = 0; i < d->num_possible_values; i++)
			possible_values[i] = d->possible_values[i].d;
		for (size_t i = num_possible_values; i < d->num_possible_values;
		     i++)
			possible_values[i] = ccs_none;
	}
	if (num_possible_values_ret)
		*num_possible_values_ret = d->num_possible_values;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_create_categorical_parameter(
	const char      *name,
	size_t           num_possible_values,
	ccs_datum_t     *possible_values,
	size_t           default_value_index,
	ccs_parameter_t *parameter_ret)
{
	CCS_VALIDATE(_ccs_create_categorical_parameter(
		CCS_PARAMETER_TYPE_CATEGORICAL, name, num_possible_values,
		possible_values, default_value_index, parameter_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_categorical_parameter_get_values(
	ccs_parameter_t parameter,
	size_t          num_possible_values,
	ccs_datum_t    *possible_values,
	size_t         *num_possible_values_ret)
{
	CCS_CHECK_PARAMETER(parameter, CCS_PARAMETER_TYPE_CATEGORICAL);
	CCS_VALIDATE(_ccs_categorical_parameter_get_values(
		parameter, num_possible_values, possible_values,
		num_possible_values_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_ordinal_parameter_compare_values(
	ccs_parameter_t parameter,
	ccs_datum_t     value1,
	ccs_datum_t     value2,
	ccs_int_t      *comp_ret)
{
	CCS_CHECK_PARAMETER(parameter, CCS_PARAMETER_TYPE_ORDINAL);
	CCS_CHECK_PTR(comp_ret);
	_ccs_parameter_categorical_data_t *d =
		((_ccs_parameter_categorical_data_t *)(parameter->data));
	_ccs_hash_datum_t *p1, *p2;
	HASH_FIND(hh, d->hash, &value1, sizeof(ccs_datum_t), p1);
	HASH_FIND(hh, d->hash, &value2, sizeof(ccs_datum_t), p2);
	CCS_REFUTE(!p1 || !p2, CCS_RESULT_ERROR_INVALID_VALUE);
	if (p1 < p2)
		*comp_ret = -1;
	else if (p1 > p2)
		*comp_ret = 1;
	else
		*comp_ret = 0;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_create_ordinal_parameter(
	const char      *name,
	size_t           num_possible_values,
	ccs_datum_t     *possible_values,
	size_t           default_value_index,
	ccs_parameter_t *parameter_ret)
{
	CCS_VALIDATE(_ccs_create_categorical_parameter(
		CCS_PARAMETER_TYPE_ORDINAL, name, num_possible_values,
		possible_values, default_value_index, parameter_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_ordinal_parameter_get_values(
	ccs_parameter_t parameter,
	size_t          num_possible_values,
	ccs_datum_t    *possible_values,
	size_t         *num_possible_values_ret)
{
	CCS_CHECK_PARAMETER(parameter, CCS_PARAMETER_TYPE_ORDINAL);
	CCS_VALIDATE(_ccs_categorical_parameter_get_values(
		parameter, num_possible_values, possible_values,
		num_possible_values_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_create_discrete_parameter(
	const char      *name,
	size_t           num_possible_values,
	ccs_datum_t     *possible_values,
	size_t           default_value_index,
	ccs_parameter_t *parameter_ret)
{
	CCS_VALIDATE(_ccs_create_categorical_parameter(
		CCS_PARAMETER_TYPE_DISCRETE, name, num_possible_values,
		possible_values, default_value_index, parameter_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_discrete_parameter_get_values(
	ccs_parameter_t parameter,
	size_t          num_possible_values,
	ccs_datum_t    *possible_values,
	size_t         *num_possible_values_ret)
{
	CCS_CHECK_PARAMETER(parameter, CCS_PARAMETER_TYPE_DISCRETE);
	CCS_VALIDATE(_ccs_categorical_parameter_get_values(
		parameter, num_possible_values, possible_values,
		num_possible_values_ret));
	return CCS_RESULT_SUCCESS;
}
