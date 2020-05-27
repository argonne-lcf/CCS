#ifndef _DATUMHASH_H
#define _DATUMHASH_H

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

struct _ccs_hash_datum_s {
	ccs_datum_t d;
	UT_hash_handle hh;
};
typedef struct _ccs_hash_datum_s _ccs_hash_datum_t;


#endif //_DATUMHASH_H
