#include "cconfigspace_internal.h"
#include <stdlib.h>
#include <gsl/gsl_rng.h>

const ccs_datum_t ccs_none = CCS_NONE_VAL;
const ccs_datum_t ccs_inactive = CCS_INACTIVE_VAL;
const ccs_datum_t ccs_true = CCS_TRUE_VAL;
const ccs_datum_t ccs_false = CCS_FALSE_VAL;
const ccs_version_t ccs_version = { 0, 1, 0, 0 };

ccs_result_t
ccs_init() {
	gsl_rng_env_setup();
	return CCS_SUCCESS;
}

ccs_result_t
ccs_fini() {
	return CCS_SUCCESS;
}

ccs_version_t
ccs_get_version() {
	return ccs_version;
}

ccs_result_t
ccs_retain_object(ccs_object_t object) {
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
        if (!obj || obj->refcount <= 0)
		return -CCS_INVALID_OBJECT;
	obj->refcount += 1;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_release_object(ccs_object_t object) {
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	if (!obj || obj->refcount <= 0)
		return -CCS_INVALID_OBJECT;
	obj->refcount -= 1;
	if (obj->refcount == 0) {
		if (obj->callbacks) {
			_ccs_object_callback_t *cb = NULL;
			while ( (cb = (_ccs_object_callback_t *)
			              utarray_prev(obj->callbacks, cb)) ) {
				cb->callback(object, cb->user_data);
			}
			utarray_free(obj->callbacks);
		}
		CCS_VALIDATE(obj->ops->del(object));
		free(object);
	}
	return CCS_SUCCESS;
}

ccs_result_t
ccs_object_get_type(ccs_object_t       object,
                     ccs_object_type_t *type_ret) {
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	if (!obj)
		return -CCS_INVALID_OBJECT;
	CCS_CHECK_PTR(type_ret);
	*type_ret = obj->type;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_object_get_refcount(ccs_object_t  object,
                         int32_t      *refcount_ret) {
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	if (!obj)
		return -CCS_INVALID_OBJECT;
	CCS_CHECK_PTR(refcount_ret);
	*refcount_ret = obj->refcount;
	return CCS_SUCCESS;
}

static const UT_icd _object_callback_icd = {
	sizeof(_ccs_object_callback_t),
	NULL,
	NULL,
	NULL
};

ccs_result_t
ccs_object_set_destroy_callback(ccs_object_t  object,
                                void (*callback)(
                                  ccs_object_t object,
                                  void *user_data),
                                void *user_data) {
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	if (!obj)
		return -CCS_INVALID_OBJECT;
	if (!callback)
		return -CCS_INVALID_VALUE;
	if (!obj->callbacks)
		utarray_new(obj->callbacks, &_object_callback_icd);

	_ccs_object_callback_t cb = { callback, user_data };
	utarray_push_back(obj->callbacks, &cb);
	return CCS_SUCCESS;
}

ccs_result_t
ccs_object_set_user_data(ccs_object_t  object,
                         void         *user_data) {
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	if (!obj)
		return -CCS_INVALID_OBJECT;
	obj->user_data = user_data;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_object_get_user_data(ccs_object_t   object,
                         void         **user_data_ret) {
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	if (!obj)
		return -CCS_INVALID_OBJECT;
	CCS_CHECK_PTR(user_data_ret);
	*user_data_ret = obj->user_data;
	return CCS_SUCCESS;
}

static size_t
_ccs_serialize_header_size(ccs_serialize_format_t format) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		/* MAGIC + size */
		return 4 + sizeof(CCS_SERIALIZATION_API_VERSION_TYPE);
		break;
	default:
		return 0;
	}
}

ccs_result_t
_ccs_serialize_header(
		ccs_serialize_format_t   format,
		size_t                  *buffer_size,
		char                   **buffer) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
	{
		if (CCS_UNLIKELY(*buffer_size < 4))
			return -CCS_OUT_OF_MEMORY;
		const char tag[4] = CCS_MAGIC_TAG;
		memcpy(*buffer, tag, 4);
		*buffer += 4;
		*buffer_size -= 4;
		CCS_VALIDATE(_ccs_serialize_bin_uint32(
		    CCS_SERIALIZATION_API_VERSION, buffer_size, buffer));
	}
	break;
	default:
		return -CCS_INVALID_VALUE;
	}
	return CCS_SUCCESS;
}

