#include "cconfigspace_internal.h"
#include <stdlib.h>
#include <gsl/gsl_rng.h>

const ccs_datum_t ccs_none = CCS_NONE_VAL;
const ccs_datum_t ccs_inactive = CCS_INACTIVE_VAL;

ccs_error_t
ccs_init() {
	gsl_rng_env_setup();
	return CCS_SUCCESS;
}

ccs_error_t
ccs_retain_object(ccs_object_t object) {
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
        if (!obj || obj->refcount <= 0)
		return -CCS_INVALID_OBJECT;
	obj->refcount += 1;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_release_object(ccs_object_t object) {
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	if (!obj || obj->refcount <= 0)
		return -CCS_INVALID_OBJECT;
	obj->refcount -= 1;
	if (obj->refcount == 0) {
		ccs_error_t err = obj->ops->del(object);
		if (err)
			return err;
		free(object);
	}
	return CCS_SUCCESS;
}

ccs_error_t
ccs_object_get_type(ccs_object_t       object,
                     ccs_object_type_t *type_ret) {
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	if (!obj)
		return -CCS_INVALID_OBJECT;
	if (!type_ret)
		return -CCS_INVALID_VALUE;
	*type_ret = obj->type;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_object_get_refcount(ccs_object_t  object,
                         int32_t      *refcount_ret) {
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	if (!obj)
		return -CCS_INVALID_OBJECT;
        if (!refcount_ret)
		return -CCS_INVALID_VALUE;
	*refcount_ret = obj->refcount;
	return CCS_SUCCESS;
}
