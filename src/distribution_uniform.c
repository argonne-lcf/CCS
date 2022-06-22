#include <math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include "cconfigspace_internal.h"
#include "distribution_internal.h"

struct _ccs_distribution_uniform_data_s {
	_ccs_distribution_common_data_t common_data;
	ccs_numeric_t                   lower;
	ccs_numeric_t                   upper;
	ccs_scale_type_t                scale_type;
	ccs_numeric_t                   quantization;
	ccs_numeric_type_t              internal_type;
	ccs_numeric_t                   internal_lower;
        ccs_numeric_t                   internal_upper;
	int                             quantize;
};
typedef struct _ccs_distribution_uniform_data_s _ccs_distribution_uniform_data_t;

static ccs_error_t
_ccs_distribution_uniform_del(ccs_object_t o) {
	(void)o;
	return CCS_SUCCESS;
}

static inline size_t
_ccs_serialize_bin_size_ccs_distribution_uniform_data(
		_ccs_distribution_uniform_data_t *data) {
	return _ccs_serialize_bin_size_ccs_distribution_common_data(&data->common_data) +
	       _ccs_serialize_bin_size_ccs_numeric_type(data->common_data.data_types[0]) +
	       _ccs_serialize_bin_size_ccs_scale_type(data->scale_type) +
	       (data->common_data.data_types[0] == CCS_NUM_FLOAT ?
		_ccs_serialize_bin_size_ccs_float(data->lower.f) +
		_ccs_serialize_bin_size_ccs_float(data->upper.f) +
		_ccs_serialize_bin_size_ccs_float(data->quantization.f) :
		_ccs_serialize_bin_size_ccs_int(data->lower.i) +
		_ccs_serialize_bin_size_ccs_int(data->upper.i) +
		_ccs_serialize_bin_size_ccs_int(data->quantization.i));
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_distribution_uniform_data(
		_ccs_distribution_uniform_data_t  *data,
		size_t                            *buffer_size,
		char                             **buffer) {
	CCS_VALIDATE(_ccs_serialize_bin_ccs_distribution_common_data(
		&data->common_data, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_numeric_type(
		data->common_data.data_types[0], buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_scale_type(
		data->scale_type, buffer_size, buffer));
	if (data->common_data.data_types[0] == CCS_NUM_FLOAT) {
		CCS_VALIDATE(_ccs_serialize_bin_ccs_float(
			data->lower.f, buffer_size, buffer));
		CCS_VALIDATE(_ccs_serialize_bin_ccs_float(
			data->upper.f, buffer_size, buffer));
		CCS_VALIDATE(_ccs_serialize_bin_ccs_float(
			data->quantization.f, buffer_size, buffer));
	} else {
		CCS_VALIDATE(_ccs_serialize_bin_ccs_int(
			data->lower.i, buffer_size, buffer));
		CCS_VALIDATE(_ccs_serialize_bin_ccs_int(
			data->upper.i, buffer_size, buffer));
		CCS_VALIDATE(_ccs_serialize_bin_ccs_int(
			data->quantization.i, buffer_size, buffer));
	}
	return CCS_SUCCESS;
}

static inline size_t
_ccs_serialize_bin_size_ccs_distribution_uniform(
		ccs_distribution_t distribution) {
	_ccs_distribution_uniform_data_t *data =
		(_ccs_distribution_uniform_data_t *)(distribution->data);
	return	_ccs_serialize_bin_size_ccs_object_internal(
			(_ccs_object_internal_t *)distribution) +
	        _ccs_serialize_bin_size_ccs_distribution_uniform_data(data);
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_distribution_uniform(
		ccs_distribution_t   distribution,
		size_t              *buffer_size,
		char               **buffer) {
	_ccs_distribution_uniform_data_t *data =
		(_ccs_distribution_uniform_data_t *)(distribution->data);
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)distribution, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_distribution_uniform_data(
		data, buffer_size, buffer));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_distribution_uniform_serialize_size(
		ccs_object_t                     object,
		ccs_serialize_format_t           format,
		size_t                          *cum_size,
		_ccs_object_serialize_options_t *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		*cum_size += _ccs_serialize_bin_size_ccs_distribution_uniform(
			(ccs_distribution_t)object);
		break;
	default:
		CCS_RAISE(CCS_INVALID_VALUE, "Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data_size(
		object, format, cum_size, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_distribution_uniform_serialize(
		ccs_object_t                      object,
		ccs_serialize_format_t            format,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_distribution_uniform(
		    (ccs_distribution_t)object, buffer_size, buffer));
		break;
	default:
		CCS_RAISE(CCS_INVALID_VALUE, "Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data(
		object, format, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_distribution_uniform_get_bounds(_ccs_distribution_data_t *data,
                                     ccs_interval_t           *interval_ret);

static ccs_error_t
_ccs_distribution_uniform_samples(_ccs_distribution_data_t *data,
                                  ccs_rng_t                 rng,
                                  size_t                    num_values,
                                  ccs_numeric_t            *values);

static ccs_error_t
_ccs_distribution_uniform_strided_samples(_ccs_distribution_data_t *data,
                                          ccs_rng_t                 rng,
                                          size_t                    num_values,
                                          size_t                    stride,
                                          ccs_numeric_t            *values);

static ccs_error_t
_ccs_distribution_uniform_soa_samples(_ccs_distribution_data_t  *data,
                                      ccs_rng_t                  rng,
                                      size_t                     num_values,
                                      ccs_numeric_t            **values);

static _ccs_distribution_ops_t _ccs_distribution_uniform_ops = {
	{ &_ccs_distribution_uniform_del,
	  &_ccs_distribution_uniform_serialize_size,
	  &_ccs_distribution_uniform_serialize },
	&_ccs_distribution_uniform_samples,
	&_ccs_distribution_uniform_get_bounds,
	&_ccs_distribution_uniform_strided_samples,
	&_ccs_distribution_uniform_soa_samples
 };

static ccs_error_t
_ccs_distribution_uniform_get_bounds(_ccs_distribution_data_t *data,
                                     ccs_interval_t           *interval_ret) {
	_ccs_distribution_uniform_data_t *d = (_ccs_distribution_uniform_data_t *)data;
        ccs_numeric_t          l;
	ccs_bool_t             li;
	ccs_numeric_t          u;
	ccs_bool_t             ui;

	l = d->lower;
	li = CCS_TRUE;
	u = d->upper;
        ui = CCS_FALSE;
	interval_ret->type = d->common_data.data_types[0];
	interval_ret->lower = l;
	interval_ret->upper = u;
	interval_ret->lower_included = li;
	interval_ret->upper_included = ui;
	return CCS_SUCCESS;
}


static ccs_error_t
_ccs_distribution_uniform_strided_samples(_ccs_distribution_data_t *data,
                                          ccs_rng_t                 rng,
                                          size_t                    num_values,
                                          size_t                    stride,
                                          ccs_numeric_t            *values) {
	_ccs_distribution_uniform_data_t *d = (_ccs_distribution_uniform_data_t *)data;
	size_t i;
	const ccs_numeric_type_t  data_type      = d->common_data.data_types[0];
	const ccs_scale_type_t    scale_type     = d->scale_type;
	const ccs_numeric_t       quantization   = d->quantization;
	const ccs_numeric_t       lower          = d->lower;
	const ccs_numeric_t       internal_lower = d->internal_lower;
	const ccs_numeric_t       internal_upper = d->internal_upper;
	const int                 quantize       = d->quantize;
	gsl_rng *grng;
	CCS_VALIDATE(ccs_rng_get_gsl_rng(rng, &grng));

	if (data_type == CCS_NUM_FLOAT) {
		for (i = 0; i < num_values; i++) {
			values[i*stride].f = gsl_ran_flat(grng, internal_lower.f, internal_upper.f);
		}
		if (scale_type == CCS_LOGARITHMIC) {
			for (i = 0; i < num_values; i++)
				values[i*stride].f = exp(values[i*stride].f);
			if (quantize)
				for (i = 0; i < num_values; i++)
					values[i*stride].f = floor((values[i*stride].f - lower.f)/quantization.f) * quantization.f + lower.f;
		} else
			if (quantize)
				for (i = 0; i < num_values; i++)
					values[i*stride].f = floor(values[i*stride].f) * quantization.f + lower.f;
			else
				for (i = 0; i < num_values; i++)
					values[i*stride].f += lower.f;
	} else {
		if (scale_type == CCS_LOGARITHMIC) {
			for (i = 0; i < num_values; i++) {
				values[i*stride].i = floor(exp(gsl_ran_flat(grng, internal_lower.f, internal_upper.f)));
			}
			if (quantize)
				for (i = 0; i < num_values; i++)
					values[i*stride].i = ((values[i*stride].i - lower.i)/quantization.i) * quantization.i + lower.i;
		} else {
			for (i = 0; i < num_values; i++) {
				values[i*stride].i = gsl_rng_uniform_int(grng, internal_upper.i);
			}
			if (quantize)
				for (i = 0; i < num_values; i++)
					values[i*stride].i = values[i*stride].i * quantization.i + lower.i;
			else
				for (i = 0; i < num_values; i++)
					values[i*stride].i += lower.i;
		}
	}
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_distribution_uniform_samples(_ccs_distribution_data_t *data,
                                  ccs_rng_t                 rng,
                                  size_t                    num_values,
                                  ccs_numeric_t            *values) {
	_ccs_distribution_uniform_data_t *d = (_ccs_distribution_uniform_data_t *)data;
	size_t i;
	const ccs_numeric_type_t  data_type      = d->common_data.data_types[0];
	const ccs_scale_type_t    scale_type     = d->scale_type;
	const ccs_numeric_t       quantization   = d->quantization;
	const ccs_numeric_t       lower          = d->lower;
	const ccs_numeric_t       internal_lower = d->internal_lower;
	const ccs_numeric_t       internal_upper = d->internal_upper;
	const int                 quantize       = d->quantize;
	gsl_rng *grng;
	CCS_VALIDATE(ccs_rng_get_gsl_rng(rng, &grng));

	if (data_type == CCS_NUM_FLOAT) {
		for (i = 0; i < num_values; i++) {
			values[i].f = gsl_ran_flat(grng, internal_lower.f, internal_upper.f);
		}
		if (scale_type == CCS_LOGARITHMIC) {
			for (i = 0; i < num_values; i++)
				values[i].f = exp(values[i].f);
			if (quantize)
				for (i = 0; i < num_values; i++)
					values[i].f = floor((values[i].f - lower.f)/quantization.f) * quantization.f + lower.f;
		} else
			if (quantize)
				for (i = 0; i < num_values; i++)
					values[i].f = floor(values[i].f) * quantization.f + lower.f;
			else
				for (i = 0; i < num_values; i++)
					values[i].f += lower.f;
	} else {
		if (scale_type == CCS_LOGARITHMIC) {
			for (i = 0; i < num_values; i++) {
				values[i].i = floor(exp(gsl_ran_flat(grng, internal_lower.f, internal_upper.f)));
			}
			if (quantize)
				for (i = 0; i < num_values; i++)
					values[i].i = ((values[i].i - lower.i)/quantization.i) * quantization.i + lower.i;
		} else {
			for (i = 0; i < num_values; i++) {
				values[i].i = gsl_rng_uniform_int(grng, internal_upper.i);
			}
			if (quantize)
				for (i = 0; i < num_values; i++)
					values[i].i = values[i].i * quantization.i + lower.i;
			else
				for (i = 0; i < num_values; i++)
					values[i].i += lower.i;
		}
	}
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_distribution_uniform_soa_samples(_ccs_distribution_data_t  *data,
                                      ccs_rng_t                  rng,
                                      size_t                     num_values,
                                      ccs_numeric_t            **values) {
	if (*values)
		CCS_VALIDATE(_ccs_distribution_uniform_samples(data, rng, num_values, *values));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_create_uniform_distribution(ccs_numeric_type_t  data_type,
                                ccs_numeric_t       lower,
                                ccs_numeric_t       upper,
                                ccs_scale_type_t    scale_type,
                                ccs_numeric_t       quantization,
                                ccs_distribution_t *distribution_ret) {
	CCS_CHECK_PTR(distribution_ret);
	CCS_REFUTE(data_type != CCS_NUM_FLOAT && data_type != CCS_NUM_INTEGER, CCS_INVALID_TYPE);
	CCS_REFUTE(scale_type != CCS_LINEAR && scale_type != CCS_LOGARITHMIC, CCS_INVALID_SCALE);
	CCS_REFUTE(data_type == CCS_NUM_INTEGER && (lower.i >= upper.i || (scale_type == CCS_LOGARITHMIC && lower.i <= 0) || quantization.i < 0 || quantization.i > upper.i - lower.i), CCS_INVALID_VALUE);
	CCS_REFUTE(data_type == CCS_NUM_FLOAT && (lower.f >= upper.f || (scale_type == CCS_LOGARITHMIC && lower.f <= 0.0) || quantization.f < 0.0 || quantization.f > upper.f - lower.f), CCS_INVALID_VALUE);
	uintptr_t mem = (uintptr_t)calloc(1, sizeof(struct _ccs_distribution_s) + sizeof(_ccs_distribution_uniform_data_t) + sizeof(ccs_numeric_type_t));
	CCS_REFUTE(!mem, CCS_OUT_OF_MEMORY);
	ccs_distribution_t distrib = (ccs_distribution_t)mem;
	_ccs_object_init(&(distrib->obj), CCS_DISTRIBUTION, (_ccs_object_ops_t *)&_ccs_distribution_uniform_ops);
        _ccs_distribution_uniform_data_t * distrib_data = (_ccs_distribution_uniform_data_t *)(mem + sizeof(struct _ccs_distribution_s));
	distrib_data->common_data.data_types    = (ccs_numeric_type_t *)(mem + sizeof(struct _ccs_distribution_s) + sizeof(_ccs_distribution_uniform_data_t));
	distrib_data->common_data.type          = CCS_UNIFORM;
	distrib_data->common_data.dimension     = 1;
	distrib_data->common_data.data_types[0] = data_type;
	distrib_data->scale_type                = scale_type;
	distrib_data->quantization              = quantization;
	distrib_data->lower                     = lower;
	distrib_data->upper                     = upper;

	if (data_type == CCS_NUM_FLOAT) {
		if (quantization.f != 0.0)
			distrib_data->quantize = 1;
		if (scale_type == CCS_LOGARITHMIC) {
			distrib_data->internal_lower.f = log(lower.f);
			distrib_data->internal_upper.f = log(upper.f);
		} else {
			distrib_data->internal_lower.f = 0.0;
			distrib_data->internal_upper.f = upper.f - lower.f;
			if (distrib_data->quantize)
				distrib_data->internal_upper.f /= quantization.f;
		}
	} else {
		if (quantization.i != 0)
			distrib_data->quantize = 1;
		if (scale_type == CCS_LOGARITHMIC) {
			distrib_data->internal_lower.f = log(lower.i);
			distrib_data->internal_upper.f = log(upper.i);
		} else {
			distrib_data->internal_lower.i = 0;
			distrib_data->internal_upper.i = upper.i - lower.i;
			if (quantization.i != 0)
				distrib_data->internal_upper.i /= quantization.i;
		}
	}
	distrib->data = (_ccs_distribution_data_t *)distrib_data;
	*distribution_ret = distrib;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_uniform_distribution_get_parameters(ccs_distribution_t  distribution,
                                        ccs_numeric_t      *lower_ret,
                                        ccs_numeric_t      *upper_ret,
                                        ccs_scale_type_t   *scale_type_ret,
                                        ccs_numeric_t      *quantization_ret) {
	CCS_CHECK_DISTRIBUTION(distribution, CCS_UNIFORM);
	CCS_REFUTE(!lower_ret && !upper_ret && !scale_type_ret && !quantization_ret, CCS_INVALID_VALUE);
	_ccs_distribution_uniform_data_t * data = (_ccs_distribution_uniform_data_t *)distribution->data;

	if (lower_ret)
		*lower_ret = data->lower;
	if (upper_ret)
		*upper_ret = data->upper;
	if (scale_type_ret)
		*scale_type_ret = data->scale_type;
	if (quantization_ret)
		*quantization_ret = data->quantization;
	return CCS_SUCCESS;
}


