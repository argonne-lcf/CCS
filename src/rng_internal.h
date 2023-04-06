#ifndef _RNG_INTERNAL_H
#define _RNG_INTERNAL_H

#include <gsl/gsl_rng.h>

struct _ccs_rng_data_s {
	const gsl_rng_type *rng_type;
	gsl_rng            *rng;
};

typedef struct _ccs_rng_data_s _ccs_rng_data_t;

struct _ccs_rng_ops_s {
	_ccs_object_ops_t obj_ops;
};
typedef struct _ccs_rng_ops_s _ccs_rng_ops_t;

struct _ccs_rng_s {
	_ccs_object_internal_t obj;
	_ccs_rng_data_t       *data;
};

#endif //_RNG_INTERNAL_H
