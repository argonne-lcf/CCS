#include "cconfigspace_internal.h"

ccs_error_t
ccs_retain_object(ccs_object_t object) {
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
        if (obj->refcount <= 0)
		return CCS_INVALID_OBJECT;
	obj->refcount += 1;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_release_object(ccs_object_t object) {
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	if (obj->refcount <= 0)
		return CCS_INVALID_OBJECT;
	obj->refcount -= 1;
	if (obj->refcount == 0)
		return obj->obs->del(object);
	return CCS_SUCCESS;
}