ccs_result_t
_ccs_deserialize_header(
		ccs_serialize_format_t   format,
		size_t                  *buffer_size,
		const char             **buffer,
		uint32_t                *version) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
	{
		if (CCS_UNLIKELY(*buffer_size < 4))
			return -CCS_NOT_ENOUGH_DATA;
		const char ccs_tag[4] = CCS_MAGIC_TAG;
		char tag[4];
		memcpy(tag, *buffer, 4);
		*buffer += 4;
		*buffer_size -= 4;
		if (CCS_UNLIKELY(memcmp(tag, ccs_tag, 4)))
			return -CCS_INVALID_VALUE;
		CCS_VALIDATE(_ccs_deserialize_bin_uint32(
			version, buffer_size, buffer));
		if (CCS_UNLIKELY(*version > CCS_SERIALIZATION_API_VERSION))
			return -CCS_INVALID_VALUE;
	}
	break;
	default:
		return -CCS_INVALID_VALUE;
	}
	return CCS_SUCCESS;
}

ccs_result_t
ccs_object_serialize(ccs_object_t           object,
                     ccs_serialize_format_t format,
                     ccs_serialize_type_t   type,
                     ...) {
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	va_list args;
	char *buffer = NULL;
	size_t *p_buffer_size = NULL;
	size_t buffer_size = 0;

	if (!obj)
		return -CCS_INVALID_OBJECT;
	if (!obj->ops->serialize)
		return -CCS_UNSUPPORTED_OPERATION;

	switch(type) {
	case CCS_SERIALIZE_TYPE_SIZE:
		va_start(args, type);
		p_buffer_size = va_arg(args, size_t *);
		va_end(args);
		CCS_CHECK_PTR(p_buffer_size);
		*p_buffer_size = _ccs_serialize_header_size(format);
		CCS_VALIDATE(obj->ops->serialize_size(
		    object, format, p_buffer_size));
		break;
	case CCS_SERIALIZE_TYPE_MEMORY:
		va_start(args, type);
		buffer_size = va_arg(args, size_t);
		buffer = va_arg(args, char *);
		va_end(args);
		CCS_CHECK_PTR(buffer);
		CCS_VALIDATE(_ccs_serialize_header(
		    format, &buffer_size, &buffer));
		CCS_VALIDATE(obj->ops->serialize(
		    object, format, &buffer_size, &buffer));
		break;
	default:
		return -CCS_INVALID_VALUE;
	}
	return CCS_SUCCESS;
}

#include "rng_deserialize.h"
#include "distribution_deserialize.h"
#include "hyperparameter_deserialize.h"
#include "expression_deserialize.h"
#include "features_space_deserialize.h"
#include "configuration_space_deserialize.h"
#include "objective_space_deserialize.h"
#include "configuration_deserialize.h"
#include "features_deserialize.h"

