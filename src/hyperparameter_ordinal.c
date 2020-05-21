#include "cconfigspace_internal.h"
#include "hyperparameter_internal.h"
#include <string.h>

#define HASH_NONFATAL_OOM 1
#define HASH_FUNCTION(s,len,hashv) (hashv) = _hash_datum((ccs_datum_t *)(s))
#define HASH_KEYCMP(a,b,len) (_datum_cmp((ccs_datum_t *)a, (ccs_datum_t *)b))
#include "uthash.h"

/* BEWARE: ccs_float_t are used as hash keys. In order to recall sucessfully,
 * The *SAME* float must be used.
 * Alternative is o(n) access for floating point values as they would all go
 * in the same bucket. May be the wisest... Switch to find in the possiblie_values list?
 * #define MAXULPDIFF 7 // To define
 * // Could be doing type puning...
 * static inline int _cmp_float(ccs_float_t a, ccs_float_t b) {
 *   int64_t t1, t2, cmp;
 *   memcpy(&t1, &a, sizeof(int64));
 *   memcpy(&t2, &b, sizeof(int64));
 *   if (a == b)
 *     return 0;
 *   if ((t1 < 0) != (t2 < 0)) {
 *     if (t1 < 0)
 *       return -1;
 *     if (t2 < 0)
 *       return 1;
 *   }
 *   cmp = labs(t1-t2);
 *   if (cmp <= MAXULPDIFF)
 *     return 0;
 *   else if (a < b)
 *     return -1;
 *   else
 *     return 1;
 * }
 */

static inline unsigned _hash_datum(ccs_datum_t *d) {
	unsigned h;
	switch(d->type) {
	case CCS_STRING:
		if (d->value.s)
			HASH_JEN(d->value.s, strlen(d->value.s), h);
		else
			HASH_JEN(d, sizeof(ccs_datum_t), h);
		break;
	case CCS_NONE:
		HASH_JEN(&(d->type), sizeof(d->type), h);
		break;
	default:
		HASH_JEN(d, sizeof(ccs_datum_t), h);
	}
	return h;
}

static inline int _datum_cmp(ccs_datum_t *a, ccs_datum_t *b) {
	if (a->type < b->type) {
		return -1;
	} else if (a->type > b->type) {
		return 1;
	} else {
		switch(a->type) {
		case CCS_STRING:
			if (a->value.s == b->value.s)
				return 0;
			else if (!a->value.s)
				return -1;
			else if (!b->value.s)
				return 1;
			else
				return strcmp(a->value.s, b->value.s);
		case CCS_NONE:
			return 0;
			break;
		default:
			return memcmp(&(a->value), &(b->value),  sizeof(ccs_value_t));
		}
	}
}

struct _ccs_hash_datum_s {
	ccs_datum_t d;
	UT_hash_handle hh;
};
typedef struct _ccs_hash_datum_s _ccs_hash_datum_t;

struct _ccs_hyperparameter_ordinal_data_s {
	_ccs_hyperparameter_common_data_t  common_data;
	ccs_int_t                          num_possible_values;
	_ccs_hash_datum_t                 *possible_values;
	_ccs_hash_datum_t                 *hash;
};
typedef struct _ccs_hyperparameter_ordinal_data_s _ccs_hyperparameter_ordinal_data_t;

