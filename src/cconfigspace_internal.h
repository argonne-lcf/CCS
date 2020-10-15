#ifndef _CONFIGSPACE_INTERNAL_H
#define _CONFIGSPACE_INTERNAL_H

#include <cconfigspace.h>

static inline ccs_bool_t
_ccs_interval_include(ccs_interval_t *interval, ccs_numeric_t value) {
	if (interval->type == CCS_NUM_FLOAT) {
		return ( interval->lower_included ?
		           interval->lower.f <= value.f :
		           interval->lower.f < value.f ) &&
		       ( interval->upper_included ?
		           interval->upper.f >= value.f :
		           interval->upper.f > value.f );
	} else {
		return ( interval->lower_included ?
		           interval->lower.i <= value.i :
		           interval->lower.i < value.i ) &&
		       ( interval->upper_included ?
		           interval->upper.i >= value.i :
		           interval->upper.i > value.i );
	}
}

#define CCS_CHECK_OBJ(o, t) do { \
	if (unlikely(!(o) || \
	    !((_ccs_object_template_t *)(o))->data || \
	     ((_ccs_object_template_t *)(o))->obj.type != (t))) \
		return -CCS_INVALID_OBJECT; \
} while (0)

#define CCS_CHECK_PTR(p) do { \
	if (unlikely(!(p))) \
		return -CCS_INVALID_VALUE; \
} while (0)

#define CCS_CHECK_ARY(c, a) do { \
	if (unlikely((c > 0) && !(a))) \
		return -CCS_INVALID_VALUE; \
} while (0)

struct _ccs_object_ops_s {
	ccs_result_t (*del)(ccs_object_t object);
};

typedef struct _ccs_object_ops_s _ccs_object_ops_t;

struct _ccs_object_internal_s {
	ccs_object_type_t              type;
	int32_t                        refcount;
        ccs_object_release_callback_t  cb;
	void                          *cb_user_data;
	_ccs_object_ops_t             *ops;
};

typedef struct _ccs_object_internal_s _ccs_object_internal_t;

struct _ccs_object_template_s {
	_ccs_object_internal_t  obj;
	void                   *data;
};
typedef struct _ccs_object_template_s _ccs_object_template_t;

static inline __attribute__((always_inline)) void
_ccs_object_init(_ccs_object_internal_t *o,
                 ccs_object_type_t       t,
                 _ccs_object_ops_t      *ops) {
	o->type = t;
	o->refcount = 1;
	o->cb = NULL;
	o->cb_user_data = NULL;
	o->ops = ops;
}

#endif //_CONFIGSPACE_INTERNAL_H
