#ifndef _CONFIGSPACE_INTERNAL_H
#define _CONFIGSPACE_INTERNAL_H

#include <cconfigspace.h>
#include <stdarg.h>
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

/**
 * The statement x is likely evaluating to true.
 */
#define CCS_LIKELY(x)      __builtin_expect(!!(x), 1)
/**
 * The statement x is likely evaluating to false.
 */
#define CCS_UNLIKELY(x)    __builtin_expect(!!(x), 0)

#define CCS_RICH_ERRORS 1

#if CCS_RICH_ERRORS
#define CCS_ADD_STACK_ELEM() do { \
	ccs_thread_error_stack_push(__FILE__, __LINE__, __func__); \
} while (0)
#else
#define CCS_ADD_STACK_ELEM() do { \
} while (0)
#endif

#if CCS_RICH_ERRORS
#define CCS_CREATE_ERROR(error, ...) do { \
	ccs_create_thread_error(error, __VA_ARGS__); \
	CCS_ADD_STACK_ELEM(); \
} while (0)
#else
#define CCS_CREATE_ERROR(error, ...) do { \
} while (0)
#endif

#define CCS_RAISE(error, ...) do { \
	CCS_CREATE_ERROR(error, __VA_ARGS__); \
	return error; \
} while (0)

#define CCS_RAISE_ERR_GOTO(err, error, label, ...) do { \
	CCS_CREATE_ERROR(error, __VA_ARGS__); \
	err = error; \
	goto label; \
} while (0)

#define CCS_REFUTE_MSG(cond, error, ...) do { \
	if (CCS_UNLIKELY(cond)) \
		CCS_RAISE(error, __VA_ARGS__); \
} while (0)

#define CCS_REFUTE_MSG_ERR_GOTO(err, cond, error, label, ...) do { \
	if (CCS_UNLIKELY(cond)) \
		CCS_RAISE_ERR_GOTO(err, error, label, __VA_ARGS__); \
} while(0)

#define CCS_CHECK_OBJ(o, t) CCS_REFUTE_MSG(!(o) || \
		!((_ccs_object_template_t *)(o))->data || \
		((_ccs_object_template_t *)(o))->obj.type != (t), \
	CCS_INVALID_OBJECT, "Invalid CCS object '%s' == %p supplied, expected %s", #o, o, #t)

#define CCS_CHECK_PTR(p) CCS_REFUTE_MSG(!(p), CCS_INVALID_VALUE, "NULL pointer supplied '%s'", #p);

#define CCS_CHECK_ARY(c, a) CCS_REFUTE_MSG((c > 0) && !(a), CCS_INVALID_VALUE, "Invalid array '%s' == %p of size '%s' == %zu supplied", #a, a, #c, c)

#define CCS_REFUTE(cond, error) CCS_REFUTE_MSG(cond, error, "%s: Error condition '%s' was verified", #error, #cond)

#define CCS_REFUTE_ERR_GOTO(err, cond, error, label) CCS_REFUTE_MSG_ERR_GOTO(err, cond, error, label, "%s: Error condition '%s' was verified", #error, #cond)

#define CCS_VALIDATE_ERR_GOTO(err, cmd, label) do { \
	err = (cmd); \
	if (CCS_UNLIKELY(err < CCS_SUCCESS)) {\
		CCS_ADD_STACK_ELEM(); \
		goto label; \
	} \
} while (0)

#define CCS_VALIDATE_ERR(err, cmd) do { \
	err = (cmd); \
	if (CCS_UNLIKELY(err < CCS_SUCCESS)) { \
		CCS_ADD_STACK_ELEM(); \
		return err; \
	} \
} while (0)

#define CCS_VALIDATE(cmd) do { \
	ccs_error_t _err; \
	CCS_VALIDATE_ERR(_err, cmd); \
} while(0)

#define CCS_SERIALIZATION_API_VERSION_TYPE uint32_t
#define CCS_SERIALIZATION_API_VERSION \
	((CCS_SERIALIZATION_API_VERSION_TYPE)1)
#define CCS_SERIALIZATION_API_VERSION_SERIALIZE_BIN _ccs_serialize_bin_uint32
#define CCS_SERIALIZATION_API_VERSION_SERIALIZE_SIZE_BIN _ccs_serialize_bin_size_uint32
#define CCS_SERIALIZATION_API_VERSION_DESERIALIZE_BIN _ccs_deserialize_bin_uint32

typedef void *ccs_user_data_t;

/* "CCS" */
#define CCS_MAGIC_TAG { 0x43, 0x43, 0x53, 0x00 }
static const char _ccs_magic_tag[4] = CCS_MAGIC_TAG;

static inline ccs_error_t
_ccs_serialize_bin_magic_tag(
		const char  *tag,
		size_t      *buffer_size,
		char       **buffer) {
	CCS_REFUTE(*buffer_size < 4, CCS_NOT_ENOUGH_DATA);
	memcpy(*buffer, tag, 4);
	*buffer += 4;
	*buffer_size -= 4;
	return CCS_SUCCESS;
}

