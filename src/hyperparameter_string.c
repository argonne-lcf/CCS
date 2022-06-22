#include "cconfigspace_internal.h"
#include "hyperparameter_internal.h"
#include "datum_uthash.h"
#include "datum_hash.h"
#include <string.h>

struct _ccs_hyperparameter_string_data_s {
	_ccs_hyperparameter_common_data_t  common_data;
	_ccs_hash_datum_t                 *stored_values;
};
typedef struct _ccs_hyperparameter_string_data_s _ccs_hyperparameter_string_data_t;

static inline size_t
_ccs_serialize_bin_size_ccs_hyperparameter_string_data(
		_ccs_hyperparameter_string_data_t *data) {
	return _ccs_serialize_bin_size_ccs_hyperparameter_common_data(&data->common_data);
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_hyperparameter_string_data(
		_ccs_hyperparameter_string_data_t  *data,
		size_t                             *buffer_size,
		char                              **buffer) {
	CCS_VALIDATE(_ccs_serialize_bin_ccs_hyperparameter_common_data(
		&data->common_data, buffer_size, buffer));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_hyperparameter_string_del(ccs_object_t o) {
	ccs_hyperparameter_t d = (ccs_hyperparameter_t)o;
	_ccs_hyperparameter_string_data_t *data = (_ccs_hyperparameter_string_data_t *)(d->data);
	_ccs_hash_datum_t *current, *tmp;
	HASH_ITER(hh, data->stored_values, current, tmp) {
		HASH_DEL(data->stored_values, current);
		free(current);
	}
	return CCS_SUCCESS;
}

static inline size_t
_ccs_serialize_bin_size_ccs_hyperparameter_string(
		ccs_hyperparameter_t hyperparameter) {
	_ccs_hyperparameter_string_data_t *data =
		(_ccs_hyperparameter_string_data_t *)(hyperparameter->data);
	return  _ccs_serialize_bin_size_ccs_object_internal(
			(_ccs_object_internal_t *)hyperparameter) +
		_ccs_serialize_bin_size_ccs_hyperparameter_string_data(data);
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_hyperparameter_string(
		ccs_hyperparameter_t   hyperparameter,
		size_t                *buffer_size,
		char                 **buffer) {
	_ccs_hyperparameter_string_data_t *data =
		(_ccs_hyperparameter_string_data_t *)(hyperparameter->data);
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)hyperparameter, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_hyperparameter_string_data(
		data, buffer_size, buffer));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_hyperparameter_string_serialize_size(
		ccs_object_t                     object,
		ccs_serialize_format_t           format,
		size_t                          *cum_size,
		_ccs_object_serialize_options_t *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		*cum_size += _ccs_serialize_bin_size_ccs_hyperparameter_string(
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
_ccs_hyperparameter_string_serialize(
		ccs_object_t                      object,
		ccs_serialize_format_t            format,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_hyperparameter_string(
		    (ccs_hyperparameter_t)object, buffer_size, buffer));
		break;
	default:
		CCS_RAISE(CCS_INVALID_VALUE, "Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data(
		object, format, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

#undef uthash_nonfatal_oom
#define uthash_nonfatal_oom(elt) { \
	CCS_RAISE(CCS_OUT_OF_MEMORY, "Not enough memory to allocate array"); \
}

static ccs_error_t
_ccs_hyperparameter_string_check_values(_ccs_hyperparameter_data_t *data,
                                        size_t                      num_values,
                                        const ccs_datum_t          *values,
                                        ccs_datum_t                *values_ret,
                                        ccs_bool_t                 *results) {
	_ccs_hyperparameter_string_data_t *d =
	    (_ccs_hyperparameter_string_data_t *)data;
	for(size_t i = 0; i < num_values; i++)
		if (values[i].type != CCS_STRING)
			results[i] = CCS_FALSE;
		else
			results[i] = CCS_TRUE;
	if (values_ret) {
		for (size_t i = 0; i < num_values; i++)
			if (results[i] == CCS_TRUE) {
				_ccs_hash_datum_t *p;
				HASH_FIND(hh, d->stored_values, values + i, sizeof(ccs_datum_t), p);
				if (!p) {
					size_t sz_str = 0;
					if (values[i].value.s)
						sz_str = strlen(values[i].value.s) + 1;
					p = (_ccs_hash_datum_t *)malloc(sizeof(_ccs_hash_datum_t) + sz_str);
					CCS_REFUTE(!p, CCS_OUT_OF_MEMORY);
					if (sz_str) {
						strcpy((char *)((intptr_t)p + sizeof(_ccs_hash_datum_t)), values[i].value.s);
						p->d = ccs_string((char *)((intptr_t)p + sizeof(_ccs_hash_datum_t)));
					} else
						p->d = ccs_string(NULL);
					HASH_ADD(hh, d->stored_values, d, sizeof(ccs_datum_t), p);
				}
				values_ret[i] = p->d;
			} else {
				values_ret[i] = ccs_inactive;
			}
	}
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_hyperparameter_string_samples(_ccs_hyperparameter_data_t *data,
                                   ccs_distribution_t          distribution,
                                   ccs_rng_t                   rng,
                                   size_t                      num_values,
                                   ccs_datum_t                *values) {
	(void)data;
	(void)distribution;
	(void)rng;
	(void)num_values;
	(void)values;
	CCS_RAISE(CCS_UNSUPPORTED_OPERATION, "String hyperparameters cannot be sampled");
}

static ccs_error_t
_ccs_hyperparameter_string_get_default_distribution(
		_ccs_hyperparameter_data_t *data,
		ccs_distribution_t         *distribution) {
	(void)data;
	(void)distribution;
	CCS_RAISE(CCS_UNSUPPORTED_OPERATION, "String hyperparameters don't have default distributions");
}

static ccs_error_t
_ccs_hyperparameter_string_convert_samples(
		_ccs_hyperparameter_data_t *data,
		ccs_bool_t                  oversampling,
		size_t                      num_values,
		const ccs_numeric_t        *values,
		ccs_datum_t                *results) {
	(void)data;
	(void)oversampling;
	(void)num_values;
	(void)values;
	(void)results;
	CCS_RAISE(CCS_UNSUPPORTED_OPERATION, "String hyperparameters cannot convert samples");
}

static _ccs_hyperparameter_ops_t _ccs_hyperparameter_string_ops = {
	{ &_ccs_hyperparameter_string_del,
	  &_ccs_hyperparameter_string_serialize_size,
	  &_ccs_hyperparameter_string_serialize },
	&_ccs_hyperparameter_string_check_values,
	&_ccs_hyperparameter_string_samples,
	&_ccs_hyperparameter_string_get_default_distribution,
	&_ccs_hyperparameter_string_convert_samples
};

extern ccs_error_t
ccs_create_string_hyperparameter(const char           *name,
                                 ccs_hyperparameter_t *hyperparameter_ret) {
	CCS_CHECK_PTR(name);
	CCS_CHECK_PTR(hyperparameter_ret);
	uintptr_t mem = (uintptr_t)calloc(1, sizeof(struct _ccs_hyperparameter_s) + sizeof(_ccs_hyperparameter_string_data_t) + strlen(name) + 1);
	CCS_REFUTE(!mem, CCS_OUT_OF_MEMORY);

	ccs_hyperparameter_t hyperparam = (ccs_hyperparameter_t)mem;
	_ccs_object_init(&(hyperparam->obj), CCS_HYPERPARAMETER, (_ccs_object_ops_t *)&_ccs_hyperparameter_string_ops);
	_ccs_hyperparameter_string_data_t *hyperparam_data = (_ccs_hyperparameter_string_data_t *)(mem + sizeof(struct _ccs_hyperparameter_s));
	hyperparam_data->common_data.type = CCS_HYPERPARAMETER_TYPE_STRING;
	hyperparam_data->common_data.name = (char *)(mem + sizeof(struct _ccs_hyperparameter_s) + sizeof(_ccs_hyperparameter_string_data_t));
	strcpy((char *)hyperparam_data->common_data.name, name);
	hyperparam_data->common_data.interval.type = CCS_NUM_INTEGER;
	hyperparam_data->stored_values = NULL;
	hyperparam->data = (_ccs_hyperparameter_data_t *)hyperparam_data;
	*hyperparameter_ret = hyperparam;

	return CCS_SUCCESS;
}
