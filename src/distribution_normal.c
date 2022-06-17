#include <math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include "cconfigspace_internal.h"
#include "distribution_internal.h"

struct _ccs_distribution_normal_data_s {
	_ccs_distribution_common_data_t common_data;
	ccs_float_t                     mu;
	ccs_float_t                     sigma;
	ccs_scale_type_t                scale_type;
	ccs_numeric_t                   quantization;
	int                             quantize;
};
typedef struct _ccs_distribution_normal_data_s _ccs_distribution_normal_data_t;

static ccs_result_t
_ccs_distribution_normal_del(ccs_object_t o) {
	(void)o;
	return CCS_SUCCESS;
}

static inline size_t
_ccs_serialize_bin_size_ccs_distribution_normal_data(
		_ccs_distribution_normal_data_t *data) {
	return _ccs_serialize_bin_size_ccs_distribution_common_data(&data->common_data) +
	       _ccs_serialize_bin_size_ccs_numeric_type(data->common_data.data_types[0]) +
	       _ccs_serialize_bin_size_ccs_scale_type(data->scale_type) +
	       _ccs_serialize_bin_size_ccs_float(data->mu) +
	       _ccs_serialize_bin_size_ccs_float(data->sigma) +
	       (data->common_data.data_types[0] == CCS_NUM_FLOAT ?
		_ccs_serialize_bin_size_ccs_float(data->quantization.f) :
		_ccs_serialize_bin_size_ccs_int(data->quantization.i));
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_distribution_normal_data(
		_ccs_distribution_normal_data_t  *data,
		size_t                            *buffer_size,
		char                             **buffer) {
	CCS_VALIDATE(_ccs_serialize_bin_ccs_distribution_common_data(
		&data->common_data, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_numeric_type(
		data->common_data.data_types[0], buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_scale_type(
		data->scale_type, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_float(
		data->mu, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_float(
		data->sigma, buffer_size, buffer));
	if (data->common_data.data_types[0] == CCS_NUM_FLOAT) {
		CCS_VALIDATE(_ccs_serialize_bin_ccs_float(
			data->quantization.f, buffer_size, buffer));
	} else {
		CCS_VALIDATE(_ccs_serialize_bin_ccs_int(
			data->quantization.i, buffer_size, buffer));
	}
	return CCS_SUCCESS;
}

static inline size_t
_ccs_serialize_bin_size_ccs_distribution_normal(
		ccs_distribution_t distribution) {
	_ccs_distribution_normal_data_t *data =
		(_ccs_distribution_normal_data_t *)(distribution->data);
	return	_ccs_serialize_bin_size_ccs_object_internal(
			(_ccs_object_internal_t *)distribution) +
	        _ccs_serialize_bin_size_ccs_distribution_normal_data(data);
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_distribution_normal(
		ccs_distribution_t   distribution,
		size_t              *buffer_size,
		char               **buffer) {
	_ccs_distribution_normal_data_t *data =
		(_ccs_distribution_normal_data_t *)(distribution->data);
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)distribution, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_distribution_normal_data(
		data, buffer_size, buffer));
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_distribution_normal_serialize_size(
		ccs_object_t                     object,
		ccs_serialize_format_t           format,
		size_t                          *cum_size,
		_ccs_object_serialize_options_t *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		*cum_size += _ccs_serialize_bin_size_ccs_distribution_normal(
			(ccs_distribution_t)object);
		break;
	default:
		CCS_RAISE(CCS_INVALID_VALUE, "Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data_size(
		object, format, cum_size, opts));
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_distribution_normal_serialize(
		ccs_object_t                      object,
		ccs_serialize_format_t            format,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_distribution_normal(
		    (ccs_distribution_t)object, buffer_size, buffer));
		break;
	default:
		CCS_RAISE(CCS_INVALID_VALUE, "Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data(
		object, format, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_distribution_normal_get_bounds(_ccs_distribution_data_t *data,
                                    ccs_interval_t           *interval_ret);

static ccs_result_t
_ccs_distribution_normal_samples(_ccs_distribution_data_t *data,
                                 ccs_rng_t                 rng,
                                 size_t                    num_values,
                                 ccs_numeric_t            *values);

static ccs_result_t
_ccs_distribution_normal_strided_samples(_ccs_distribution_data_t *data,
                                         ccs_rng_t                 rng,
                                         size_t                    num_values,
                                         size_t                    stride,
                                         ccs_numeric_t            *values);

static ccs_result_t
_ccs_distribution_normal_soa_samples(_ccs_distribution_data_t  *data,
                                     ccs_rng_t                  rng,
                                     size_t                     num_values,
                                     ccs_numeric_t            **values);

static _ccs_distribution_ops_t _ccs_distribution_normal_ops = {
	{ &_ccs_distribution_normal_del,
	  &_ccs_distribution_normal_serialize_size,
	  &_ccs_distribution_normal_serialize },
	&_ccs_distribution_normal_samples,
	&_ccs_distribution_normal_get_bounds,
	&_ccs_distribution_normal_strided_samples,
	&_ccs_distribution_normal_soa_samples
};

static ccs_result_t
_ccs_distribution_normal_get_bounds(_ccs_distribution_data_t *data,
                                    ccs_interval_t           *interval_ret) {
	_ccs_distribution_normal_data_t *d = (_ccs_distribution_normal_data_t *)data;
	const ccs_numeric_type_t  data_type   = d->common_data.data_types[0];
	const ccs_scale_type_t scale_type     = d->scale_type;
	const ccs_numeric_t    quantization   = d->quantization;
	const int              quantize       = d->quantize;
	ccs_numeric_t          l;
	ccs_bool_t             li;
	ccs_numeric_t          u;
	ccs_bool_t             ui;

	if (scale_type == CCS_LOGARITHMIC) {
		if (data_type == CCS_NUM_FLOAT) {
			if (quantize) {
				l.f = quantization.f;
				li = CCS_TRUE;
			} else {
				l.f = 0.0;
				li = CCS_FALSE;
			}
			u.f = CCS_INFINITY;
			ui = CCS_FALSE;
		} else {
			if (quantize) {
				l.i = quantization.i;
				u.i = (CCS_INT_MAX/quantization.i)*quantization.i;
			} else {
				l.i = 1;
				u.i = CCS_INT_MAX;
			}
			li = CCS_TRUE;
			ui = CCS_TRUE;
		}
	} else {
		if (data_type == CCS_NUM_FLOAT) {
			l.f = -CCS_INFINITY;
			li = CCS_FALSE;
			u.f = CCS_INFINITY;
			ui = CCS_FALSE;
		} else {
			if (quantize) {
				l.i = (CCS_INT_MIN/quantization.i)*quantization.i;
				u.i = (CCS_INT_MAX/quantization.i)*quantization.i;
			} else {
				l.i = CCS_INT_MIN;
				u.i = CCS_INT_MAX;
			}
			li = CCS_TRUE;
			ui = CCS_TRUE;
		}
	}
	interval_ret->type = data_type;
	interval_ret->lower = l;
	interval_ret->upper = u;
	interval_ret->lower_included = li;
	interval_ret->upper_included = ui;
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_distribution_normal_samples_float(gsl_rng                *grng,
                                       const ccs_scale_type_t  scale_type,
                                       const ccs_float_t       quantization,
                                       const ccs_float_t       mu,
                                       const ccs_float_t       sigma,
                                       const int               quantize,
                                       size_t                  num_values,
                                       ccs_float_t            *values) {
	size_t i;
	if (scale_type == CCS_LOGARITHMIC && quantize) {
		ccs_float_t lq = log(quantization*0.5);
		if (mu - lq >= 0.0)
			//at least 50% chance to get a valid value
			for (i = 0; i < num_values; i++)
				do {
					values[i] = gsl_ran_gaussian(grng, sigma) + mu;
				} while (values[i] < lq);
		else
			//use tail distribution
			for (i = 0; i < num_values; i++)
				values[i] = gsl_ran_gaussian_tail(grng, lq - mu, sigma) + mu;
	} else
		for (i = 0; i < num_values; i++)
			values[i] = gsl_ran_gaussian(grng, sigma) + mu;
	if (scale_type == CCS_LOGARITHMIC)
		for (i = 0; i < num_values; i++)
			values[i] = exp(values[i]);
	if (quantize) {
			ccs_float_t rquantization = 1.0 / quantization;
			for (i = 0; i < num_values; i++)
				values[i] = round(values[i] * rquantization) * quantization;
	}
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_distribution_normal_samples_int(gsl_rng                *grng,
                                     const ccs_scale_type_t  scale_type,
                                     const ccs_int_t         quantization,
                                     const ccs_float_t       mu,
                                     const ccs_float_t       sigma,
                                     const int               quantize,
                                     size_t                  num_values,
                                     ccs_numeric_t          *values) {
	size_t i;
	ccs_float_t q;
	if (quantize)
		q = quantization*0.5;
	else
		q = 0.5;
	if (scale_type == CCS_LOGARITHMIC) {
		ccs_float_t lq = log(q);
		if (mu - lq >= 0.0)
			for (i = 0; i < num_values; i++)
				do {
					do {
						values[i].f = gsl_ran_gaussian(grng, sigma) + mu;
					} while (values[i].f < lq);
					values[i].f = exp(values[i].f);
				} while (CCS_UNLIKELY(values[i].f - q > (ccs_float_t)CCS_INT_MAX));
		else
			for (i = 0; i < num_values; i++)
				do {
					values[i].f = gsl_ran_gaussian_tail(grng, lq - mu, sigma) + mu;
					values[i].f = exp(values[i].f);
				} while (CCS_UNLIKELY(values[i].f - q > (ccs_float_t)CCS_INT_MAX));
	}
	else
		for (i = 0; i < num_values; i++)
			do {
				values[i].f = gsl_ran_gaussian(grng, sigma) + mu;
			} while (CCS_UNLIKELY(values[i].f - q > (ccs_float_t)CCS_INT_MAX || values[i].f + q < (ccs_float_t)CCS_INT_MIN));
	if (quantize) {
		ccs_float_t rquantization = 1.0 / quantization;
		for (i = 0; i < num_values; i++)
			values[i].i = (ccs_int_t)round(values[i].f * rquantization) * quantization;
	} else
		for (i = 0; i < num_values; i++)
			values[i].i = round(values[i].f);
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_distribution_normal_samples(_ccs_distribution_data_t *data,
                                  ccs_rng_t                 rng,
                                  size_t                    num_values,
                                  ccs_numeric_t            *values) {
	_ccs_distribution_normal_data_t *d = (_ccs_distribution_normal_data_t *)data;
	const ccs_numeric_type_t  data_type   = d->common_data.data_types[0];
	const ccs_scale_type_t scale_type     = d->scale_type;
	const ccs_numeric_t    quantization   = d->quantization;
	const ccs_float_t      mu             = d->mu;
	const ccs_float_t      sigma          = d->sigma;
	const int              quantize       = d->quantize;
	gsl_rng *grng;
	CCS_VALIDATE(ccs_rng_get_gsl_rng(rng, &grng));
	if (data_type == CCS_NUM_FLOAT)
		CCS_VALIDATE(_ccs_distribution_normal_samples_float(
			grng, scale_type, quantization.f, mu, sigma, quantize,
			num_values, (ccs_float_t*) values));
	else
		CCS_VALIDATE(_ccs_distribution_normal_samples_int(
			grng, scale_type, quantization.i, mu, sigma, quantize,
			num_values, values));
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_distribution_normal_strided_samples_float(
		gsl_rng                *grng,
		const ccs_scale_type_t  scale_type,
		const ccs_float_t       quantization,
		const ccs_float_t       mu,
		const ccs_float_t       sigma,
		const int               quantize,
		size_t                  num_values,
		size_t                  stride,
		ccs_float_t            *values) {
	size_t i;
	if (scale_type == CCS_LOGARITHMIC && quantize) {
		ccs_float_t lq = log(quantization*0.5);
		if (mu - lq >= 0.0)
			//at least 50% chance to get a valid value
			for (i = 0; i < num_values; i++)
				do {
					values[i*stride] = gsl_ran_gaussian(grng, sigma) + mu;
				} while (values[i*stride] < lq);
		else
			//use tail distribution
			for (i = 0; i < num_values; i++)
				values[i*stride] = gsl_ran_gaussian_tail(grng, lq - mu, sigma) + mu;
	} else
		for (i = 0; i < num_values; i++)
			values[i*stride] = gsl_ran_gaussian(grng, sigma) + mu;
	if (scale_type == CCS_LOGARITHMIC)
		for (i = 0; i < num_values; i++)
			values[i*stride] = exp(values[i*stride]);
	if (quantize) {
			ccs_float_t rquantization = 1.0 / quantization;
			for (i = 0; i < num_values; i++)
				values[i*stride] = round(values[i*stride] * rquantization) * quantization;
	}
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_distribution_normal_strided_samples_int(
		gsl_rng                *grng,
		const ccs_scale_type_t  scale_type,
		const ccs_int_t         quantization,
		const ccs_float_t       mu,
		const ccs_float_t       sigma,
		const int               quantize,
		size_t                  num_values,
		size_t                  stride,
		ccs_numeric_t          *values) {
	size_t i;
	ccs_float_t q;
	if (quantize)
		q = quantization*0.5;
	else
		q = 0.5;
	if (scale_type == CCS_LOGARITHMIC) {
		ccs_float_t lq = log(q);
		if (mu - lq >= 0.0)
			for (i = 0; i < num_values; i++)
				do {
					do {
						values[i*stride].f = gsl_ran_gaussian(grng, sigma) + mu;
					} while (values[i*stride].f < lq);
					values[i*stride].f = exp(values[i*stride].f);
				} while (CCS_UNLIKELY(values[i*stride].f - q > (ccs_float_t)CCS_INT_MAX));
		else
			for (i = 0; i < num_values; i++)
				do {
					values[i*stride].f = gsl_ran_gaussian_tail(grng, lq - mu, sigma) + mu;
					values[i*stride].f = exp(values[i*stride].f);
				} while (CCS_UNLIKELY(values[i*stride].f - q > (ccs_float_t)CCS_INT_MAX));
	}
	else
		for (i = 0; i < num_values; i++)
			do {
				values[i*stride].f = gsl_ran_gaussian(grng, sigma) + mu;
			} while (CCS_UNLIKELY(values[i*stride].f - q > (ccs_float_t)CCS_INT_MAX || values[i*stride].f + q < (ccs_float_t)CCS_INT_MIN));
	if (quantize) {
		ccs_float_t rquantization = 1.0 / quantization;
		for (i = 0; i < num_values; i++)
			values[i*stride].i = (ccs_int_t)round(values[i*stride].f * rquantization) * quantization;
	} else
		for (i = 0; i < num_values; i++)
			values[i*stride].i = round(values[i*stride].f);
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_distribution_normal_strided_samples(_ccs_distribution_data_t *data,
                                         ccs_rng_t                 rng,
                                         size_t                    num_values,
                                         size_t                    stride,
                                         ccs_numeric_t            *values) {
	_ccs_distribution_normal_data_t *d = (_ccs_distribution_normal_data_t *)data;
	const ccs_numeric_type_t  data_type   = d->common_data.data_types[0];
	const ccs_scale_type_t scale_type     = d->scale_type;
	const ccs_numeric_t    quantization   = d->quantization;
	const ccs_float_t      mu             = d->mu;
	const ccs_float_t      sigma          = d->sigma;
	const int              quantize       = d->quantize;
	gsl_rng *grng;
	CCS_VALIDATE(ccs_rng_get_gsl_rng(rng, &grng));
	if (data_type == CCS_NUM_FLOAT)
		CCS_VALIDATE(_ccs_distribution_normal_strided_samples_float(
			grng, scale_type, quantization.f, mu, sigma, quantize,
			num_values, stride, (ccs_float_t*) values));
	else
		CCS_VALIDATE(_ccs_distribution_normal_strided_samples_int(
			grng, scale_type, quantization.i, mu, sigma, quantize,
			num_values, stride, values));
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_distribution_normal_soa_samples(_ccs_distribution_data_t  *data,
                                     ccs_rng_t                  rng,
                                     size_t                     num_values,
                                     ccs_numeric_t            **values) {
	if (*values)
		CCS_VALIDATE(_ccs_distribution_normal_samples(data, rng, num_values, *values));
	return CCS_SUCCESS;
}

extern ccs_result_t
ccs_create_normal_distribution(ccs_numeric_type_t  data_type,
                               ccs_float_t         mu,
                               ccs_float_t         sigma,
                               ccs_scale_type_t    scale_type,
                               ccs_numeric_t       quantization,
                               ccs_distribution_t *distribution_ret) {
	CCS_CHECK_PTR(distribution_ret);
	CCS_REFUTE(data_type != CCS_NUM_FLOAT && data_type != CCS_NUM_INTEGER, CCS_INVALID_TYPE);
	CCS_REFUTE(scale_type != CCS_LINEAR && scale_type != CCS_LOGARITHMIC, CCS_INVALID_SCALE);
	CCS_REFUTE(data_type == CCS_NUM_INTEGER && quantization.i < 0, CCS_INVALID_VALUE);
	CCS_REFUTE(data_type == CCS_NUM_FLOAT && quantization.f < 0.0, CCS_INVALID_VALUE);
	uintptr_t mem = (uintptr_t)calloc(1, sizeof(struct _ccs_distribution_s) + sizeof(_ccs_distribution_normal_data_t) + sizeof(ccs_numeric_type_t));
	CCS_REFUTE(!mem, CCS_OUT_OF_MEMORY);
	ccs_distribution_t distrib = (ccs_distribution_t)mem;
	_ccs_object_init(&(distrib->obj), CCS_DISTRIBUTION, (_ccs_object_ops_t *)&_ccs_distribution_normal_ops);
        _ccs_distribution_normal_data_t * distrib_data = (_ccs_distribution_normal_data_t *)(mem + sizeof(struct _ccs_distribution_s));
	distrib_data->common_data.data_types    = (ccs_numeric_type_t *)(mem + sizeof(struct _ccs_distribution_s) + sizeof(_ccs_distribution_normal_data_t));
	distrib_data->common_data.type          = CCS_NORMAL;
	distrib_data->common_data.dimension     = 1;
	distrib_data->common_data.data_types[0] = data_type;
	distrib_data->scale_type                = scale_type;
	distrib_data->quantization              = quantization;
	distrib_data->mu                        = mu;
	distrib_data->sigma                     = sigma;
	if (data_type == CCS_NUM_FLOAT) {
		if (quantization.f != 0.0)
			distrib_data->quantize = 1;
	} else {
		if (quantization.i != 0)
			distrib_data->quantize = 1;
	}
	distrib->data = (_ccs_distribution_data_t *)distrib_data;
	*distribution_ret = distrib;
	return CCS_SUCCESS;
}

extern ccs_result_t
ccs_normal_distribution_get_parameters(ccs_distribution_t  distribution,
                                       ccs_float_t        *mu_ret,
                                       ccs_float_t        *sigma_ret,
                                       ccs_scale_type_t   *scale_type_ret,
                                       ccs_numeric_t      *quantization_ret) {
	CCS_CHECK_DISTRIBUTION(distribution, CCS_NORMAL);
	CCS_REFUTE(!mu_ret && !sigma_ret && !scale_type_ret && !quantization_ret, CCS_INVALID_VALUE);
	_ccs_distribution_normal_data_t * data = (_ccs_distribution_normal_data_t *)distribution->data;

	if (mu_ret)
		*mu_ret = data->mu;
	if (sigma_ret)
		*sigma_ret = data->sigma;
	if (scale_type_ret)
		*scale_type_ret = data->scale_type;
	if (quantization_ret)
		*quantization_ret = data->quantization;
	return CCS_SUCCESS;
}

