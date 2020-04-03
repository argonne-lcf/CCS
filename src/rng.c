#include "cconfigspace_internal.h"
#include "rng_internal.h"
#include <stdlib.h>

static ccs_error_t
_ccs_rng_del(ccs_object_t object);

static struct _ccs_rng_ops_s _rng_ops = { {&_ccs_rng_del} };

ccs_error_t
ccs_rng_create_with_type(const gsl_rng_type *rng_type,
                         ccs_rng_t          *rng_ret) {
	if (!rng_type || !rng_ret)
		return -CCS_INVALID_VALUE;
	gsl_rng *grng = gsl_rng_alloc(rng_type);

	if (!grng) {
		return -CCS_ENOMEM;
	}
	uintptr_t mem = (uintptr_t)calloc(1, sizeof(struct _ccs_rng_s) + sizeof(struct _ccs_rng_data_s));

	if (!mem) {
		gsl_rng_free(grng);
		return -CCS_ENOMEM;
	}
	ccs_rng_t rng = (ccs_rng_t)mem;
	_ccs_object_init(&(rng->obj), CCS_RNG, (_ccs_object_ops_t *)&_rng_ops);
	rng->data = (struct _ccs_rng_data_s *)(mem + sizeof(struct _ccs_rng_s));
	rng->data->rng_type = rng_type;
	rng->data->rng = grng;
	*rng_ret = rng;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_rng_create(ccs_rng_t *rng_ret) {
	return ccs_rng_create_with_type(gsl_rng_default, rng_ret);
}

static ccs_error_t
_ccs_rng_del(ccs_object_t object) {
	gsl_rng_free(((ccs_rng_t)object)->data->rng);
	((ccs_rng_t)object)->data->rng = NULL;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_rng_get_type(ccs_rng_t            rng,
                 const gsl_rng_type **rng_type_ret) {
	if (!rng)
		return -CCS_INVALID_OBJECT;
	*rng_type_ret = rng->data->rng_type;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_rng_set_seed(ccs_rng_t         rng,
                 unsigned long int seed) {
	if (!rng)
		return -CCS_INVALID_OBJECT;
	gsl_rng_set(rng->data->rng, seed);
	return CCS_SUCCESS;
}

ccs_error_t
ccs_rng_get(ccs_rng_t          rng,
            unsigned long int *value_ret) {
	if (!rng)
		return -CCS_INVALID_OBJECT;
	if (!value_ret)
		return -CCS_INVALID_VALUE;
	*value_ret = gsl_rng_get(rng->data->rng);
	return CCS_SUCCESS;
}

ccs_error_t
ccs_rng_uniform(ccs_rng_t    rng,
                ccs_float_t *value_ret) {
	if (!rng)
		return -CCS_INVALID_OBJECT;
	if (!value_ret)
		return -CCS_INVALID_VALUE;
	*value_ret = gsl_rng_uniform(rng->data->rng);
	return CCS_SUCCESS;
}

ccs_error_t
ccs_rng_get_gsl_rng(ccs_rng_t   rng,
                    gsl_rng   **gsl_rng_ret) {
	if (!rng)
		return -CCS_INVALID_OBJECT;
	if (!gsl_rng_ret)
		return -CCS_INVALID_VALUE;
	*gsl_rng_ret = rng->data->rng;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_rng_min(ccs_rng_t          rng,
            unsigned long int *value_ret) {
	if (!rng)
		return -CCS_INVALID_OBJECT;
	if (!value_ret)
		return -CCS_INVALID_VALUE;
	*value_ret = gsl_rng_min(rng->data->rng);
	return CCS_SUCCESS;
}

ccs_error_t
ccs_rng_max(ccs_rng_t          rng,
            unsigned long int *value_ret) {
	if (!rng)
		return -CCS_INVALID_OBJECT;
	if (!value_ret)
		return -CCS_INVALID_VALUE;
	*value_ret = gsl_rng_max(rng->data->rng);
	return CCS_SUCCESS;
}
