#ifndef _DATUM_UTHASH_H
#define _DATUM_UTHASH_H

#define HASH_NONFATAL_OOM 1
#define HASH_FUNCTION(s,len,hashv) do { (hashv) = _hash_datum((ccs_datum_t *)(s)); } while(0)
#define HASH_KEYCMP(a,b,len) (_datum_cmp((ccs_datum_t *)a, (ccs_datum_t *)b))

#endif //_DATUM_UTHASH_H
