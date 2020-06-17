#include "cconfigspace_internal.h"
#include "hyperparameter_internal.h"
#include <string.h>

struct _ccs_hyperparameter_numerical_data_s {
	_ccs_hyperparameter_common_data_t common_data;
	ccs_numeric_t                     quantization;
};
typedef struct _ccs_hyperparameter_numerical_data_s _ccs_hyperparameter_numerical_data_t;

static ccs_result_t
_ccs_hyperparameter_numerical_del(ccs_object_t o) {
	(void)o;
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_hyperparameter_numerical_check_values(_ccs_hyperparameter_data_t *data,
                                           size_t                num_values,
                                           const ccs_datum_t    *values,
                                           ccs_bool_t           *results) {
	_ccs_hyperparameter_numerical_data_t *d =
	    (_ccs_hyperparameter_numerical_data_t *)data;
	ccs_interval_t *interval = &(d->common_data.interval);
	ccs_numeric_type_t type = d->common_data.interval.type;
	if (type == CCS_NUM_FLOAT) {
		for(size_t i = 0; i < num_values; i++)
			if (values[i].type != CCS_FLOAT)
				results[i] = CCS_FALSE;
			else
				results[i] = ccs_interval_include(interval, CCSF(values[i].value.f));
	}
	else {
		for(size_t i = 0; i < num_values; i++)
			if (values[i].type != CCS_INTEGER)
				results[i] = CCS_FALSE;
			else
				results[i] = ccs_interval_include(interval, CCSI(values[i].value.i));
	}
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_hyperparameter_numerical_samples(_ccs_hyperparameter_data_t *data,
                                      ccs_distribution_t          distribution,
                                      ccs_rng_t                   rng,
                                      size_t                      num_values,
                                      ccs_datum_t                *values) {
	_ccs_hyperparameter_numerical_data_t *d =
	    (_ccs_hyperparameter_numerical_data_t *)data;
	ccs_numeric_type_t type = d->common_data.interval.type;
	ccs_interval_t *interval = &(d->common_data.interval);
	ccs_result_t err;
	ccs_numeric_t *vs = (ccs_numeric_t *)values + num_values;
        ccs_bool_t oversampling;

	err = ccs_distribution_check_oversampling(distribution,
	                                          interval,
	                                          &oversampling);
	if (err)
		return err;
	err = ccs_distribution_samples(distribution,
	                               rng, num_values, vs);
	if (err)
		return err;
	if (!oversampling) {
		if (type == CCS_NUM_FLOAT) {
			for(size_t i = 0; i < num_values; i++)
				values[i].value.f = vs[i].f;
		} else {
			for(size_t i = 0; i < num_values; i++)
				values[i].value.i = vs[i].i;
		}
	} else {
		size_t found = 0;
		if (type == CCS_NUM_FLOAT) {
			for(size_t i = 0; i < num_values; i++)
				if (ccs_interval_include(interval, vs[i]))
					values[found++].value.f = vs[i].f;
		} else {
			for(size_t i = 0; i < num_values; i++)
				if (ccs_interval_include(interval, vs[i]))
					values[found++].value.i = vs[i].i;
		}
		vs = NULL;
		size_t coeff = 2;
		while (found < num_values) {
			size_t buff_sz = (num_values - found)*coeff;
			vs = (ccs_numeric_t *)malloc(sizeof(ccs_numeric_t)*buff_sz);
			if (!vs)
				return -CCS_OUT_OF_MEMORY;
			err = ccs_distribution_samples(distribution, rng,
			                               buff_sz, vs);
			if (type == CCS_NUM_FLOAT) {
				for(size_t i = 0; i < buff_sz && found < num_values; i++)
					if (ccs_interval_include(interval, vs[i]))
						values[found++].value.f = vs[i].f;
			} else {
				for(size_t i = 0; i < buff_sz && found < num_values; i++)
					if (ccs_interval_include(interval, vs[i]))
						values[found++].value.i = vs[i].i;
			}
			coeff <<= 1;
			free(vs);
			if (coeff > 32)
				return -CCS_SAMPLING_UNSUCCESSFUL;
		}
	}
	for (size_t i = 0; i < num_values; i++)
		values[i].type = (ccs_data_type_t)type;
	return CCS_SUCCESS;
}

ccs_result_t
_ccs_hyperparameter_numerical_get_default_distribution(
		_ccs_hyperparameter_data_t *data,
		ccs_distribution_t         *distribution) {
	_ccs_hyperparameter_numerical_data_t *d = (_ccs_hyperparameter_numerical_data_t *)data;
	ccs_interval_t *interval = &(d->common_data.interval);
	return ccs_create_uniform_distribution(interval->type,
	                                       interval->lower, interval->upper,
	                                       CCS_LINEAR, d->quantization,
	                                       distribution);
}

static _ccs_hyperparameter_ops_t _ccs_hyperparameter_numerical_ops = {
	{ &_ccs_hyperparameter_numerical_del },
	&_ccs_hyperparameter_numerical_check_values,
	&_ccs_hyperparameter_numerical_samples,
	&_ccs_hyperparameter_numerical_get_default_distribution
};

ccs_result_t
ccs_create_numerical_hyperparameter(const char           *name,
                                    ccs_numeric_type_t    data_type,
                                    ccs_numeric_t         lower,
                                    ccs_numeric_t         upper,
                                    ccs_numeric_t         quantization,
                                    ccs_numeric_t         default_value,
                                    void                 *user_data,
                                    ccs_hyperparameter_t *hyperparameter_ret) {
	if (!hyperparameter_ret || !name)
		return -CCS_INVALID_VALUE;
	if (data_type != CCS_NUM_FLOAT && data_type != CCS_NUM_INTEGER)
		return -CCS_INVALID_TYPE;
	if (data_type == CCS_NUM_INTEGER && (
		lower.i >= upper.i ||
		quantization.i < 0 ||
		quantization.i > upper.i - lower.i ||
		default_value.i < lower.i ||
		default_value.i >= upper.i ) )
		return -CCS_INVALID_VALUE;
	if (data_type == CCS_NUM_FLOAT && (
		lower.f >= upper.f ||
		quantization.f < 0.0 ||
		quantization.f > upper.f - lower.f ||
		default_value.f < lower.f ||
		default_value.f >= upper.f ) )
		return -CCS_INVALID_VALUE;
	uintptr_t mem = (uintptr_t)calloc(1, sizeof(struct _ccs_hyperparameter_s) + sizeof(_ccs_hyperparameter_numerical_data_t) + strlen(name) + 1);
	if (!mem)
		return -CCS_OUT_OF_MEMORY;

	ccs_interval_t interval;
	interval.type = data_type;
	interval.lower = lower;
	interval.upper = upper;
	interval.lower_included = CCS_TRUE;
	interval.upper_included = CCS_FALSE;

	ccs_hyperparameter_t hyperparam = (ccs_hyperparameter_t)mem;
	_ccs_object_init(&(hyperparam->obj), CCS_HYPERPARAMETER, (_ccs_object_ops_t *)&_ccs_hyperparameter_numerical_ops);
	_ccs_hyperparameter_numerical_data_t *hyperparam_data = (_ccs_hyperparameter_numerical_data_t *)(mem + sizeof(struct _ccs_hyperparameter_s));
	hyperparam_data->common_data.type = CCS_NUMERICAL;
	hyperparam_data->common_data.name = (char *)(mem + sizeof(struct _ccs_hyperparameter_s) + sizeof(_ccs_hyperparameter_numerical_data_t));
	strcpy((char *)hyperparam_data->common_data.name, name);
	hyperparam_data->common_data.user_data = user_data;
	if (data_type == CCS_NUM_FLOAT) {
		hyperparam_data->common_data.default_value.type = CCS_FLOAT;
		hyperparam_data->common_data.default_value.value.f = default_value.f;
	} else {
		hyperparam_data->common_data.default_value.type = CCS_INTEGER;
		hyperparam_data->common_data.default_value.value.i = default_value.i;
	}
	hyperparam_data->common_data.interval = interval;
	hyperparam_data->quantization = quantization;
	hyperparam->data = (_ccs_hyperparameter_data_t *)hyperparam_data;
	*hyperparameter_ret = hyperparam;

	return CCS_SUCCESS;
}

ccs_result_t
ccs_numerical_hyperparameter_get_parameters(ccs_hyperparameter_t  hyperparameter,
                                            ccs_numeric_type_t   *data_type_ret,
                                            ccs_numeric_t        *lower_ret,
                                            ccs_numeric_t        *upper_ret,
                                            ccs_numeric_t        *quantization_ret) {
	if (!hyperparameter || !hyperparameter->data)
		return -CCS_INVALID_OBJECT;
	if (!data_type_ret && !lower_ret && !upper_ret && !quantization_ret)
		return -CCS_INVALID_VALUE;
	_ccs_hyperparameter_numerical_data_t *d =
		(_ccs_hyperparameter_numerical_data_t *)hyperparameter->data;
        if (data_type_ret)
		*data_type_ret = d->common_data.interval.type;
        if (lower_ret)
		*lower_ret = d->common_data.interval.lower;
        if (upper_ret)
		*upper_ret = d->common_data.interval.upper;
	if (quantization_ret)
		*quantization_ret = d->quantization;
	return CCS_SUCCESS;
}


