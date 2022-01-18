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

static inline int ccs_is_little_endian(void) {
#ifdef __cplusplus
  uint32_t u = 1;
  char *pc = (char*)&u;
  return pc[0];
#else
  const union { uint32_t u; char c[4]; } one = { 1 };
  return one.c[0];
#endif
}

static inline uint8_t ccs_bswap_uint8(uint8_t x) {
  return x;
}

// Those should be compiled to the bswap instruction when available
static inline uint16_t ccs_bswap_uint16(uint16_t x) {
  return ((( x  & 0xff00u ) >> 8 ) |
          (( x  & 0x00ffu ) << 8 ));
}

static inline uint32_t ccs_bswap_uint32(uint32_t x) {
  return ((( x & 0xff000000u ) >> 24 ) |
          (( x & 0x00ff0000u ) >> 8  ) |
          (( x & 0x0000ff00u ) << 8  ) |
          (( x & 0x000000ffu ) << 24 ));
}

static inline uint64_t ccs_bswap_uint64(uint64_t x) {
  return ((( x & 0xff00000000000000ull ) >> 56 ) |
          (( x & 0x00ff000000000000ull ) >> 40 ) |
          (( x & 0x0000ff0000000000ull ) >> 24 ) |
          (( x & 0x000000ff00000000ull ) >> 8  ) |
          (( x & 0x00000000ff000000ull ) << 8  ) |
          (( x & 0x0000000000ff0000ull ) << 24 ) |
          (( x & 0x000000000000ff00ull ) << 40 ) |
          (( x & 0x00000000000000ffull ) << 56 ));
}

#ifdef __cplusplus
#define CCS_SWAP_CONVERT(TYPE, MAPPED_NAME, MAPPED_TYPE) \
do {                                                     \
  static_assert(sizeof(MAPPED_TYPE) == sizeof(TYPE),     \
    #MAPPED_TYPE " and " #TYPE " size differ");          \
  MAPPED_TYPE m = x;                                     \
  TYPE t;                                                \
  m = ccs_bswap_ ## MAPPED_NAME(m);                      \
  memcpy(&t, &m, sizeof(t));                             \
  return t;                                              \
} while (0)
#else
#define CCS_SWAP_CONVERT(TYPE, MAPPED_NAME, MAPPED_TYPE) \
do {                                                     \
  union { TYPE t; MAPPED_TYPE m; } v = { .m = x };       \
  v.m = ccs_bswap_ ## MAPPED_NAME(v.m);                  \
  return v.t;                                            \
} while (0)
#endif

#ifdef __cplusplus
#define CCS_CONVERT(TYPE, MAPPED_TYPE)               \
do {                                                 \
  static_assert(sizeof(MAPPED_TYPE) == sizeof(TYPE), \
    #MAPPED_TYPE " and " #TYPE " size differ");      \
  MAPPED_TYPE m = x;                                 \
  TYPE t;                                            \
  memcpy(&t, &m, sizeof(t));                         \
  return t;                                          \
} while (0)
#else
#define CCS_CONVERT(TYPE, MAPPED_TYPE)             \
do {                                               \
  union { TYPE t; MAPPED_TYPE m; } v = { .m = x }; \
  return v.t;                                      \
} while (0)
#endif

#ifdef __cplusplus
#define CCS_CONVERT_SWAP(TYPE, MAPPED_NAME, MAPPED_TYPE) \
do {                                                     \
  static_assert(sizeof(MAPPED_TYPE) == sizeof(TYPE),     \
    #MAPPED_TYPE " and " #TYPE " size differ");          \
  TYPE t = x;                                            \
  MAPPED_TYPE m;                                         \
  memcpy(&m, &t, sizeof(m));                             \
  return ccs_bswap_ ## MAPPED_NAME(m);                   \
} while(0)
#else
#define CCS_CONVERT_SWAP(TYPE, MAPPED_NAME, MAPPED_TYPE) \
do {                                                     \
  union { TYPE t; MAPPED_TYPE m; } v = { .t = x };       \
  v.m = ccs_bswap_ ## MAPPED_NAME(v.m);                  \
  return v.m;                                            \
} while(0)
#endif

#define CCS_UNPACKER(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE) \
static inline TYPE ccs_unpack_ ## NAME (MAPPED_TYPE x) {   \
  if (ccs_is_little_endian())                              \
    CCS_CONVERT(TYPE, MAPPED_TYPE);                        \
  else                                                     \
    CCS_SWAP_CONVERT(TYPE, MAPPED_NAME, MAPPED_TYPE);      \
}

#define CCS_PACKER(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE) \
static inline MAPPED_TYPE ccs_pack_ ## NAME (TYPE x) {   \
  if (ccs_is_little_endian())                            \
    CCS_CONVERT(MAPPED_TYPE, TYPE);                      \
  else                                                   \
    CCS_CONVERT_SWAP(TYPE, MAPPED_NAME, MAPPED_TYPE);    \
}

#define CCS_CONVERTER_TYPE(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE) \
  CCS_UNPACKER(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)             \
  CCS_PACKER(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)

#define CCS_CONVERTER(NAME, TYPE, SIZE)                            \
  CCS_CONVERTER_TYPE(NAME, TYPE, uint ## SIZE, uint ## SIZE ## _t)

CCS_CONVERTER(uint8, uint8_t, 8)
CCS_CONVERTER(int8, int8_t, 8)
CCS_CONVERTER(uint16, uint16_t, 16)
CCS_CONVERTER(int16, int16_t, 16)
CCS_CONVERTER(uint32, uint32_t, 32)
CCS_CONVERTER(int32, int32_t, 32)
CCS_CONVERTER(uint64, uint64_t, 64)
CCS_CONVERTER(int64, int64_t, 64)
CCS_CONVERTER(ccs_hash, ccs_hash_t, 32)
CCS_CONVERTER(ccs_bool, ccs_bool_t, 32)
CCS_CONVERTER(ccs_int, ccs_int_t, 64)
CCS_CONVERTER(ccs_float, ccs_float_t, 64)

#endif //_CONFIGSPACE_INTERNAL_H