static inline size_t
_ccs_serialize_bin_size_magic_tag(
		const char  *tag) {
	return strlen(tag) + 1;
}

static inline ccs_error_t
_ccs_deserialize_bin_magic_tag(
		char        *tag,
		size_t      *buffer_size,
		const char **buffer) {
	CCS_REFUTE(*buffer_size < 4, CCS_NOT_ENOUGH_DATA);
	memcpy(tag, *buffer, 4);
	*buffer += 4;
	*buffer_size -= 4;
	return CCS_SUCCESS;
}

struct _ccs_file_descriptor_state_s {
	char     *base;
	size_t    base_size;
	char     *buffer;
	size_t    buffer_size;
	int       fd;
	uint32_t  version;
};
typedef struct _ccs_file_descriptor_state_s _ccs_file_descriptor_state_t;

struct _ccs_object_serialize_options_s {
	_ccs_file_descriptor_state_t    **ppfd_state;
	ccs_object_serialize_callback_t   serialize_callback;
	void                             *serialize_user_data;
};
typedef struct _ccs_object_serialize_options_s _ccs_object_serialize_options_t;

struct _ccs_object_ops_s {
	ccs_error_t (*del)(ccs_object_t object);

	ccs_error_t (*serialize_size)(
		ccs_object_t                     object,
		ccs_serialize_format_t           format,
		size_t                          *cum_size,
		_ccs_object_serialize_options_t *opts);

	ccs_error_t (*serialize)(
		ccs_object_t                      object,
		ccs_serialize_format_t            format,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts);
};

typedef struct _ccs_object_ops_s _ccs_object_ops_t;

struct _ccs_object_callback_s {
	ccs_object_release_callback_t  callback;
	void                          *user_data;
};
typedef struct _ccs_object_callback_s _ccs_object_callback_t;

struct _ccs_object_internal_s {
	ccs_object_type_t                type;
	int32_t                          refcount;
	void                            *user_data;
	UT_array                        *callbacks;
	_ccs_object_ops_t               *ops;
	ccs_object_serialize_callback_t  serialize_callback;
	void                            *serialize_user_data;
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
	o->user_data = NULL;
	o->callbacks = NULL;
	o->ops = ops;
	o->serialize_callback = NULL;
	o->serialize_user_data = NULL;
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

// https://stackoverflow.com/a/48924178
static inline uint8_t ccs_zigzag_encode_int8(int8_t x) {
  return ( ( uint8_t ) x << 1 ) ^ -( ( uint8_t ) x >> 7 );
}

static inline uint16_t ccs_zigzag_encode_int16(int16_t x) {
  return ( ( uint16_t ) x << 1 ) ^ -( ( uint16_t ) x >> 15 );
}

static inline uint32_t ccs_zigzag_encode_int32(int32_t x) {
  return ( ( uint32_t ) x << 1 ) ^ -( ( uint32_t ) x >> 31 );
}

static inline uint64_t ccs_zigzag_encode_int64(int64_t x) {
  return ( ( uint64_t ) x << 1 ) ^ -( ( uint64_t ) x >> 63 );
}

static inline int8_t ccs_zigzag_decode_int8(uint8_t x) {
  return ( int8_t ) ( ( x >> 1 ) ^ -( x & 0x1 ) );
}

static inline int16_t ccs_zigzag_decode_int16(uint16_t x) {
  return ( int16_t ) ( ( x >> 1 ) ^ -( x & 0x1 ) );
}

static inline int32_t ccs_zigzag_decode_int32(uint32_t x) {
  return ( int32_t ) ( ( x >> 1 ) ^ -( x & 0x1 ) );
}

static inline int64_t ccs_zigzag_decode_int64(uint64_t x) {
  return ( int64_t ) ( ( x >> 1 ) ^ -( x & 0x1 ) );
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
static inline TYPE _ccs_unpack_ ## NAME (MAPPED_TYPE x) {  \
  if (ccs_is_little_endian())                              \
    CCS_CONVERT(TYPE, MAPPED_TYPE);                        \
  else                                                     \
    CCS_SWAP_CONVERT(TYPE, MAPPED_NAME, MAPPED_TYPE);      \
}

#define CCS_PACKER(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE) \
static inline MAPPED_TYPE _ccs_pack_ ## NAME (TYPE x) {  \
  if (ccs_is_little_endian())                            \
    CCS_CONVERT(MAPPED_TYPE, TYPE);                      \
  else                                                   \
    CCS_CONVERT_SWAP(TYPE, MAPPED_NAME, MAPPED_TYPE);    \
}