static ccs_error_t
_ccs_hyperparameter_ordinal_del(ccs_object_t o) {
	ccs_hyperparameter_t d = (ccs_hyperparameter_t)o;
	_ccs_hyperparameter_ordinal_data_t *data = (_ccs_hyperparameter_ordinal_data_t *)(d->data);
	for (ccs_int_t i = 0; i < data->num_possible_values; i++) {
		HASH_DELETE(hh, data->hash, data->possible_values + i);
	}
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_hyperparameter_ordinal_samples(_ccs_hyperparameter_data_t *data,
                                    ccs_distribution_t          distribution,
                                    ccs_rng_t                   rng,
                                    size_t                      num_values,
                                    ccs_datum_t                *values) {
	_ccs_hyperparameter_ordinal_data_t *d =
	    (_ccs_hyperparameter_ordinal_data_t *)data;
	ccs_error_t err;
	ccs_int_t *vs = (ccs_int_t *)values + num_values;
        ccs_bool_t oversampling;
	err = ccs_distribution_check_oversampling(distribution,
	                                          &(d->common_data.interval),
	                                          &oversampling);
	if (err)
		return err;
	err = ccs_distribution_samples(distribution, rng,
	                               num_values, (ccs_numeric_t *)vs);
	if (err)
		return err;
	if (!oversampling) {
		for(size_t i = 0; i < num_values; i++)
			values[i] = d->possible_values[vs[i]].d;
	} else {
		size_t found = 0;
		for(size_t i = 0; i < num_values; i++)
			if (vs[i] >= 0 && vs[i] < d->num_possible_values)
				values[found++] = d->possible_values[vs[i]].d;
		vs = NULL;
		size_t coeff = 2;
		while (found < num_values) {
			size_t buff_sz = (num_values - found)*coeff;
			vs = (ccs_int_t *)malloc(sizeof(ccs_numeric_t)*buff_sz);
			if (!vs)
				return -CCS_ENOMEM;
			err = ccs_distribution_samples(distribution, rng,
			                               buff_sz, (ccs_numeric_t *)vs);
			for(size_t i = 0; i < buff_sz && found < num_values; i++)
				if (vs[i] >= 0 && vs[i] < d->num_possible_values)
					values[found++] = d->possible_values[vs[i]].d;
			coeff <<= 1;
			free(vs);
			if (coeff > 32)
				return -CCS_SAMPLING_UNSUCCESSFUL;
		}
	}
	return CCS_SUCCESS;
}

ccs_error_t
_ccs_hyperparameter_ordinal_get_default_distribution(
		_ccs_hyperparameter_data_t *data,
		ccs_distribution_t         *distribution) {
	_ccs_hyperparameter_ordinal_data_t *d = (_ccs_hyperparameter_ordinal_data_t *)data;
	ccs_interval_t *interval = &(d->common_data.interval);
	return ccs_create_uniform_distribution(interval->type,
	                                       interval->lower, interval->upper,
	                                       CCS_LINEAR, CCSI(0),
	                                       distribution);
}

static _ccs_hyperparameter_ops_t _ccs_hyperparameter_ordinal_ops = {
	{ &_ccs_hyperparameter_ordinal_del },
	&_ccs_hyperparameter_ordinal_samples,
        &_ccs_hyperparameter_ordinal_get_default_distribution
};

ccs_error_t
ccs_ordinal_hyperparameter_compare_values(ccs_hyperparameter_t  hyperparameter,
                                          ccs_datum_t           value1,
                                          ccs_datum_t           value2,
                                          ccs_int_t            *comp_ret) {
	if (unlikely(!hyperparameter || !hyperparameter->data))
		return -CCS_INVALID_OBJECT;
	if (unlikely(!comp_ret))
		return -CCS_INVALID_VALUE;
	_ccs_hyperparameter_ordinal_data_t *d = ((_ccs_hyperparameter_ordinal_data_t *)(hyperparameter->data));
	_ccs_hash_datum_t *p1, *p2;
	HASH_FIND(hh, d->hash, &value1, sizeof(ccs_datum_t), p1);
	HASH_FIND(hh, d->hash, &value2, sizeof(ccs_datum_t), p2);
	if (likely(p1 && p2)) {
		if (p1 < p2)
			*comp_ret = -1;
		else if (p1 > p2)
			*comp_ret =  1;
		else
			*comp_ret =  0;
		return CCS_SUCCESS;
	} else  {
		return -CCS_INVALID_VALUE;
	}
}

#undef uthash_nonfatal_oom
#define uthash_nonfatal_oom(elt) { \
	_ccs_hash_datum_t *v = (_ccs_hash_datum_t *)elt; \
	while ( v > pvs) { \
		HASH_DELETE(hh, hyperparam_data->hash, --v); \
	} \
	free((void*)mem); \
	return -CCS_ENOMEM; \
}

