#include "cconfigspace_internal.h"
#include "hyperparameter_internal.h"
#include <string.h>

static ccs_error_t
_ccs_hyperparameter_numerical_del(ccs_object_t o) {
	(void)o;
	return CCS_SUCCESS;
}

static inline size_t
_ccs_serialize_bin_size_ccs_hyperparameter_numerical(
		ccs_hyperparameter_t hyperparameter) {
	_ccs_hyperparameter_numerical_data_t *data =
		(_ccs_hyperparameter_numerical_data_t *)(hyperparameter->data);
	return	_ccs_serialize_bin_size_ccs_object_internal(
			(_ccs_object_internal_t *)hyperparameter) +
	        _ccs_serialize_bin_size_ccs_hyperparameter_numerical_data(data);
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_hyperparameter_numerical(
		ccs_hyperparameter_t   hyperparameter,
		size_t                *buffer_size,
		char                 **buffer) {
	_ccs_hyperparameter_numerical_data_t *data =
		(_ccs_hyperparameter_numerical_data_t *)(hyperparameter->data);
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)hyperparameter, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_hyperparameter_numerical_data(
		data, buffer_size, buffer));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_hyperparameter_numerical_serialize_size(
		ccs_object_t                     object,
		ccs_serialize_format_t           format,
		size_t                          *cum_size,
		_ccs_object_serialize_options_t *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		*cum_size += _ccs_serialize_bin_size_ccs_hyperparameter_numerical(
			(ccs_hyperparameter_t)object);
		break;
	default:
		CCS_RAISE(CCS_INVALID_VALUE, "Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data_size(
		object, format, cum_size, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_hyperparameter_numerical_serialize(
		ccs_object_t                      object,
		ccs_serialize_format_t            format,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_hyperparameter_numerical(
		    (ccs_hyperparameter_t)object, buffer_size, buffer));
		break;
	default:
		CCS_RAISE(CCS_INVALID_VALUE, "Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data(
		object, format, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_hyperparameter_numerical_check_values(_ccs_hyperparameter_data_t *data,
                                           size_t                num_values,
                                           const ccs_datum_t    *values,
                                           ccs_datum_t          *values_ret,
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
				results[i] = _ccs_interval_include(interval, CCSF(values[i].value.f));
	}
	else {
		for(size_t i = 0; i < num_values; i++)
			if (values[i].type != CCS_INTEGER)
				results[i] = CCS_FALSE;
			else
				results[i] = _ccs_interval_include(interval, CCSI(values[i].value.i));
	}
	if (values_ret) {
		for (size_t i = 0; i < num_values; i++) {
			if (results[i] == CCS_TRUE) {
				values_ret[i] = values[i];
			} else {
				values_ret[i] = ccs_inactive;
			}
		}
	}
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_hyperparameter_numerical_samples(_ccs_hyperparameter_data_t *data,
                                      ccs_distribution_t          distribution,
                                      ccs_rng_t                   rng,
                                      size_t                      num_values,
                                      ccs_datum_t                *values) {
	_ccs_hyperparameter_numerical_data_t *d =
	    (_ccs_hyperparameter_numerical_data_t *)data;
	ccs_numeric_type_t type = d->common_data.interval.type;
	ccs_interval_t *interval = &(d->common_data.interval);
	ccs_error_t err;
	ccs_numeric_t *vs = (ccs_numeric_t *)values + num_values;
        ccs_bool_t oversampling;

	CCS_VALIDATE(ccs_distribution_check_oversampling(
	               distribution, interval, &oversampling));
	CCS_VALIDATE(ccs_distribution_samples(
	               distribution, rng, num_values, vs));
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
				if (_ccs_interval_include(interval, vs[i]))
					values[found++].value.f = vs[i].f;
		} else {
			for(size_t i = 0; i < num_values; i++)
				if (_ccs_interval_include(interval, vs[i]))
					values[found++].value.i = vs[i].i;
		}
		vs = NULL;
		size_t coeff = 2;
		while (found < num_values) {
			CCS_REFUTE(coeff > 32, CCS_SAMPLING_UNSUCCESSFUL);
			size_t buff_sz = (num_values - found)*coeff;
			ccs_numeric_t *oldvs = vs;
			vs = (ccs_numeric_t *)realloc(oldvs, sizeof(ccs_numeric_t)*buff_sz);
			if (CCS_UNLIKELY(!vs)) {
				if (oldvs)
					free(oldvs);
				CCS_RAISE(CCS_OUT_OF_MEMORY, "Not enough memory to reallocate buffer");
			}
			CCS_VALIDATE_ERR_GOTO(err,
			  ccs_distribution_samples(distribution, rng,
			                           buff_sz, vs),
			  errmem);
			if (type == CCS_NUM_FLOAT) {
				for(size_t i = 0; i < buff_sz && found < num_values; i++)
					if (_ccs_interval_include(interval, vs[i]))
						values[found++].value.f = vs[i].f;
			} else {
				for(size_t i = 0; i < buff_sz && found < num_values; i++)
					if (_ccs_interval_include(interval, vs[i]))
						values[found++].value.i = vs[i].i;
			}
			coeff <<= 1;
		}
		if (vs)
			free(vs);
	}
	for (size_t i = 0; i < num_values; i++) {
		values[i].type = (ccs_data_type_t)type;
		values[i].flags = CCS_FLAG_DEFAULT;
	}
	return CCS_SUCCESS;
errmem:
	free(vs);
	return err;
}

static ccs_error_t
_ccs_hyperparameter_numerical_get_default_distribution(
		_ccs_hyperparameter_data_t *data,
		ccs_distribution_t         *distribution) {
	_ccs_hyperparameter_numerical_data_t *d = (_ccs_hyperparameter_numerical_data_t *)data;
	ccs_interval_t *interval = &(d->common_data.interval);
	CCS_VALIDATE(ccs_create_uniform_distribution(
		interval->type, interval->lower, interval->upper,
		CCS_LINEAR, d->quantization, distribution));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_hyperparameter_numerical_convert_samples(
		_ccs_hyperparameter_data_t *data,
		ccs_bool_t                  oversampling,
		size_t                      num_values,
		const ccs_numeric_t        *values,
		ccs_datum_t                *results) {
	_ccs_hyperparameter_numerical_data_t *d =
	    (_ccs_hyperparameter_numerical_data_t *)data;
	ccs_numeric_type_t type = d->common_data.interval.type;
	ccs_interval_t *interval = &(d->common_data.interval);

	if (!oversampling) {
		if (type == CCS_NUM_FLOAT) {
			for(size_t i = 0; i < num_values; i++)
				results[i] = ccs_float(values[i].f);
		} else {
			for(size_t i = 0; i < num_values; i++)
				results[i] = ccs_int(values[i].i);
		}
	} else {
		if (type == CCS_NUM_FLOAT) {
			for(size_t i = 0; i < num_values; i++)
				if (_ccs_interval_include(interval, values[i]))
					results[i] = ccs_float(values[i].f);
				else
					results[i] = ccs_inactive;
		} else {
			for(size_t i = 0; i < num_values; i++)
				if (_ccs_interval_include(interval, values[i]))
					results[i] = ccs_int(values[i].i);
				else
					results[i] = ccs_inactive;
		}
	}
	return CCS_SUCCESS;
}

static _ccs_hyperparameter_ops_t _ccs_hyperparameter_numerical_ops = {
	{ &_ccs_hyperparameter_numerical_del,
	  &_ccs_hyperparameter_numerical_serialize_size,
	  &_ccs_hyperparameter_numerical_serialize },
	&_ccs_hyperparameter_numerical_check_values,
	&_ccs_hyperparameter_numerical_samples,
	&_ccs_hyperparameter_numerical_get_default_distribution,
	&_ccs_hyperparameter_numerical_convert_samples
};

ccs_error_t
ccs_create_numerical_hyperparameter(const char           *name,
                                    ccs_numeric_type_t    data_type,
                                    ccs_numeric_t         lower,
                                    ccs_numeric_t         upper,
                                    ccs_numeric_t         quantization,
                                    ccs_numeric_t         default_value,
                                    ccs_hyperparameter_t *hyperparameter_ret) {
	CCS_CHECK_PTR(name);
	CCS_CHECK_PTR(hyperparameter_ret);
	CCS_REFUTE(data_type != CCS_NUM_FLOAT && data_type != CCS_NUM_INTEGER, CCS_INVALID_TYPE);
	CCS_REFUTE(data_type == CCS_NUM_INTEGER && (lower.i >= upper.i || quantization.i < 0 || quantization.i > upper.i - lower.i || default_value.i < lower.i || default_value.i >= upper.i), CCS_INVALID_VALUE);
	CCS_REFUTE(data_type == CCS_NUM_FLOAT && (lower.f >= upper.f || quantization.f < 0.0 || quantization.f > upper.f - lower.f || default_value.f < lower.f || default_value.f >= upper.f), CCS_INVALID_VALUE);
	uintptr_t mem = (uintptr_t)calloc(1, sizeof(struct _ccs_hyperparameter_s) + sizeof(_ccs_hyperparameter_numerical_data_t) + strlen(name) + 1);
	CCS_REFUTE(!mem, CCS_OUT_OF_MEMORY);

	ccs_interval_t interval;
	interval.type = data_type;
	interval.lower = lower;
	interval.upper = upper;
	interval.lower_included = CCS_TRUE;
	interval.upper_included = CCS_FALSE;

	ccs_hyperparameter_t hyperparam = (ccs_hyperparameter_t)mem;
	_ccs_object_init(&(hyperparam->obj), CCS_HYPERPARAMETER, (_ccs_object_ops_t *)&_ccs_hyperparameter_numerical_ops);
	_ccs_hyperparameter_numerical_data_t *hyperparam_data = (_ccs_hyperparameter_numerical_data_t *)(mem + sizeof(struct _ccs_hyperparameter_s));
	hyperparam_data->common_data.type = CCS_HYPERPARAMETER_TYPE_NUMERICAL;
	hyperparam_data->common_data.name = (char *)(mem + sizeof(struct _ccs_hyperparameter_s) + sizeof(_ccs_hyperparameter_numerical_data_t));
	strcpy((char *)hyperparam_data->common_data.name, name);
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

ccs_error_t
ccs_numerical_hyperparameter_get_parameters(ccs_hyperparameter_t  hyperparameter,
                                            ccs_numeric_type_t   *data_type_ret,
                                            ccs_numeric_t        *lower_ret,
                                            ccs_numeric_t        *upper_ret,
                                            ccs_numeric_t        *quantization_ret) {
	CCS_CHECK_HYPERPARAMETER(hyperparameter, CCS_HYPERPARAMETER_TYPE_NUMERICAL);
	CCS_REFUTE(!data_type_ret && !lower_ret && !upper_ret && !quantization_ret, CCS_INVALID_VALUE);
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


