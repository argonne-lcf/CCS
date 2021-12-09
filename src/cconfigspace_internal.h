#ifndef _CONFIGSPACE_INTERNAL_H
#define _CONFIGSPACE_INTERNAL_H

#include <cconfigspace.h>
#include "utarray.h"

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
	if (CCS_UNLIKELY(!(o) || \
	    !((_ccs_object_template_t *)(o))->data || \
	     ((_ccs_object_template_t *)(o))->obj.type != (t))) \
		return -CCS_INVALID_OBJECT; \
} while (0)

#define CCS_CHECK_PTR(p) do { \
	if (CCS_UNLIKELY(!(p))) \
		return -CCS_INVALID_VALUE; \
} while (0)

#define CCS_CHECK_ARY(c, a) do { \
	if (CCS_UNLIKELY((c > 0) && !(a))) \
		return -CCS_INVALID_VALUE; \
} while (0)

#define CCS_VALIDATE_ERR_GOTO(err, cmd, label) do { \
	err = (cmd); \
	if (CCS_UNLIKELY(err != CCS_SUCCESS)) \
		goto label; \
} while (0)

#define CCS_VALIDATE_ERR(err, cmd) do { \
	err = (cmd); \
	if (CCS_UNLIKELY(err != CCS_SUCCESS)) \
		return err; \
} while (0)

#define CCS_VALIDATE(cmd) do { \
	ccs_result_t _err; \
	CCS_VALIDATE_ERR(_err, cmd); \
} while(0)

struct _ccs_object_ops_s {
	ccs_result_t (*del)(ccs_object_t object);
};

typedef struct _ccs_object_ops_s _ccs_object_ops_t;

struct _ccs_object_callback_s {
	ccs_object_release_callback_t  callback;
	void                          *user_data;
};
typedef struct _ccs_object_callback_s _ccs_object_callback_t;

struct _ccs_object_internal_s {
	ccs_object_type_t              type;
	int32_t                        refcount;
	UT_array                      *callbacks;
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
	o->callbacks = NULL;
	o->ops = ops;
}

#endif //_CONFIGSPACE_INTERNAL_H