// This serializer works on unsigned types
// It uses https://en.wikipedia.org/wiki/LEB128
#define CCS_COMPRESSED_SERIALIZER(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)    \
static inline ccs_error_t                                                 \
_ccs_serialize_bin_ ## NAME (TYPE x, size_t *buffer_size, char **buffer) { \
  size_t buff_size = *buffer_size;                                         \
  uint8_t *buff = (uint8_t *)*buffer;                                      \
  MAPPED_TYPE v = (MAPPED_TYPE)x;                                          \
  do {                                                                     \
    CCS_REFUTE(buff_size < 1, CCS_NOT_ENOUGH_DATA);                        \
    uint8_t y = v & 0x7f;                                                  \
    v >>= 7;                                                               \
    if (v) y |= 0x80;                                                      \
    *buff = y;                                                             \
    buff_size -= 1;                                                        \
    buff += 1;                                                             \
  } while (v);                                                             \
  *buffer_size = buff_size;                                                \
  *buffer = (char *)buff;                                                  \
  return CCS_SUCCESS;                                                      \
}                                                                          \
static inline size_t                                                       \
_ccs_serialize_bin_size_ ## NAME (TYPE x) {                                \
  size_t sz = 0;                                                           \
  MAPPED_TYPE v = (MAPPED_TYPE)x;                                          \
  do {                                                                     \
    sz += 1;                                                               \
    v >>= 7;                                                               \
  } while(v);                                                              \
  return sz;                                                               \
}

#define CCS_COMPRESSED_DESERIALIZER(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)           \
static inline ccs_error_t                                                          \
_ccs_deserialize_bin_ ## NAME (TYPE *x, size_t *buffer_size, const char **buffer) { \
  size_t buff_size = *buffer_size;                                                  \
  const uint8_t *buff = (const uint8_t *)*buffer;                                   \
  MAPPED_TYPE v = 0;                                                                \
  MAPPED_TYPE shift = 0;                                                            \
  MAPPED_TYPE y;                                                                    \
  do {                                                                              \
    CCS_REFUTE(buff_size < 1, CCS_NOT_ENOUGH_DATA);                                 \
    y = *buff;                                                                      \
    v |= (y & 0x7f) << shift;                                                       \
    buff_size -= 1;                                                                 \
    buff += 1;                                                                      \
    shift += 7;                                                                     \
  } while(y & 0x80);                                                                \
  *x = (TYPE)v;                                                                     \
  *buffer_size = buff_size;                                                         \
  *buffer = (const char *)buff;                                                     \
  return CCS_SUCCESS;                                                               \
}                                                                                   \
static inline ccs_error_t                                                          \
_ccs_peek_bin_ ## NAME (TYPE *x, size_t *buffer_size, const char **buffer) {        \
  size_t buff_size = *buffer_size;                                                  \
  const char *buff = *buffer;                                                       \
  return _ccs_deserialize_bin_ ## NAME (x, &buff_size, &buff);                      \
}

#define CCS_COMPRESSED_SERIALIZER_SIGNED(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE) \
static inline ccs_error_t                                                     \
_ccs_serialize_bin_ ## NAME (TYPE x, size_t *buffer_size, char **buffer) {     \
  return _ccs_serialize_bin_u ## MAPPED_NAME (                                 \
    ccs_zigzag_encode_ ## MAPPED_NAME (x), buffer_size, buffer);               \
}                                                                              \
static inline size_t                                                           \
_ccs_serialize_bin_size_ ## NAME (TYPE x) {                                    \
  return _ccs_serialize_bin_size_u ## MAPPED_NAME (                            \
    ccs_zigzag_encode_ ## MAPPED_NAME (x));                                    \
}

#define CCS_COMPRESSED_DESERIALIZER_SIGNED(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)    \
static inline ccs_error_t                                                          \
_ccs_deserialize_bin_ ## NAME (TYPE *x, size_t *buffer_size, const char **buffer) { \
  u ## MAPPED_TYPE v;                                                               \
  CCS_VALIDATE(_ccs_deserialize_bin_u ## MAPPED_NAME (&v, buffer_size, buffer));    \
  *x = ccs_zigzag_decode_ ## MAPPED_NAME (v);                                       \
  return CCS_SUCCESS;                                                               \
}                                                                                   \
static inline ccs_error_t                                                          \
_ccs_peek_bin_ ## NAME (TYPE *x, size_t *buffer_size, const char **buffer) {        \
  size_t buff_size = *buffer_size;                                                  \
  const char *buff = *buffer;                                                       \
  return _ccs_deserialize_bin_ ## NAME (x, &buff_size, &buff);                      \
}

#define CCS_COMPRESSED_SERIALIZER_POINTER(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)  \
static inline ccs_error_t                                                       \
_ccs_serialize_bin_ ## NAME (TYPE x, size_t *buffer_size, char **buffer) {       \
  return _ccs_serialize_bin_uint64((uint64_t)(uintptr_t)x, buffer_size, buffer); \
}                                                                                \
static inline size_t                                                             \
_ccs_serialize_bin_size_ ## NAME (TYPE x) {                                      \
  return _ccs_serialize_bin_size_uint64((uint64_t)(uintptr_t)x);                 \
}   

