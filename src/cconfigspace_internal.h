#ifndef _CONFIGSPACE_INTERNAL_H
#define _CONFIGSPACE_INTERNAL_H

#include <cconfigspace.h>

struct _ccs_object_ops_s {
	ccs_error_t (*del)(ccs_object_t object);
};

typedef struct _ccs_object_ops_s _ccs_object_ops_t;

struct _ccs_object_internal_s {
	ccs_object_type_t  type;
	int32_t            refcount;
	_ccs_object_ops_t *ops;
};

typedef struct _ccs_object_internal_s _ccs_object_internal_t;

static inline __attribute__((always_inline)) void
_ccs_object_init(_ccs_object_internal_t *o,
                 ccs_object_type_t       t,
                 _ccs_object_ops_t      *ops) {
	o->type = t;
	o->refcount = 1;
	o->ops = ops;
}

#endif //_CONFIGSPACE_INTERNAL_H
