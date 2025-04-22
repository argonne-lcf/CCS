#ifndef _FEATURES_UTHASH_H
#define _FEATURES_UTHASH_H

#ifdef HASH_NONFATAL_OOM
#undef HASH_NONFATAL_OOM
#endif
#ifdef HASH_FUNCTION
#undef HASH_FUNCTION
#endif
#ifdef HASH_KEYCMP
#undef HASH_KEYCMP
#endif
#define HASH_NONFATAL_OOM 1
#define HASH_FUNCTION(s, len, hashv)                                           \
	do {                                                                   \
		(hashv) = _hash_features(*(ccs_features_t *)(s));              \
	} while (0)
#define HASH_KEYCMP(a, b, len) (_features_cmp(*(ccs_features_t *)(a), *(ccs_features_t *)(b)))

#endif //_FEATURES_UTHASH_H
