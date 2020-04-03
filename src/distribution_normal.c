#include <math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include "cconfigspace_internal.h"
#include "distribution_internal.h"

struct _ccs_distribution_normal_data_s {
	_ccs_distribution_common_data_t common_data;
	ccs_float_t                     mu;
	ccs_float_t                     sigma;
	int                             quantize;
};
typedef struct _ccs_distribution_normal_data_s _ccs_distribution_normal_data_t;

static ccs_error_t
_ccs_distribution_del(ccs_object_t o) {
	(void)o;
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_distribution_normal_get_bounds(_ccs_distribution_data_t *data,
                                    ccs_interval_t           *interval_ret);

static ccs_error_t
_ccs_distribution_normal_samples(_ccs_distribution_data_t *data,
                                 ccs_rng_t                 rng,
                                 size_t                    num_values,
                                 ccs_numeric_t            *values);

_ccs_distribution_ops_t _ccs_distribution_normal_ops = {
	{ &_ccs_distribution_del },
	&_ccs_distribution_normal_samples,
	&_ccs_distribution_normal_get_bounds
};

static ccs_error_t
_ccs_distribution_normal_get_bounds(_ccs_distribution_data_t *data,
                                    ccs_interval_t           *interval_ret) {
	_ccs_distribution_normal_data_t *d = (_ccs_distribution_normal_data_t *)data;
	const ccs_numeric_type_t  data_type   = d->common_data.data_type;
	const ccs_scale_type_t scale_type     = d->common_data.scale_type;
	const ccs_numeric_t    quantization   = d->common_data.quantization;
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

static inline ccs_error_t
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

static inline ccs_error_t
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
				} while (unlikely(values[i].f - q > CCS_INT_MAX));
		else
			for (i = 0; i < num_values; i++)
				do {
					values[i].f = gsl_ran_gaussian_tail(grng, lq - mu, sigma) + mu;
					values[i].f = exp(values[i].f);
				} while (unlikely(values[i].f - q > CCS_INT_MAX));
	}
	else
		for (i = 0; i < num_values; i++)
			do {
				values[i].f = gsl_ran_gaussian(grng, sigma) + mu;
			} while (unlikely(values[i].f - q > CCS_INT_MAX || values[i].f + q < CCS_INT_MIN));
	if (quantize) {
		ccs_float_t rquantization = 1.0 / quantization;
		for (i = 0; i < num_values; i++)
			values[i].i = (ccs_int_t)round(values[i].f * rquantization) * quantization;
	} else
		for (i = 0; i < num_values; i++)
			values[i].i = round(values[i].f);
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_distribution_normal_samples(_ccs_distribution_data_t *data,
                                  ccs_rng_t                 rng,
                                  size_t                    num_values,
                                  ccs_numeric_t            *values) {
	_ccs_distribution_normal_data_t *d = (_ccs_distribution_normal_data_t *)data;
	const ccs_numeric_type_t  data_type   = d->common_data.data_type;
	const ccs_scale_type_t scale_type     = d->common_data.scale_type;
	const ccs_numeric_t    quantization   = d->common_data.quantization;
	const ccs_float_t      mu             = d->mu;
	const ccs_float_t      sigma          = d->sigma;
	const int              quantize       = d->quantize;
	gsl_rng *grng;
	ccs_error_t err = ccs_rng_get_gsl_rng(rng, &grng);
	if (err)
		return err;
	if (data_type == CCS_NUM_FLOAT)
		return _ccs_distribution_normal_samples_float(grng, scale_type,
                                                              quantization.f, mu,
                                                              sigma, quantize,
                                                              num_values,
                                                              (ccs_float_t*) values);
	else
		return _ccs_distribution_normal_samples_int(grng, scale_type,
                                                            quantization.i, mu,
                                                            sigma, quantize,
                                                            num_values, values);
}

extern ccs_error_t
_ccs_create_normal_distribution(ccs_numeric_type_t  data_type,
                                ccs_float_t         mu,
                                ccs_float_t         sigma,
                                ccs_scale_type_t    scale_type,
                                ccs_numeric_t       quantization,
                                ccs_distribution_t *distribution_ret) {
	if (!distribution_ret)
		return -CCS_INVALID_VALUE;
	if (data_type != CCS_NUM_FLOAT && data_type != CCS_NUM_INTEGER)
		return -CCS_INVALID_TYPE;
	if (scale_type != CCS_LINEAR && scale_type != CCS_LOGARITHMIC)
		return -CCS_INVALID_SCALE;
	if (data_type == CCS_NUM_INTEGER && quantization.i < 0 )
		return -CCS_INVALID_VALUE;
	if (data_type == CCS_NUM_FLOAT && quantization.f < 0.0 )
		return -CCS_INVALID_VALUE;
	uintptr_t mem = (uintptr_t)calloc(1, sizeof(struct _ccs_distribution_s) + sizeof(_ccs_distribution_normal_data_t));

	if (!mem)
		return -CCS_ENOMEM;
	ccs_distribution_t distrib = (ccs_distribution_t)mem;
	_ccs_object_init(&(distrib->obj), CCS_DISTRIBUTION, (_ccs_object_ops_t *)&_ccs_distribution_normal_ops);
        _ccs_distribution_normal_data_t * distrib_data = (_ccs_distribution_normal_data_t *)(mem + sizeof(struct _ccs_distribution_s));
	distrib_data->common_data.type         = CCS_NORMAL;
	distrib_data->common_data.data_type    = data_type;
	distrib_data->common_data.scale_type   = scale_type;
	distrib_data->common_data.quantization = quantization;
	distrib_data->mu                       = mu;
	distrib_data->sigma                    = sigma;
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

extern ccs_error_t
ccs_normal_distribution_get_parameters(ccs_distribution_t  distribution,
                                       ccs_float_t        *mu,
                                       ccs_float_t        *sigma) {
	if (!distribution || distribution->obj.type != CCS_DISTRIBUTION)
		return -CCS_INVALID_OBJECT;
	if (!distribution->data || ((_ccs_distribution_common_data_t*)distribution->data)->type != CCS_NORMAL)
		return -CCS_INVALID_OBJECT;
	if (!mu && !sigma)
		return -CCS_INVALID_VALUE;
	_ccs_distribution_normal_data_t * data = (_ccs_distribution_normal_data_t *)distribution->data;

	if (mu) {
		*mu = data->mu;
	}
	if (sigma) {
		*sigma = data->sigma;
	}
	return CCS_SUCCESS;
}