#define CCS_COMPRESSED_DESERIALIZER_POINTER(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)   \
static inline ccs_error_t                                                          \
_ccs_deserialize_bin_ ## NAME (TYPE *x, size_t *buffer_size, const char **buffer) { \
  uint64_t v;                                                                       \
  CCS_VALIDATE(_ccs_deserialize_bin_uint64(&v, buffer_size, buffer));               \
  *x = (TYPE)(MAPPED_TYPE)v;                                                        \
  return CCS_SUCCESS;                                                               \
}                                                                                   \
static inline ccs_error_t                                                          \
_ccs_peek_bin_ ## NAME (TYPE *x, size_t *buffer_size, const char **buffer) {        \
  size_t buff_size = *buffer_size;                                                  \
  const char *buff = *buffer;                                                       \
  return _ccs_deserialize_bin_ ## NAME (x, &buff_size, &buff);                      \
}

#define CCS_SERIALIZER(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)               \
static inline ccs_error_t                                                 \
_ccs_serialize_bin_ ## NAME (TYPE x, size_t *buffer_size, char **buffer) { \
  CCS_REFUTE(*buffer_size < sizeof(MAPPED_TYPE), CCS_NOT_ENOUGH_DATA);     \
  MAPPED_TYPE v = _ccs_pack_ ## NAME (x);                                  \
  memcpy(*buffer, &v, sizeof(MAPPED_TYPE));                                \
  *buffer_size -= sizeof(MAPPED_TYPE);                                     \
  *buffer += sizeof(MAPPED_TYPE);                                          \
  return CCS_SUCCESS;                                                      \
}                                                                          \
static inline size_t                                                       \
_ccs_serialize_bin_size_ ## NAME (TYPE x) {                                \
  (void)x;                                                                 \
  return sizeof(MAPPED_TYPE);                                              \
}

#define CCS_DESERIALIZER(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)                      \
static inline ccs_error_t                                                          \
_ccs_peek_bin_ ## NAME (TYPE *x, size_t *buffer_size, const char **buffer) {        \
  CCS_REFUTE(*buffer_size < sizeof(MAPPED_TYPE), CCS_NOT_ENOUGH_DATA);              \
  MAPPED_TYPE v;                                                                    \
  memcpy(&v, *buffer, sizeof(MAPPED_TYPE));                                         \
  *x = _ccs_unpack_ ## NAME (v);                                                    \
  return CCS_SUCCESS;                                                               \
}                                                                                   \
static inline ccs_error_t                                                          \
_ccs_deserialize_bin_ ## NAME (TYPE *x, size_t *buffer_size, const char **buffer) { \
  CCS_VALIDATE(_ccs_peek_bin_ ## NAME (x, buffer_size, buffer));                    \
  *buffer_size -= sizeof(MAPPED_TYPE);                                              \
  *buffer += sizeof(MAPPED_TYPE);                                                   \
  return CCS_SUCCESS;                                                               \
}

#define CCS_CONVERTER_TYPE(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE) \
  CCS_UNPACKER(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)             \
  CCS_PACKER(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)               \
  CCS_SERIALIZER(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)           \
  CCS_DESERIALIZER(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)

