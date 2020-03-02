#include "cconfigspace_internal.h"

ccs_error_t
ccs_retain_object(ccs_object_t object) {
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object.ptr;
        if (!obj || obj->refcount <= 0)
		return -CCS_INVALID_OBJECT;
	obj->refcount += 1;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_release_object(ccs_object_t object) {
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object.ptr;
	if (!obj || obj->refcount <= 0)
		return -CCS_INVALID_OBJECT;
	obj->refcount -= 1;
	if (obj->refcount == 0)
		return obj->ops->del(object);
	return CCS_SUCCESS;
}

ccs_error_t
ccs_object_get_type(ccs_object_t       object,
                    ccs_object_type_t *type_ret) {
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object.ptr;
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
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object.ptr;
	if (!obj)
		return -CCS_INVALID_OBJECT;
        if (!refcount_ret)
		return -CCS_INVALID_VALUE;
	*refcount_ret = obj->refcount;
	return CCS_SUCCESS;
}
