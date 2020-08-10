#include <math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include "cconfigspace_internal.h"
#include "distribution_internal.h"

struct _ccs_distribution_uniform_data_s {
	_ccs_distribution_common_data_t common_data;
	ccs_numeric_t                   lower;
	ccs_numeric_t                   upper;
	ccs_numeric_type_t              internal_type;
	ccs_numeric_t                   internal_lower;
        ccs_numeric_t                   internal_upper;
	int                             quantize;
};
typedef struct _ccs_distribution_uniform_data_s _ccs_distribution_uniform_data_t;

static ccs_result_t
_ccs_distribution_del(ccs_object_t o) {
	(void)o;
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_distribution_uniform_get_bounds(_ccs_distribution_data_t *data,
                                     ccs_interval_t           *interval_ret);

static ccs_result_t
_ccs_distribution_uniform_samples(_ccs_distribution_data_t *data,
                                  ccs_rng_t                 rng,
                                  size_t                    num_values,
                                  ccs_numeric_t            *values);

static ccs_result_t
_ccs_distribution_uniform_strided_samples(_ccs_distribution_data_t *data,
                                          ccs_rng_t                 rng,
                                          size_t                    num_values,
                                          size_t                    stride,
                                          ccs_numeric_t            *values);

static _ccs_distribution_ops_t _ccs_distribution_uniform_ops = {
	{ &_ccs_distribution_del },
	&_ccs_distribution_uniform_samples,
	&_ccs_distribution_uniform_get_bounds,
	&_ccs_distribution_uniform_strided_samples
 };

static ccs_result_t
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
	interval_ret->type = d->common_data.data_type;
	interval_ret->lower = l;
	interval_ret->upper = u;
	interval_ret->lower_included = li;
	interval_ret->upper_included = ui;
	return CCS_SUCCESS;
}


static ccs_result_t
_ccs_distribution_uniform_strided_samples(_ccs_distribution_data_t *data,
                                          ccs_rng_t                 rng,
                                          size_t                    num_values,
                                          size_t                    stride,
                                          ccs_numeric_t            *values) {
	_ccs_distribution_uniform_data_t *d = (_ccs_distribution_uniform_data_t *)data;
	size_t i;
	const ccs_numeric_type_t  data_type      = d->common_data.data_type;
	const ccs_scale_type_t    scale_type     = d->common_data.scale_type;
	const ccs_numeric_t       quantization   = d->common_data.quantization;
	const ccs_numeric_t       lower          = d->lower;
	const ccs_numeric_t       internal_lower = d->internal_lower;
	const ccs_numeric_t       internal_upper = d->internal_upper;
	const int                 quantize       = d->quantize;
	gsl_rng *grng;
	ccs_result_t err = ccs_rng_get_gsl_rng(rng, &grng);
	if (err)
		return err;

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

static ccs_result_t
_ccs_distribution_uniform_samples(_ccs_distribution_data_t *data,
                                  ccs_rng_t                 rng,
                                  size_t                    num_values,
                                  ccs_numeric_t            *values) {
	_ccs_distribution_uniform_data_t *d = (_ccs_distribution_uniform_data_t *)data;
	size_t i;
	const ccs_numeric_type_t  data_type      = d->common_data.data_type;
	const ccs_scale_type_t    scale_type     = d->common_data.scale_type;
	const ccs_numeric_t       quantization   = d->common_data.quantization;
	const ccs_numeric_t       lower          = d->lower;
	const ccs_numeric_t       internal_lower = d->internal_lower;
	const ccs_numeric_t       internal_upper = d->internal_upper;
	const int                 quantize       = d->quantize;
	gsl_rng *grng;
	ccs_result_t err = ccs_rng_get_gsl_rng(rng, &grng);
	if (err)
		return err;

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

ccs_result_t
ccs_create_uniform_distribution(ccs_numeric_type_t  data_type,
                                ccs_numeric_t       lower,
                                ccs_numeric_t       upper,
                                ccs_scale_type_t    scale_type,
                                ccs_numeric_t       quantization,
                                ccs_distribution_t *distribution_ret) {
	CCS_CHECK_PTR(distribution_ret);
	if (data_type != CCS_NUM_FLOAT && data_type != CCS_NUM_INTEGER)
		return -CCS_INVALID_TYPE;
	if (scale_type != CCS_LINEAR && scale_type != CCS_LOGARITHMIC)
		return -CCS_INVALID_SCALE;
	if (data_type == CCS_NUM_INTEGER && (
		lower.i >= upper.i ||
		(scale_type == CCS_LOGARITHMIC && lower.i <= 0) ||
		quantization.i < 0 ||
		quantization.i > upper.i - lower.i ) )
		return -CCS_INVALID_VALUE;
	if (data_type == CCS_NUM_FLOAT && (
		lower.f >= upper.f ||
		(scale_type == CCS_LOGARITHMIC && lower.f <= 0.0) ||
		quantization.f < 0.0 ||
		quantization.f > upper.f - lower.f ) )
		return -CCS_INVALID_VALUE;
	uintptr_t mem = (uintptr_t)calloc(1, sizeof(struct _ccs_distribution_s) + sizeof(_ccs_distribution_uniform_data_t));

	if (!mem)
		return -CCS_OUT_OF_MEMORY;
	ccs_distribution_t distrib = (ccs_distribution_t)mem;
	_ccs_object_init(&(distrib->obj), CCS_DISTRIBUTION, (_ccs_object_ops_t *)&_ccs_distribution_uniform_ops);
        _ccs_distribution_uniform_data_t * distrib_data = (_ccs_distribution_uniform_data_t *)(mem + sizeof(struct _ccs_distribution_s));
	distrib_data->common_data.type         = CCS_UNIFORM;
	distrib_data->common_data.data_type    = data_type;
	distrib_data->common_data.scale_type   = scale_type;
	distrib_data->common_data.quantization = quantization;
	distrib_data->lower                    = lower;
	distrib_data->upper                    = upper;

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

ccs_result_t
ccs_uniform_distribution_get_parameters(ccs_distribution_t  distribution,
                                        ccs_numeric_t      *lower_ret,
                                        ccs_numeric_t      *upper_ret) {
	CCS_CHECK_OBJ(distribution, CCS_DISTRIBUTION);
	if (((_ccs_distribution_common_data_t*)distribution->data)->type != CCS_UNIFORM)
		return -CCS_INVALID_OBJECT;
	if (!lower_ret && !upper_ret)
		return -CCS_INVALID_VALUE;
	_ccs_distribution_uniform_data_t * data = (_ccs_distribution_uniform_data_t *)distribution->data;

	if (lower_ret) {
		*lower_ret = data->lower;
	}
	if (upper_ret) {
		*upper_ret = data->upper;
	}
	return CCS_SUCCESS;
}