static inline ccs_result_t
_ccs_object_deserialize_options(ccs_serialize_format_t             format,
                                va_list                            args,
                                _ccs_object_deserialize_options_t *opts) {
	(void)format;
	ccs_deserialize_option_t opt =
		(ccs_deserialize_option_t)va_arg(args, int);
	while (opt != CCS_DESERIALIZE_OPTION_END) {
		switch (opt) {
		case CCS_DESERIALIZE_OPTION_HANDLE_MAP:
			opts->handle_map = va_arg(args, ccs_map_t);
			CCS_CHECK_OBJ(opts->handle_map, CCS_MAP);
			CCS_CHECK_PTR(opts->handle_map);
			break;
		default:
			return CCS_INVALID_VALUE;
		}
		opt = (ccs_deserialize_option_t)va_arg(args, int);
	}
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_object_deserialize(ccs_object_t            *object_ret,
                        ccs_serialize_format_t   format,
                        size_t                  *buffer_size,
                        const char             **buffer,
                        va_list args) {
	uint32_t version;
	_ccs_object_deserialize_options_t opts = { NULL, CCS_TRUE };
	CCS_VALIDATE(_ccs_object_deserialize_options(format, args, &opts));
	CCS_VALIDATE(_ccs_deserialize_header(
		format, buffer_size, buffer, &version));
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
	{
		ccs_object_type_t otype;
		CCS_VALIDATE(_ccs_peek_bin_ccs_object_type(
			&otype, buffer_size, buffer));
		switch(otype) {
		case CCS_RNG:
			CCS_VALIDATE(_ccs_rng_deserialize(
				(ccs_rng_t *)object_ret,
				format, version, buffer_size, buffer, &opts));
			break;
		case CCS_DISTRIBUTION:
			CCS_VALIDATE(_ccs_distribution_deserialize(
				(ccs_distribution_t *)object_ret,
				format, version, buffer_size, buffer, &opts));
			break;
		case CCS_HYPERPARAMETER:
			CCS_VALIDATE(_ccs_hyperparameter_deserialize(
				(ccs_hyperparameter_t *)object_ret,
				format, version, buffer_size, buffer, &opts));
			break;
		case CCS_EXPRESSION:
			CCS_VALIDATE(_ccs_expression_deserialize(
				(ccs_expression_t *)object_ret,
				format, version, buffer_size, buffer, &opts));
			break;
		case CCS_FEATURES_SPACE:
			CCS_VALIDATE(_ccs_features_space_deserialize(
				(ccs_features_space_t *)object_ret,
				format, version, buffer_size, buffer, &opts));
			break;
		case CCS_CONFIGURATION_SPACE:
			CCS_VALIDATE(_ccs_configuration_space_deserialize(
				(ccs_configuration_space_t *)object_ret,
				format, version, buffer_size, buffer, &opts));
			break;
		case CCS_OBJECTIVE_SPACE:
			CCS_VALIDATE(_ccs_objective_space_deserialize(
				(ccs_objective_space_t *)object_ret,
				format, version, buffer_size, buffer, &opts));
			break;
		case CCS_CONFIGURATION:
			CCS_VALIDATE(_ccs_configuration_deserialize(
				(ccs_configuration_t *)object_ret,
				format, version, buffer_size, buffer, &opts));
			break;
		case CCS_FEATURES:
			CCS_VALIDATE(_ccs_features_deserialize(
				(ccs_features_t *)object_ret,
				format, version, buffer_size, buffer, &opts));
			break;
		default:
			return -CCS_UNSUPPORTED_OPERATION;
		}
	}
	break;
	default:
		return -CCS_INVALID_VALUE;
	}
	return CCS_SUCCESS;
}


ccs_result_t
ccs_object_deserialize(ccs_object_t           *object_ret,
                       ccs_serialize_format_t  format,
                       ccs_serialize_type_t    type,
                       ...) {
	ccs_result_t res = CCS_SUCCESS;
	va_list args;
	const char *buffer = NULL;
	size_t buffer_size = 0;

	if (!object_ret)
		return -CCS_INVALID_VALUE;

	switch (type) {
	case CCS_SERIALIZE_TYPE_MEMORY:
		va_start(args, type);
		buffer_size = va_arg(args, size_t);
		buffer = va_arg(args, const char *);
		if(CCS_UNLIKELY(!buffer)) {
			res = -CCS_INVALID_VALUE;
			goto end_memory;
		}
		CCS_VALIDATE_ERR_GOTO(res, _ccs_object_deserialize(
			object_ret, format, &buffer_size, &buffer, args),
			end_memory);
end_memory:
		va_end(args);
		break;
	default:
		return -CCS_INVALID_VALUE;
	}

	return res;
}

