#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <math.h>
#include "cconfigspace_internal.h"
#include "distribution_internal.h"

struct _ccs_distribution_roulette_data_s {
	_ccs_distribution_common_data_t  common_data;
	ccs_int_t                        num_areas;
	ccs_float_t                     *areas;
};
typedef struct _ccs_distribution_roulette_data_s _ccs_distribution_roulette_data_t;

static ccs_error_t
_ccs_distribution_del(ccs_object_t o) {
	(void)o;
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_distribution_roulette_get_bounds(_ccs_distribution_data_t *data,
                                      ccs_interval_t           *interval_ret);

static ccs_error_t
_ccs_distribution_roulette_samples(_ccs_distribution_data_t *data,
                                   ccs_rng_t                 rng,
                                   size_t                    num_values,
                                   ccs_numeric_t            *values);

static _ccs_distribution_ops_t _ccs_distribution_roulette_ops = {
	{ &_ccs_distribution_del },
	&_ccs_distribution_roulette_samples,
	&_ccs_distribution_roulette_get_bounds
};

static ccs_error_t
_ccs_distribution_roulette_get_bounds(_ccs_distribution_data_t *data,
                                      ccs_interval_t           *interval_ret) {
	_ccs_distribution_roulette_data_t *d = (_ccs_distribution_roulette_data_t *)data;

	interval_ret->type = CCS_NUM_INTEGER;
	interval_ret->lower = CCSI(INT64_C(0));
	interval_ret->upper = CCSI(d->num_areas);
	interval_ret->lower_included = CCS_TRUE;
	interval_ret->upper_included = CCS_FALSE;
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_distribution_roulette_samples(_ccs_distribution_data_t *data,
                                   ccs_rng_t                 rng,
                                   size_t                    num_values,
                                   ccs_numeric_t            *values) {
	_ccs_distribution_roulette_data_t *d = (_ccs_distribution_roulette_data_t *)data;

	gsl_rng *grng;
	ccs_error_t err = ccs_rng_get_gsl_rng(rng, &grng);
	if (err)
		return err;

	for (size_t i = 0; i < num_values; i++) {
		ccs_float_t rnd = gsl_rng_uniform(grng);
		ccs_int_t upper = d->num_areas - 1;
		ccs_int_t lower = 0;
		ccs_int_t index = upper * rnd;
		int found = 0;
		while( !found ) {
			if ( rnd < d->areas[index] ) {
				upper = index - 1;
				index = (lower+upper)/2;
			} else if ( rnd >= d->areas[index+1] ) {
				lower = index + 1;
				index = (lower+upper)/2;
			} else
				found = 1;
		}
		values[i].i = index;
	}
	return CCS_SUCCESS;
}

ccs_error_t
ccs_create_roulette_distribution(size_t              num_areas,
                                 ccs_float_t        *areas,
                                 ccs_distribution_t *distribution_ret) {
	if (!distribution_ret || !areas || !num_areas || num_areas > INT64_MAX)
		return -CCS_INVALID_VALUE;
	ccs_float_t sum = 0.0;

	for(size_t i = 0; i < num_areas; i++) {
		if (areas[i] < 0.0)
			return -CCS_INVALID_VALUE;
		sum += areas[i];
	}
	if (sum == 0.0)
		return -CCS_INVALID_VALUE;
	ccs_float_t inv_sum = 1.0/sum;
	if (isnan(inv_sum) || !isfinite(inv_sum))
		return -CCS_INVALID_VALUE;

	uintptr_t mem = (uintptr_t)calloc(1, sizeof(struct _ccs_distribution_s) + sizeof(_ccs_distribution_roulette_data_t) + sizeof(ccs_float_t)*(num_areas + 1));

	if (!mem)
		return -CCS_ENOMEM;

	ccs_distribution_t distrib = (ccs_distribution_t)mem;
	_ccs_object_init(&(distrib->obj), CCS_DISTRIBUTION, (_ccs_object_ops_t *)&_ccs_distribution_roulette_ops);
	_ccs_distribution_roulette_data_t * distrib_data = (_ccs_distribution_roulette_data_t *)(mem + sizeof(struct _ccs_distribution_s));
	distrib_data->common_data.type         = CCS_ROULETTE;
	distrib_data->common_data.data_type    = CCS_NUM_INTEGER;
	distrib_data->common_data.scale_type   = CCS_LINEAR;
	distrib_data->common_data.quantization = CCSI(0);
	distrib_data->num_areas                = num_areas;
	distrib_data->areas                    = (ccs_float_t *)(mem + sizeof(struct _ccs_distribution_s) + sizeof(_ccs_distribution_roulette_data_t));

	distrib_data->areas[0] = 0.0;
	for(size_t i = 1; i <= num_areas; i++) {
		distrib_data->areas[i] = distrib_data->areas[i-1] + areas[i-1] * inv_sum;
	}
	distrib_data->areas[num_areas] = 1.0;
	if (distrib_data->areas[num_areas] < distrib_data->areas[num_areas-1]) {
		free((void *)mem);
		return -CCS_INVALID_VALUE;
	}
	distrib->data = (_ccs_distribution_data_t *)distrib_data;
	*distribution_ret = distrib;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_roulette_distribution_get_num_areas(ccs_distribution_t  distribution,
                                        size_t             *num_areas_ret) {
	if (!distribution || distribution->obj.type != CCS_DISTRIBUTION)
		return -CCS_INVALID_OBJECT;
	if (!distribution->data || ((_ccs_distribution_common_data_t*)distribution->data)->type != CCS_ROULETTE)
		return -CCS_INVALID_OBJECT;
	if (!num_areas_ret)
		return -CCS_INVALID_VALUE;
	_ccs_distribution_roulette_data_t * data = (_ccs_distribution_roulette_data_t *)distribution->data;
	*num_areas_ret = data->num_areas;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_roulette_distribution_get_areas(ccs_distribution_t  distribution,
                                    size_t              num_areas,
                                    ccs_float_t        *areas) {
	if (!distribution || distribution->obj.type != CCS_DISTRIBUTION)
		return -CCS_INVALID_OBJECT;
	if (!distribution->data || ((_ccs_distribution_common_data_t*)distribution->data)->type != CCS_ROULETTE)
		return -CCS_INVALID_OBJECT;
	if (!areas)
		return -CCS_INVALID_VALUE;
	_ccs_distribution_roulette_data_t * data = (_ccs_distribution_roulette_data_t *)distribution->data;
	if ((ccs_int_t)num_areas != data->num_areas)
		return -CCS_INVALID_VALUE;

	for (size_t i = 0; i < num_areas; i++)
		areas[i] = data->areas[i+1] - data->areas[i];
	return CCS_SUCCESS;
}
