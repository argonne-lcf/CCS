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
		char                    *buffer,
		char                   **buffer_out) {
	if (_ccs_serialize_header_size(format) > *buffer_size)
		return -CCS_OUT_OF_MEMORY;
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
	{
		char tag[4] = CCS_MAGIC_TAG;
		memcpy(buffer, tag, 4);
		buffer += 4;
		*buffer_size -= 4;
		buffer = _ccs_serialize_bin_uint32(
		    CCS_SERIALIZATION_API_VERSION, buffer_size, buffer);
		if (buffer_out)
			*buffer_out = buffer;
		return CCS_SUCCESS;
	}
	default:
		return -CCS_INVALID_VALUE;
	}
}

ccs_result_t
ccs_object_serialize(ccs_object_t           object,
                     ccs_serialize_format_t format,
                     ccs_serialize_type_t   type,
                     ...) {
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	va_list      args;
	char *buffer = NULL;
	size_t *p_buffer_size = NULL;
	size_t buffer_size = 0;

	if (!obj)
		return -CCS_INVALID_OBJECT;
	if (!obj->ops->serialize)
		return -CCS_UNSUPPORTED_OPERATION;

	va_start(args, type);
	switch(type) {
	case CCS_SERIALIZE_TYPE_SIZE:
		p_buffer_size = va_arg(args, size_t *);
		if (!p_buffer_size) {
			va_end(args);
			return -CCS_INVALID_VALUE;
		}
		*p_buffer_size = _ccs_serialize_header_size(format);
		CCS_VALIDATE(obj->ops->serialize(object, format, 0, NULL,
		    p_buffer_size, NULL));
		break;
	case CCS_SERIALIZE_TYPE_MEMORY:
		buffer_size = va_arg(args, size_t);
		buffer = va_arg(args, char *);
		CCS_VALIDATE(_ccs_serialize_header(
		    format, &buffer_size, buffer, &buffer));
		CCS_VALIDATE(obj->ops->serialize(
		    object, format, &buffer_size, buffer, 0, NULL));
		break;
	default:
		va_end(args);
		return -CCS_INVALID_VALUE;
	}
	va_end(args);

	return CCS_SUCCESS;
}

ccs_result_t
ccs_object_deserialize(ccs_object_t           *object_ret,
                       ccs_serialize_format_t  format,
                       ccs_serialize_type_t    type,
                       ...) {
	(void)object_ret;
	(void)format;
	(void)type;
	return -CCS_UNSUPPORTED_OPERATION;
}