#define CCS_CONVERTER(NAME, TYPE, SIZE)                            \
  CCS_CONVERTER_TYPE(NAME, TYPE, uint ## SIZE, uint ## SIZE ## _t)

#define CCS_CONVERTER_UNCOMPRESSED(NAME, TYPE, SIZE)               \
  CCS_CONVERTER_TYPE(uncompressed_ ## NAME, TYPE, uint ## SIZE, uint ## SIZE ## _t)

#define CCS_CONVERTER_COMPRESSED_TYPE(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE) \
  CCS_COMPRESSED_SERIALIZER(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)           \
  CCS_COMPRESSED_DESERIALIZER(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)

#define CCS_CONVERTER_COMPRESSED(NAME, TYPE, SIZE)                            \
  CCS_CONVERTER_COMPRESSED_TYPE(NAME, TYPE, uint ## SIZE, uint ## SIZE ## _t)

#define CCS_CONVERTER_COMPRESSED_SIGNED_TYPE(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE) \
  CCS_COMPRESSED_SERIALIZER_SIGNED(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)           \
  CCS_COMPRESSED_DESERIALIZER_SIGNED(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)

#define CCS_CONVERTER_COMPRESSED_SIGNED(NAME, TYPE, SIZE) \
  CCS_CONVERTER_COMPRESSED_SIGNED_TYPE(NAME, TYPE, int ## SIZE, int ## SIZE ## _t)

#define CCS_CONVERTER_COMPRESSED_POINTER_TYPE(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE) \
  CCS_COMPRESSED_SERIALIZER_POINTER(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)           \
  CCS_COMPRESSED_DESERIALIZER_POINTER(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)

#define CCS_CONVERTER_COMPRESSED_POINTER(NAME, TYPE) \
  CCS_CONVERTER_COMPRESSED_POINTER_TYPE(NAME, TYPE, uintptr, uintptr_t)

#define CCS_USE_COMPRESSED
#ifndef CCS_USE_COMPRESSED
CCS_CONVERTER(uint8, uint8_t, 8)
CCS_CONVERTER(int8, int8_t, 8)
CCS_CONVERTER(uint16, uint16_t, 16)
CCS_CONVERTER(int16, int16_t, 16)
CCS_CONVERTER(uint32, uint32_t, 32)
CCS_CONVERTER(int32, int32_t, 32)
CCS_CONVERTER(uint64, uint64_t, 64)
CCS_CONVERTER_UNCOMPRESSED(uint64, uint64_t, 64)
CCS_CONVERTER(int64, int64_t, 64)
CCS_CONVERTER(ccs_hash, ccs_hash_t, 32)
CCS_CONVERTER(ccs_bool, ccs_bool_t, 32)
CCS_CONVERTER(ccs_int, ccs_int_t, 64)
CCS_CONVERTER(ccs_float, ccs_float_t, 64)
CCS_CONVERTER(ccs_numeric_type, ccs_numeric_type_t, 32)
CCS_CONVERTER(ccs_hyperparameter_type, ccs_hyperparameter_type_t, 32)
CCS_CONVERTER(ccs_datum_flags, ccs_datum_flags_t, 32)
CCS_CONVERTER(ccs_data_type, ccs_data_type_t, 32)
CCS_CONVERTER(ccs_object_type, ccs_object_type_t, 32)
CCS_CONVERTER(ccs_scale_type, ccs_scale_type_t, 32)
CCS_CONVERTER(ccs_distribution_type, ccs_distribution_type_t, 32)
CCS_CONVERTER(ccs_expression_type, ccs_expression_type_t, 32)
CCS_CONVERTER(ccs_objective_type, ccs_objective_type_t, 32)
CCS_CONVERTER(ccs_tuner_type, ccs_tuner_type_t, 32)
CCS_CONVERTER(ccs_features_tuner_type, ccs_features_tuner_type_t, 32)
CCS_CONVERTER(ccs_tree_type, ccs_tree_type_t, 32)
CCS_CONVERTER(ccs_tree_space_type, ccs_tree_space_type_t, 32)
CCS_CONVERTER(ccs_result, ccs_result_t, 32)
CCS_CONVERTER(ccs_object, ccs_object_t, 64)
#else
CCS_CONVERTER(uint8, uint8_t, 8)
CCS_CONVERTER(int8, int8_t, 8)
CCS_CONVERTER_COMPRESSED(uint16, uint16_t, 16)
CCS_CONVERTER_COMPRESSED_SIGNED(int16, int16_t, 16)
CCS_CONVERTER_COMPRESSED(uint32, uint32_t, 32)
CCS_CONVERTER_COMPRESSED_SIGNED(int32, int32_t, 32)
CCS_CONVERTER_COMPRESSED(uint64, uint64_t, 64)
CCS_CONVERTER_UNCOMPRESSED(uint64, uint64_t, 64)
CCS_CONVERTER_COMPRESSED_SIGNED(int64, int64_t, 64)
CCS_CONVERTER(ccs_hash, ccs_hash_t, 32)
CCS_CONVERTER_COMPRESSED(ccs_bool, ccs_bool_t, 32)
CCS_CONVERTER_COMPRESSED_SIGNED(ccs_int, ccs_int_t, 64)
CCS_CONVERTER(ccs_float, ccs_float_t, 64)
CCS_CONVERTER_COMPRESSED(ccs_numeric_type, ccs_numeric_type_t, 32)
CCS_CONVERTER_COMPRESSED(ccs_hyperparameter_type, ccs_hyperparameter_type_t, 32)
CCS_CONVERTER_COMPRESSED(ccs_datum_flags, ccs_datum_flags_t, 32)
CCS_CONVERTER_COMPRESSED(ccs_data_type, ccs_data_type_t, 32)
CCS_CONVERTER_COMPRESSED(ccs_object_type, ccs_object_type_t, 32)
CCS_CONVERTER_COMPRESSED(ccs_scale_type, ccs_scale_type_t, 32)
CCS_CONVERTER_COMPRESSED(ccs_distribution_type, ccs_distribution_type_t, 32)
CCS_CONVERTER_COMPRESSED(ccs_expression_type, ccs_expression_type_t, 32)
CCS_CONVERTER_COMPRESSED(ccs_objective_type, ccs_objective_type_t, 32)
CCS_CONVERTER_COMPRESSED(ccs_tuner_type, ccs_tuner_type_t, 32)
CCS_CONVERTER_COMPRESSED(ccs_features_tuner_type, ccs_features_tuner_type_t, 32)
CCS_CONVERTER_COMPRESSED(ccs_tree_type, ccs_tree_type_t, 32)
CCS_CONVERTER_COMPRESSED(ccs_tree_space_type, ccs_tree_space_type_t, 32)
CCS_CONVERTER_COMPRESSED_SIGNED(ccs_result, ccs_result_t, 32)
CCS_CONVERTER_COMPRESSED_POINTER(ccs_object, ccs_object_t)
#endif

static inline size_t
_ccs_serialize_bin_size_size(size_t sz) {
	return _ccs_serialize_bin_size_uint64(sz);
}

static inline ccs_error_t
_ccs_serialize_bin_size(
		size_t   sz,
		size_t  *buffer_size,
		char   **buffer) {
	CCS_VALIDATE(_ccs_serialize_bin_uint64(sz, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_deserialize_bin_size(
		size_t      *sz,
		size_t      *buffer_size,
		const char **buffer) {
	uint64_t tmp;
	CCS_VALIDATE(_ccs_deserialize_bin_uint64(&tmp, buffer_size, buffer));
	*sz = tmp;
	return CCS_SUCCESS;
}

static inline size_t
_ccs_serialize_bin_size_string(const char *str) {
	size_t sz = strlen(str) + 1;
	return sz + _ccs_serialize_bin_size_size(sz);
}

static inline ccs_error_t
_ccs_serialize_bin_string(const char  *str,
                          size_t      *buffer_size,
                          char       **buffer) {
	uint64_t sz = strlen(str) + 1;
	CCS_VALIDATE(_ccs_serialize_bin_size(sz, buffer_size, buffer));
	CCS_REFUTE(*buffer_size < sz, CCS_NOT_ENOUGH_DATA);
	memcpy(*buffer, str, sz);
	*buffer_size -= sz;
	*buffer += sz;
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_deserialize_bin_string(const char **str,
                            size_t      *buffer_size,
                            const char **buffer) {
	size_t sz;
	CCS_VALIDATE(_ccs_deserialize_bin_size(&sz, buffer_size, buffer));
	CCS_REFUTE(*buffer_size < sz, CCS_NOT_ENOUGH_DATA);
	*str = *buffer;
	*buffer_size -= sz;
	*buffer += sz;
	return CCS_SUCCESS;
}

struct _ccs_blob_s {
	size_t      sz;
	const void *blob;
};
typedef struct _ccs_blob_s _ccs_blob_t;

static inline size_t
_ccs_serialize_bin_size_ccs_blob(_ccs_blob_t *b) {
	return _ccs_serialize_bin_size_size(b->sz) +
	       b->sz;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_blob(
		_ccs_blob_t *b,
		size_t      *buffer_size,
		char       **buffer) {
	CCS_VALIDATE(_ccs_serialize_bin_size(b->sz, buffer_size, buffer));
	CCS_REFUTE(*buffer_size < b->sz, CCS_NOT_ENOUGH_DATA);
	memcpy(*buffer, b->blob, b->sz);
	*buffer_size -= b->sz;
	*buffer += b->sz;
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_deserialize_bin_ccs_blob(
		_ccs_blob_t *b,
		size_t      *buffer_size,
		const char **buffer) {
	CCS_VALIDATE(_ccs_deserialize_bin_size(&b->sz, buffer_size, buffer));
	CCS_REFUTE(*buffer_size < b->sz, CCS_NOT_ENOUGH_DATA);
	b->blob = *buffer;
	*buffer_size -= b->sz;
	*buffer += b->sz;
	return CCS_SUCCESS;
}

static inline size_t
_ccs_serialize_bin_size_ccs_interval(const ccs_interval_t *interval) {
	return _ccs_serialize_bin_size_ccs_numeric_type(interval->type) +
	       (interval->type == CCS_NUM_FLOAT ?
	           _ccs_serialize_bin_size_ccs_float(interval->lower.f) +
	             _ccs_serialize_bin_size_ccs_float(interval->upper.f) :
	           _ccs_serialize_bin_size_ccs_int(interval->lower.i) +
	             _ccs_serialize_bin_size_ccs_int(interval->upper.i) ) +
	       _ccs_serialize_bin_size_ccs_bool(interval->lower_included) +
	       _ccs_serialize_bin_size_ccs_bool(interval->upper_included);
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_interval(const ccs_interval_t  *interval,
                                size_t                *buffer_size,
                                char                 **buffer) {
	CCS_VALIDATE(_ccs_serialize_bin_ccs_numeric_type(
		interval->type, buffer_size, buffer));
	if (interval->type == CCS_NUM_FLOAT) {
		CCS_VALIDATE(_ccs_serialize_bin_ccs_float(interval->lower.f, buffer_size, buffer));
		CCS_VALIDATE(_ccs_serialize_bin_ccs_float(interval->upper.f, buffer_size, buffer));
	} else {
		CCS_VALIDATE(_ccs_serialize_bin_ccs_int(interval->lower.i, buffer_size, buffer));
		CCS_VALIDATE(_ccs_serialize_bin_ccs_int(interval->upper.i, buffer_size, buffer));
	}
	CCS_VALIDATE(_ccs_serialize_bin_ccs_bool(interval->lower_included, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_bool(interval->upper_included, buffer_size, buffer));
        return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_deserialize_bin_ccs_interval(ccs_interval_t  *interval,
                                  size_t          *buffer_size,
                                  const char     **buffer) {
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_numeric_type(
		&interval->type, buffer_size, buffer));
	if (interval->type == CCS_NUM_FLOAT) {
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_float(&interval->lower.f, buffer_size, buffer));
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_float(&interval->upper.f, buffer_size, buffer));
	} else {
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_int(&interval->lower.i, buffer_size, buffer));
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_int(&interval->upper.i, buffer_size, buffer));
	}
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_bool(&interval->lower_included, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_bool(&interval->upper_included, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline size_t
_ccs_serialize_bin_size_ccs_datum(ccs_datum_t datum) {
	size_t sz = 0;
	sz += _ccs_serialize_bin_size_ccs_data_type(datum.type);
	switch(datum.type) {
	case CCS_NONE:
	case CCS_INACTIVE:
		break;
	case CCS_INTEGER:
		sz += _ccs_serialize_bin_size_ccs_int(datum.value.i);
		break;
	case CCS_FLOAT:
		sz += _ccs_serialize_bin_size_ccs_float(datum.value.f);
		break;
	case CCS_BOOLEAN:
		sz += _ccs_serialize_bin_size_ccs_bool(datum.value.i);
		break;
	case CCS_STRING:
		sz += _ccs_serialize_bin_size_string(datum.value.s);
		break;
	case CCS_OBJECT:
		sz += _ccs_serialize_bin_size_ccs_object(datum.value.o);
		break;
	default: /* should be a hard error */
		;
	}
	return sz;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_datum(
		ccs_datum_t   datum,
		size_t       *buffer_size,
		char        **buffer) {
	CCS_VALIDATE(_ccs_serialize_bin_ccs_data_type(datum.type, buffer_size, buffer));
	switch(datum.type) {
	case CCS_NONE:
	case CCS_INACTIVE:
		break;
	case CCS_INTEGER:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_int(datum.value.i, buffer_size, buffer));
		break;
	case CCS_FLOAT:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_float(datum.value.f, buffer_size, buffer));
		break;
	case CCS_BOOLEAN:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_bool(datum.value.i, buffer_size, buffer));
		break;
	case CCS_STRING:
		CCS_VALIDATE(_ccs_serialize_bin_string(datum.value.s, buffer_size, buffer));
		break;
	case CCS_OBJECT:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_object(datum.value.o, buffer_size, buffer));
		break;
	default:
		CCS_RAISE(CCS_INVALID_TYPE, "Unsupported datum type: %d", datum.type);
	}
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_deserialize_bin_ccs_datum(
		ccs_datum_t  *datum,
		size_t       *buffer_size,
		const char  **buffer) {
	ccs_data_type_t type;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_data_type(&type, buffer_size, buffer));
	datum->flags = CCS_FLAG_DEFAULT;
	switch(type) {
	case CCS_NONE:
		*datum = ccs_none;
		break;
	case CCS_INACTIVE:
		*datum = ccs_inactive;
		break;
	case CCS_INTEGER:
		{
		ccs_int_t i;
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_int(&i, buffer_size, buffer));
		*datum = ccs_int(i);
		}
		break;
	case CCS_FLOAT:
		{
		ccs_float_t f;
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_float(&f, buffer_size, buffer));
		*datum = ccs_float(f);
		}
		break;
	case CCS_BOOLEAN:
		{
		ccs_bool_t b;
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_bool(&b, buffer_size, buffer));
		*datum = ccs_bool(b);
		}
		break;
	case CCS_STRING:
		{
		const char *s;
		CCS_VALIDATE(_ccs_deserialize_bin_string(&s, buffer_size, buffer));
		*datum = ccs_string(s);
		datum->flags |= CCS_FLAG_TRANSIENT;
		}
		break;
	case CCS_OBJECT:
		{
		ccs_object_t o;
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_object(&o, buffer_size, buffer));
		*datum = ccs_object(o);
		datum->flags |= CCS_FLAG_ID;
		}
		break;
	default:
		*datum = ccs_none;
		CCS_RAISE(CCS_INVALID_TYPE, "Unsupported datum type: %d", type);
	}
	return CCS_SUCCESS;
}

static inline size_t
_ccs_serialize_bin_size_ccs_object_internal(
		_ccs_object_internal_t *obj) {
	return _ccs_serialize_bin_size_ccs_object_type(obj->type) +
	       _ccs_serialize_bin_size_ccs_object((ccs_object_t)obj);
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_object_internal(
		_ccs_object_internal_t  *obj,
		size_t                  *buffer_size,
		char                   **buffer) {
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_type(
		obj->type, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object(
		(ccs_object_t)obj, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_deserialize_bin_ccs_object_internal(
		_ccs_object_internal_t  *obj,
		size_t                  *buffer_size,
		const char             **buffer,
		ccs_object_t            *handle_ret) {
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object_type(
		&obj->type, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object(
		handle_ret, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_object_handle_check_add(
		ccs_map_t map,
		ccs_object_t handle,
		ccs_object_t obj) {
	ccs_bool_t found;
	ccs_datum_t d = ccs_object(handle);
	d.flags |= CCS_FLAG_ID;
	CCS_VALIDATE(ccs_map_exist(map, d, &found));
	CCS_REFUTE(found, CCS_HANDLE_DUPLICATE);
	CCS_VALIDATE(ccs_map_set(map, d, ccs_object(obj)));
	return CCS_SUCCESS;
}

struct _ccs_object_deserialize_options_s {
	ccs_map_t                           handle_map;
	ccs_bool_t                          map_values;
	_ccs_file_descriptor_state_t      **ppfd_state;
	void                               *vector;
	void                               *data;
	ccs_object_deserialize_callback_t   deserialize_callback;
	void                               *deserialize_user_data;
};
typedef struct _ccs_object_deserialize_options_s _ccs_object_deserialize_options_t;

static inline ccs_error_t
_ccs_object_serialize_user_data_size(
		ccs_object_t                     object,
		ccs_serialize_format_t           format,
		size_t                          *buffer_size,
		_ccs_object_serialize_options_t *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
	{
		_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
		size_t serialize_data_size = 0;
		if (obj->user_data) {
			if (obj->serialize_callback)
				CCS_VALIDATE(obj->serialize_callback(
					object, 0, NULL, &serialize_data_size, obj->serialize_user_data));
			else if (opts->serialize_callback)
				CCS_VALIDATE(opts->serialize_callback(
					object, 0, NULL, &serialize_data_size, opts->serialize_user_data));
		}
		*buffer_size += _ccs_serialize_bin_size_size(serialize_data_size) + serialize_data_size;
		break;
	}
	default:
		CCS_RAISE(CCS_INVALID_VALUE, "Unsupported serialization format: %d", format);
	}
	return CCS_SUCCESS;
}


static inline ccs_error_t
_ccs_object_serialize_user_data(
		ccs_object_t                      object,
		ccs_serialize_format_t            format,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
	{
		_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
		size_t serialize_data_size = 0;
		/* optimization would require using an uncompressed size */
		if (obj->user_data) {
			if (obj->serialize_callback)
				CCS_VALIDATE(obj->serialize_callback(
					object, 0, NULL, &serialize_data_size, obj->serialize_user_data));
			else if (opts->serialize_callback)
				CCS_VALIDATE(opts->serialize_callback(
					object, 0, NULL, &serialize_data_size, opts->serialize_user_data));
		}
		CCS_VALIDATE(_ccs_serialize_bin_size(
			serialize_data_size, buffer_size, buffer));
		if (obj->user_data) {
			CCS_REFUTE(*buffer_size < serialize_data_size, CCS_NOT_ENOUGH_DATA);
			if (obj->serialize_callback)
				CCS_VALIDATE(obj->serialize_callback(
					object, serialize_data_size, *buffer, NULL, obj->serialize_user_data));
			else if (opts->serialize_callback)
				CCS_VALIDATE(opts->serialize_callback(
					object, serialize_data_size, *buffer, NULL, opts->serialize_user_data));
		}
		if (serialize_data_size) {
			*buffer_size -= serialize_data_size;
			*buffer += serialize_data_size;
		}
		break;
	}
	default:
		CCS_RAISE(CCS_INVALID_VALUE, "Unsupported serialization format: %d", format);
	}
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_object_deserialize_user_data(
		ccs_object_t                        object,
		ccs_serialize_format_t              format,
		uint32_t                            version,
		size_t                             *buffer_size,
		const char                        **buffer,
		_ccs_object_deserialize_options_t  *opts) {
	(void)version;
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
	{
		size_t serialize_data_size;
		CCS_VALIDATE(_ccs_deserialize_bin_size(
			&serialize_data_size, buffer_size, buffer));
		if (serialize_data_size) {
			if (opts->deserialize_callback)
				CCS_VALIDATE(opts->deserialize_callback(
					object, serialize_data_size, *buffer, opts->deserialize_user_data));
			*buffer_size -= serialize_data_size;
			*buffer += serialize_data_size;
		}
		break;
	}
	default:
		CCS_RAISE(CCS_INVALID_VALUE, "Unsupported serialization format: %d", format);
	}
	return CCS_SUCCESS;
}

#endif //_CONFIGSPACE_INTERNAL_H
