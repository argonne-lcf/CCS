#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <math.h>
#include "cconfigspace_internal.h"
#include "distribution_internal.h"

struct _ccs_distribution_roulette_data_s {
	_ccs_distribution_common_data_t  common_data;
	size_t                           num_areas;
	ccs_float_t                     *areas;
};
typedef struct _ccs_distribution_roulette_data_s _ccs_distribution_roulette_data_t;

static ccs_result_t
_ccs_distribution_del(ccs_object_t o) {
	(void)o;
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_distribution_roulette_get_bounds(_ccs_distribution_data_t *data,
                                      ccs_interval_t           *interval_ret);

static ccs_result_t
_ccs_distribution_roulette_samples(_ccs_distribution_data_t *data,
                                   ccs_rng_t                 rng,
                                   size_t                    num_values,
                                   ccs_numeric_t            *values);

static ccs_result_t
_ccs_distribution_roulette_strided_samples(_ccs_distribution_data_t *data,
                                           ccs_rng_t                 rng,
                                           size_t                    num_values,
                                           size_t                    stride,
                                           ccs_numeric_t            *values);

static ccs_result_t
_ccs_distribution_roulette_soa_samples(_ccs_distribution_data_t  *data,
                                       ccs_rng_t                  rng,
                                       size_t                     num_values,
                                       ccs_numeric_t            **values);

static _ccs_distribution_ops_t _ccs_distribution_roulette_ops = {
	{ &_ccs_distribution_del },
	&_ccs_distribution_roulette_samples,
	&_ccs_distribution_roulette_get_bounds,
	&_ccs_distribution_roulette_strided_samples,
	&_ccs_distribution_roulette_soa_samples
};

static ccs_result_t
_ccs_distribution_roulette_get_bounds(_ccs_distribution_data_t *data,
                                      ccs_interval_t           *interval_ret) {
	_ccs_distribution_roulette_data_t *d = (_ccs_distribution_roulette_data_t *)data;

	interval_ret->type = CCS_NUM_INTEGER;
	interval_ret->lower = CCSI(INT64_C(0));
	interval_ret->upper = CCSI((ccs_int_t)(d->num_areas));
	interval_ret->lower_included = CCS_TRUE;
	interval_ret->upper_included = CCS_FALSE;
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_distribution_roulette_samples(_ccs_distribution_data_t *data,
                                   ccs_rng_t                 rng,
                                   size_t                    num_values,
                                   ccs_numeric_t            *values) {
	_ccs_distribution_roulette_data_t *d = (_ccs_distribution_roulette_data_t *)data;

	gsl_rng *grng;
	CCS_VALIDATE(ccs_rng_get_gsl_rng(rng, &grng));

	for (size_t i = 0; i < num_values; i++) {
		ccs_float_t rnd = gsl_rng_uniform(grng);
		ccs_int_t index = ccs_dichotomic_search(d->num_areas, d->areas, rnd);
		values[i].i = index;
	}
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_distribution_roulette_strided_samples(_ccs_distribution_data_t *data,
                                           ccs_rng_t                 rng,
                                           size_t                    num_values,
                                           size_t                    stride,
                                           ccs_numeric_t            *values) {
	_ccs_distribution_roulette_data_t *d = (_ccs_distribution_roulette_data_t *)data;

	gsl_rng *grng;
	CCS_VALIDATE(ccs_rng_get_gsl_rng(rng, &grng));

	for (size_t i = 0; i < num_values; i++) {
		ccs_float_t rnd = gsl_rng_uniform(grng);
		ccs_int_t index = ccs_dichotomic_search(d->num_areas, d->areas, rnd);
		values[i*stride].i = index;
	}
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_distribution_roulette_soa_samples(_ccs_distribution_data_t  *data,
                                     ccs_rng_t                  rng,
                                     size_t                     num_values,
                                     ccs_numeric_t            **values) {
	if (*values)
		return _ccs_distribution_roulette_samples(data, rng, num_values, *values);
	return CCS_SUCCESS;
}

ccs_result_t
ccs_create_roulette_distribution(size_t              num_areas,
                                 ccs_float_t        *areas,
                                 ccs_distribution_t *distribution_ret) {
	CCS_CHECK_ARY(num_areas, areas);
	CCS_CHECK_PTR(distribution_ret);
	if (!num_areas || num_areas > INT64_MAX)
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

	uintptr_t mem = (uintptr_t)calloc(1, sizeof(struct _ccs_distribution_s) + sizeof(_ccs_distribution_roulette_data_t) + sizeof(ccs_float_t)*(num_areas + 1) + sizeof(ccs_numeric_type_t));

	if (!mem)
		return -CCS_OUT_OF_MEMORY;

	ccs_distribution_t distrib = (ccs_distribution_t)mem;
	_ccs_object_init(&(distrib->obj), CCS_DISTRIBUTION, (_ccs_object_ops_t *)&_ccs_distribution_roulette_ops);
	_ccs_distribution_roulette_data_t * distrib_data = (_ccs_distribution_roulette_data_t *)(mem + sizeof(struct _ccs_distribution_s));
	distrib_data->common_data.data_types    = (ccs_numeric_type_t *)(mem + sizeof(struct _ccs_distribution_s) + sizeof(_ccs_distribution_roulette_data_t) + sizeof(ccs_float_t)*(num_areas + 1));
	distrib_data->common_data.type          = CCS_ROULETTE;
	distrib_data->common_data.dimension     = 1;
	distrib_data->common_data.data_types[0] = CCS_NUM_INTEGER;
	distrib_data->num_areas                 = num_areas;
	distrib_data->areas                     = (ccs_float_t *)(mem + sizeof(struct _ccs_distribution_s) + sizeof(_ccs_distribution_roulette_data_t));

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

ccs_result_t
ccs_roulette_distribution_get_num_areas(ccs_distribution_t  distribution,
                                        size_t             *num_areas_ret) {
	CCS_CHECK_DISTRIBUTION(distribution, CCS_ROULETTE);
	CCS_CHECK_PTR(num_areas_ret);
	_ccs_distribution_roulette_data_t * data = (_ccs_distribution_roulette_data_t *)distribution->data;
	*num_areas_ret = data->num_areas;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_roulette_distribution_get_areas(ccs_distribution_t  distribution,
                                    size_t              num_areas,
                                    ccs_float_t        *areas,
                                    size_t             *num_areas_ret) {
	CCS_CHECK_DISTRIBUTION(distribution, CCS_ROULETTE);
	CCS_CHECK_ARY(num_areas, areas);
	if (!areas && !num_areas_ret)
		return -CCS_INVALID_VALUE;
	_ccs_distribution_roulette_data_t * data = (_ccs_distribution_roulette_data_t *)distribution->data;
	if (areas) {
		if (num_areas < data->num_areas)
			return -CCS_INVALID_VALUE;
		for (size_t i = 0; i < data->num_areas; i++)
			areas[i] = data->areas[i+1] - data->areas[i];
		for (size_t i = data->num_areas; i < num_areas; i++)
			areas[i] = 0.0;
	}
	if (num_areas_ret)
		*num_areas_ret = data->num_areas;
	return CCS_SUCCESS;
}
