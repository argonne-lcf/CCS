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

