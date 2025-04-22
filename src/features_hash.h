#ifndef _FEATURES_HASH_H
#define _FEATURES_HASH_H

#include "uthash.h"

static inline unsigned
_hash_features(ccs_features_t feat)
{
	ccs_hash_t res = 0;
	if (!feat)
		return 0;
	_ccs_binding_hash((ccs_binding_t)feat, &res);
	return res;
}

static inline int
_features_cmp(ccs_features_t feat, ccs_features_t other)
{
	int cmp_ret;
	if (feat == other)
		return 0;
	_ccs_binding_cmp((ccs_binding_t)feat, (ccs_binding_t)other, &cmp_ret);
	return cmp_ret;
}

struct _ccs_hash_features_s {
	ccs_features_t features;
	UT_hash_handle hh;
	UT_array      *history;
	UT_array      *optima;
	UT_array      *old_optima;
};

typedef struct _ccs_hash_features_s _ccs_hash_features_t;
#endif //_FEATURES_HASH_H