ccs_error_t
ccs_create_ordinal_hyperparameter(const char           *name,
                                  size_t                num_possible_values,
                                  ccs_datum_t          *possible_values,
                                  size_t                default_value_index,
                                  void                 *user_data,
                                  ccs_hyperparameter_t *hyperparameter_ret) {
	if (!hyperparameter_ret || !name)
		return -CCS_INVALID_VALUE;
	if (!num_possible_values ||
	     num_possible_values > INT64_MAX ||
             num_possible_values <= default_value_index)
		return -CCS_INVALID_VALUE;
	if (!possible_values)
		return -CCS_INVALID_VALUE;
	size_t size_strs = 0;
	for(size_t i = 0; i < num_possible_values; i++)
		if (possible_values[i].type == CCS_STRING) {
			if (!possible_values[i].value.s)
				return -CCS_INVALID_VALUE;
			size_strs += strlen(possible_values[i].value.s) + 1;
		}

	uintptr_t mem = (uintptr_t)calloc(1,
	    sizeof(struct _ccs_hyperparameter_s) +
	    sizeof(_ccs_hyperparameter_ordinal_data_t) +
	    sizeof(_ccs_hash_datum_t) * num_possible_values +
	    strlen(name) + 1 +
	    size_strs);
	if (!mem)
		return -CCS_ENOMEM;

	ccs_interval_t interval;
	interval.type = CCS_NUM_INTEGER;
	interval.lower.i = 0;
	interval.upper.i = (ccs_int_t)num_possible_values;
	interval.lower_included = CCS_TRUE;
	interval.upper_included = CCS_FALSE;

	ccs_hyperparameter_t hyperparam = (ccs_hyperparameter_t)mem;
	_ccs_object_init(&(hyperparam->obj), CCS_HYPERPARAMETER, (_ccs_object_ops_t *)&_ccs_hyperparameter_ordinal_ops);
	_ccs_hyperparameter_ordinal_data_t *hyperparam_data =
	    (_ccs_hyperparameter_ordinal_data_t *)(mem +
	        sizeof(struct _ccs_hyperparameter_s));
	hyperparam_data->common_data.type = CCS_ORDINAL;
	hyperparam_data->common_data.name = (char *)(mem +
	    sizeof(struct _ccs_hyperparameter_s) +
	    sizeof(_ccs_hyperparameter_ordinal_data_t) +
	    sizeof(_ccs_hash_datum_t) * num_possible_values);
	strcpy((char *)hyperparam_data->common_data.name, name);
	hyperparam_data->common_data.user_data = user_data;
	hyperparam_data->common_data.default_value = possible_values[default_value_index];
	hyperparam_data->common_data.interval = interval;
	hyperparam_data->num_possible_values = num_possible_values;
	_ccs_hash_datum_t *pvs = (_ccs_hash_datum_t *)(mem +
	    sizeof(struct _ccs_hyperparameter_s) +
	    sizeof(_ccs_hyperparameter_ordinal_data_t));
	hyperparam_data->possible_values = pvs;
	hyperparam_data->hash = NULL;

	char *str_pool = (char *)(hyperparam_data->common_data.name) + strlen(name) + 1;
	for (size_t i = 0; i < num_possible_values; i++) {
		_ccs_hash_datum_t *p = NULL;
		HASH_FIND(hh, hyperparam_data->hash, possible_values + i, sizeof(ccs_datum_t), p);
		if (p) {
			_ccs_hash_datum_t *tmp;
			HASH_ITER(hh, hyperparam_data->hash, p, tmp) {
				HASH_DELETE(hh, hyperparam_data->hash, p);
			}
			free((void *)mem);
			return -CCS_INVALID_VALUE;
		}
		if (possible_values[i].type == CCS_STRING) {
			pvs[i].d.type = CCS_STRING;
			pvs[i].d.value.s = str_pool;
			strcpy(str_pool, possible_values[i].value.s);
			str_pool += strlen(possible_values[i].value.s) + 1;
		} else {
			pvs[i].d = possible_values[i];
		}
		HASH_ADD(hh, hyperparam_data->hash, d, sizeof(ccs_datum_t), pvs + i);
	}
	hyperparam->data = (_ccs_hyperparameter_data_t *)hyperparam_data;
	*hyperparameter_ret = hyperparam;
	return CCS_SUCCESS;
}