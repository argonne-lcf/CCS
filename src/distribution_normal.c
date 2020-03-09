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
_ccs_distribution_normal_get_num_parameters(_ccs_distribution_data_t *data,
                                            size_t                   *num_parameters_ret);

static ccs_error_t
_ccs_distribution_normal_get_parameters(_ccs_distribution_data_t *data,
                                        size_t                    num_parameters,
                                        ccs_datum_t              *parameters,
                                        size_t                   *num_parameters_ret);

static ccs_error_t
_ccs_distribution_normal_samples(_ccs_distribution_data_t *data,
                                 ccs_rng_t                 rng,
                                 size_t                    num_values,
                                 ccs_value_t              *values);

_ccs_distribution_ops_t _ccs_distribution_normal_ops = {
	{ &_ccs_distribution_del },
	&_ccs_distribution_normal_get_num_parameters,
	&_ccs_distribution_normal_get_parameters,
	&_ccs_distribution_normal_samples
};

static ccs_error_t
_ccs_distribution_normal_get_num_parameters(_ccs_distribution_data_t *data,
                                            size_t                   *num_parameters_ret) {
	*num_parameters_ret = 2;
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_distribution_normal_get_parameters(_ccs_distribution_data_t *data,
                                        size_t                    num_parameters,
                                        ccs_datum_t              *parameters,
                                        size_t                   *num_parameters_ret) {
	if (num_parameters > 0 && num_parameters < 2)
		return -CCS_INVALID_VALUE;
	_ccs_distribution_normal_data_t *d = (_ccs_distribution_normal_data_t *)data;

	if (num_parameters_ret)
		*num_parameters_ret = 2;

	if (parameters) {
		parameters[0].type = CCS_FLOAT;
		parameters[0].value.f = d->mu;
		parameters[1].type = CCS_FLOAT;
		parameters[1].value.f = d->sigma;
	}
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_distribution_normal_samples(_ccs_distribution_data_t *data,
                                  ccs_rng_t                 rng,
                                  size_t                    num_values,
                                  ccs_value_t              *values) {
	_ccs_distribution_normal_data_t *d = (_ccs_distribution_normal_data_t *)data;
	size_t i;
	const ccs_data_type_t  data_type      = d->common_data.data_type;
	const ccs_scale_type_t scale_type     = d->common_data.scale_type;
	const ccs_value_t      quantization   = d->common_data.quantization;
	const ccs_float_t      mu             = d->mu;
	const ccs_float_t      sigma          = d->sigma;
	const int              quantize       = d->quantize;
	gsl_rng *grng;
	ccs_error_t err = ccs_rng_get_gsl_rng(rng, &grng);
	if (err)
		return err;
	if (data_type == CCS_FLOAT)
		if (scale_type == CCS_LOGARITHMIC && quantize) {
			double lq = log(quantization.f*0.5);
			if (mu - lq >= 0.0)
				//at least 50% chance to get a valid value
				for (i = 0; i < num_values; i++)
					do {
						values[i].f = gsl_ran_gaussian(grng, sigma) + mu;
					} while (values[i].f < lq);
			else
				//use tail distribution
				for (i = 0; i < num_values; i++)
					values[i].f = gsl_ran_gaussian_tail(grng, lq - mu, sigma) + mu;
		}
		else
			for (i = 0; i < num_values; i++)
				values[i].f = gsl_ran_gaussian(grng, sigma) + mu;
	else {
		if (scale_type == CCS_LOGARITHMIC) {
			double lq;
			if (quantize)
				lq = log(quantization.i*0.5);
			else
				lq = log(0.5);
			if (mu - lq >= 0.0)
				for (i = 0; i < num_values; i++)
					do {
						values[i].f = gsl_ran_gaussian(grng, sigma) + mu;
					} while (values[i].f < lq);
			else
				for (i = 0; i < num_values; i++)
					values[i].f = gsl_ran_gaussian_tail(grng, lq - mu, sigma) + mu;
		}
		else
			for (i = 0; i < num_values; i++)
				values[i].f = gsl_ran_gaussian(grng, sigma) + mu;
	}

	if (scale_type == CCS_LOGARITHMIC)
		for (i = 0; i < num_values; i++)
			values[i].f = exp(values[i].f);
	if (data_type == CCS_FLOAT) {
		if (quantize) {
			ccs_float_t rquantization = 1.0 / quantization.f;
			for (i = 0; i < num_values; i++)
				values[i].f = round(values[i].f * rquantization) * quantization.f;
		}
	} else {
		if (quantize) {
			ccs_float_t rquantization = 1.0 / quantization.i;
			for (i = 0; i < num_values; i++)
				values[i].i = (ccs_int_t)round(values[i].f * rquantization) * quantization.i;
		} else
			for (i = 0; i < num_values; i++)
				values[i].i = round(values[i].f);
	}
	return CCS_SUCCESS;
}

extern ccs_error_t
_ccs_create_normal_distribution(ccs_data_type_t     data_type,
                                ccs_float_t         mu,
                                ccs_float_t         sigma,
                                ccs_scale_type_t    scale_type,
                                ccs_value_t         quantization,
                                ccs_distribution_t *distribution_ret) {
	if (!distribution_ret)
		return -CCS_INVALID_VALUE;
	if (data_type != CCS_FLOAT && data_type != CCS_INTEGER)
		return -CCS_INVALID_TYPE;
	if (scale_type != CCS_LINEAR && scale_type != CCS_LOGARITHMIC)
		return -CCS_INVALID_SCALE;
	if (data_type == CCS_INTEGER && quantization.i < 0 )
		return -CCS_INVALID_VALUE;
	if (data_type == CCS_FLOAT && quantization.f < 0.0 )
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
	if (data_type == CCS_FLOAT) {
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
                                       ccs_datum_t        *mu,
                                       ccs_datum_t        *sigma) {
	if (!distribution || distribution->obj.type != CCS_DISTRIBUTION)
		return -CCS_INVALID_OBJECT;
	if (!distribution->data || ((_ccs_distribution_common_data_t*)distribution->data)->type != CCS_NORMAL)
		return -CCS_INVALID_OBJECT;
	if (!mu && !sigma)
		return -CCS_INVALID_VALUE;
	_ccs_distribution_normal_data_t * data = (_ccs_distribution_normal_data_t *)distribution->data;

	if (mu) {
		mu->value.f = data->mu;
		mu->type = CCS_FLOAT;
	}
	if (sigma) {
		sigma->value.f = data->sigma;
		sigma->type = CCS_FLOAT;
	}
	return CCS_SUCCESS;
}


