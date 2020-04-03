#include "cconfigspace_internal.h"
#include "hyperparameter_internal.h"

struct _ccs_hyperparameter_numerical_data_s {
	_ccs_hyperparameter_common_data_t common_data;
	ccs_interval_t                    interval;
	ccs_numeric_t                     quantization;
};
typedef struct _ccs_hyperparameter_numerical_data_s _ccs_hyperparameter_numerical_data_t;

static ccs_error_t
_ccs_hyperparameter_numerical_del(ccs_object_t o) {
	ccs_hyperparameter_t d = o.hyperparameter;
	_ccs_hyperparameter_numerical_data_t *data = (_ccs_hyperparameter_numerical_data_t *)(d->data);
	return ccs_release_object(data->common_data.distribution);
}



static inline
ccs_bool_t _check_value(_ccs_hyperparameter_numerical_data_t *d,
                        ccs_numeric_t value) {
	return ccs_interval_include(&(d->interval), value);
}

static ccs_error_t
_ccs_hyperparameter_numerical_samples(_ccs_hyperparameter_data_t *data,
                                      ccs_rng_t                   rng,
                                      size_t                      num_values,
                                      ccs_datum_t                *values) {
	_ccs_hyperparameter_numerical_data_t *d = (_ccs_hyperparameter_numerical_data_t *)data;
	ccs_error_t err;
	ccs_numeric_t *vs = (ccs_numeric_t *)values;
	err = ccs_distribution_samples(d->common_data.distribution,
	                               rng, num_values, vs);
	if (err)
		return err;
	if (!d->common_data.oversampling) {
		if (d->interval.type == CCS_NUM_FLOAT) {
			for(size_t i = 0; i < num_values; i++)
				values[num_values - 1 - i].value.f =
				    vs[num_values - 1 - i].f;
		} else {
			for(size_t i = 0; i < num_values; i++)
				values[num_values - 1 - i].value.i =
				    vs[num_values - 1 - i].i;
		}
	} else {
		size_t found = 0;
		if (d->interval.type == CCS_NUM_FLOAT) {
			for(size_t i = 0; i < num_values; i++)
				if (_check_value(d, vs[i]))
					values[found++].value.f = vs[i].f;
		} else {
			for(size_t i = 0; i < num_values; i++)
				if (_check_value(d, vs[i]))
					values[found++].value.i = vs[i].i;
		}
		vs = NULL;
		size_t coeff = 2;
		while (found < num_values) {
			size_t buff_sz = num_values - found;
			vs = (ccs_numeric_t *)malloc(sizeof(ccs_numeric_t)*buff_sz);
			if (!vs)
				return -CCS_ENOMEM;
			err = ccs_distribution_samples(d->common_data.distribution,
			                               rng, buff_sz, vs);
			if (d->interval.type == CCS_NUM_FLOAT) {
				for(size_t i = 0; i < buff_sz && found < num_values; i++)
					values[found++].value.f = vs[i].f;
			} else {
				for(size_t i = 0; i < buff_sz && found < num_values; i++)
					values[found++].value.i = vs[i].i;
			}
			coeff <<= 1;
			free(vs);
			if (coeff > 32)
				return -CCS_SAMPLING_UNSUCCESSFUL;
		}
	}
	for (size_t i = 0; i < num_values; i++)
		values[i].type = (ccs_data_type_t)(d->interval.type);
	return CCS_SUCCESS;
}

_ccs_hyperparameter_ops_t _ccs_hyperparameter_numerical_ops = {
	{ &_ccs_hyperparameter_numerical_del },
	&_ccs_hyperparameter_numerical_samples
};

ccs_error_t
_ccs_create_numerical_hyperparameter(const char           *name,
                                     ccs_numeric_type_t    data_type,
                                     ccs_numeric_t         lower,
                                     ccs_numeric_t         upper,
                                     ccs_numeric_t         quantization,
                                     ccs_numeric_t         default_value,
                                     ccs_distribution_t    distribution,
                                     void                 *user_data,
                                     ccs_hyperparameter_t *hyperparameter_ret) {
	if (!hyperparameter_ret)
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
	uintptr_t mem = (uintptr_t)calloc(1, sizeof(struct _ccs_hyperparameter_s) + sizeof(_ccs_hyperparameter_numerical_data_t));

	ccs_interval_t interval;
	interval.type = data_type;
	interval.lower = lower;
	interval.upper = upper;
	interval.lower_included = CCS_TRUE;
	interval.upper_included = CCS_FALSE;

	ccs_error_t err;
	if (!mem)
		return -CCS_ENOMEM;
	ccs_bool_t oversampling;
	if (!distribution) {
		err = ccs_create_uniform_distribution(data_type, lower, upper,
                                                      CCS_LINEAR, quantization,
                                                      &distribution);
		if (err) {
			free((void *)mem);
			return err;
		}
		oversampling = CCS_FALSE;
	} else {
		err = ccs_distribution_check_oversampling(distribution, &interval,
		                                          &oversampling);
		if (err) {
			ccs_release_object(distribution);
			free((void *)mem);
			return err;
		}
	}
 
	ccs_hyperparameter_t hyperparam = (ccs_hyperparameter_t)mem;
	_ccs_object_init(&(hyperparam->obj), CCS_HYPERPARAMETER, (_ccs_object_ops_t *)&_ccs_hyperparameter_numerical_ops);
	_ccs_hyperparameter_numerical_data_t *hyperparam_data = (_ccs_hyperparameter_numerical_data_t *)(mem + sizeof(struct _ccs_hyperparameter_s));
	hyperparam_data->common_data.type = CCS_NUMERICAL;
	hyperparam_data->common_data.name = name;
	hyperparam_data->common_data.user_data = user_data;
	hyperparam_data->common_data.distribution = distribution;
	if (data_type == CCS_NUM_FLOAT) {
		hyperparam_data->common_data.default_value.type = CCS_FLOAT;
		hyperparam_data->common_data.default_value.value.f = default_value.f;
	} else {
		hyperparam_data->common_data.default_value.type = CCS_INTEGER;
		hyperparam_data->common_data.default_value.value.i = default_value.i;
	}
	hyperparam_data->common_data.oversampling = oversampling;
	hyperparam_data->interval = interval;
	hyperparam_data->quantization = quantization;
	hyperparam->data = (_ccs_hyperparameter_data_t *)hyperparam_data;
	*hyperparameter_ret = hyperparam;

	return CCS_SUCCESS;
}
