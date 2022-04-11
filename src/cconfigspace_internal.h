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

#define CCS_SERIALIZATION_API_VERSION_TYPE uint32_t
#define CCS_SERIALIZATION_API_VERSION \
	((CCS_SERIALIZATION_API_VERSION_TYPE)1)

typedef void *ccs_user_data_t;

/* "CCS" */
#define CCS_MAGIC_TAG { 0x43, 0x43, 0x53, 0x00 }

struct _ccs_object_ops_s {
	ccs_result_t (*del)(ccs_object_t object);

	ccs_result_t (*serialize_size)(
		ccs_object_t            object,
		ccs_serialize_format_t  format,
		size_t                 *cum_size);

	ccs_result_t (*serialize)(
		ccs_object_t             object,
		ccs_serialize_format_t   format,
		size_t                  *buffer_size,
		char                   **buffer);

/*	ccs_result_t (*deserialize)(
		ccs_object_t            *object_ret,
		ccs_serialize_format_t   format,
		uint32_t                 version,
		size_t                  *buffer_size,
		const char              *buffer,
		size_t                  *buffer_size_ret,
		char                   **buffer_ret); */
};

typedef struct _ccs_object_ops_s _ccs_object_ops_t;

struct _ccs_object_callback_s {
	ccs_object_release_callback_t  callback;
	void                          *user_data;
};
typedef struct _ccs_object_callback_s _ccs_object_callback_t;

struct _ccs_object_internal_s {
	ccs_object_type_t  type;
	int32_t            refcount;
	void              *user_data;
	UT_array          *callbacks;
	_ccs_object_ops_t *ops;
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
                 void                   *user_data,
                 _ccs_object_ops_t      *ops) {
	o->type = t;
	o->refcount = 1;
	o->user_data = user_data;
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

#define CCS_SERIALIZER(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)               \
static inline ccs_result_t                                                 \
_ccs_serialize_bin_ ## NAME (TYPE x, size_t *buffer_size, char **buffer) { \
  if (CCS_UNLIKELY(*buffer_size < sizeof(MAPPED_TYPE)))                    \
    return -CCS_NOT_ENOUGH_DATA;                                           \
  MAPPED_TYPE v = _ccs_pack_ ## NAME (x);                                  \
  memcpy(*buffer, &v, sizeof(MAPPED_TYPE));                                \
  *buffer_size -= sizeof(MAPPED_TYPE);                                     \
  *buffer += sizeof(MAPPED_TYPE);                                          \
  return CCS_SUCCESS;                                                      \
}                                                                          \
static inline size_t _ccs_serialize_bin_size_ ## NAME (TYPE x) {           \
  (void)x;                                                                 \
  return sizeof(MAPPED_TYPE);                                              \
}

#define CCS_DESERIALIZER(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)                      \
static inline ccs_result_t                                                          \
_ccs_peek_bin_ ## NAME (TYPE *x, size_t *buffer_size, const char **buffer) {        \
  if (CCS_UNLIKELY(*buffer_size < sizeof(MAPPED_TYPE)))                             \
    return -CCS_NOT_ENOUGH_DATA;                                                    \
  MAPPED_TYPE v;                                                                    \
  memcpy(&v, *buffer, sizeof(MAPPED_TYPE));                                         \
  *x = _ccs_unpack_ ## NAME (v);                                                    \
  return CCS_SUCCESS;                                                               \
}                                                                                   \
static inline ccs_result_t                                                          \
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
CCS_CONVERTER(ccs_result, ccs_result_t, 32)
CCS_CONVERTER(ccs_object, ccs_object_t, 64)
CCS_CONVERTER(ccs_user_data, ccs_user_data_t, 64)

static inline size_t
_ccs_serialize_bin_size_string(const char *str) {
	size_t sz = strlen(str) + 1;
	return sz + _ccs_serialize_bin_size_uint64(sz);
}

static inline ccs_result_t
_ccs_serialize_bin_string(const char  *str,
                          size_t      *buffer_size,
                          char       **buffer) {
	uint64_t sz = strlen(str) + 1;
	CCS_VALIDATE(_ccs_serialize_bin_uint64(sz, buffer_size, buffer));
	if (CCS_UNLIKELY(*buffer_size < sz))
		return -CCS_NOT_ENOUGH_DATA;
	memcpy(*buffer, str, sz);
	*buffer_size -= sz;
	*buffer += sz;
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_string(const char **str,
                            size_t      *buffer_size,
                            const char **buffer) {
	uint64_t sz;
	CCS_VALIDATE(_ccs_deserialize_bin_uint64(&sz, buffer_size, buffer));
	if (CCS_UNLIKELY(*buffer_size < sz))
		return -CCS_NOT_ENOUGH_DATA;
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
	return _ccs_serialize_bin_size_uint64(b->sz) +
	       b->sz;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_blob(
		_ccs_blob_t *b,
		size_t      *buffer_size,
		char       **buffer) {
	CCS_VALIDATE(_ccs_serialize_bin_uint64(b->sz, buffer_size, buffer));
	if (CCS_UNLIKELY(*buffer_size < b->sz))
		return -CCS_NOT_ENOUGH_DATA;
	memcpy(*buffer, b->blob, b->sz);
	*buffer_size -= b->sz;
	*buffer += b->sz;
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_ccs_blob(
		_ccs_blob_t *b,
		size_t      *buffer_size,
		const char **buffer) {
	uint64_t sz;
	CCS_VALIDATE(_ccs_deserialize_bin_uint64(&sz, buffer_size, buffer));
	b->sz = sz;
	if (CCS_UNLIKELY(*buffer_size < b->sz))
		return -CCS_NOT_ENOUGH_DATA;
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

static inline ccs_result_t
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

static inline ccs_result_t
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

static inline ccs_result_t
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
		return -CCS_INVALID_TYPE;
	}
	return CCS_SUCCESS;
}

static inline ccs_result_t
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
		return -CCS_INVALID_TYPE;
	}
	return CCS_SUCCESS;
}

static inline size_t
_ccs_serialize_bin_size_ccs_object_internal(
		_ccs_object_internal_t *obj) {
	return _ccs_serialize_bin_size_ccs_object_type(obj->type) +
	       _ccs_serialize_bin_size_ccs_object((ccs_object_t)obj) +
	       _ccs_serialize_bin_size_ccs_user_data(obj->user_data);
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_object_internal(
		_ccs_object_internal_t  *obj,
		size_t                  *buffer_size,
		char                   **buffer) {
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_type(
		obj->type, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object(
		(ccs_object_t)obj, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_user_data(
		obj->user_data, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_ccs_object_internal(
		_ccs_object_internal_t  *obj,
		size_t                  *buffer_size,
		const char             **buffer,
		ccs_object_t            *handle_ret) {
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object_type(
		&obj->type, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object(
		handle_ret, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_user_data(
		&obj->user_data, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_object_handle_check_add(
		ccs_map_t map,
		ccs_object_t handle,
		ccs_object_t obj) {
	ccs_bool_t found;
	ccs_datum_t d = ccs_object(handle);
	d.flags |= CCS_FLAG_ID;
	CCS_VALIDATE(ccs_map_exist(map, d, &found));
	if (CCS_UNLIKELY(found))
		return -CCS_HANDLE_DUPLICATE;
	CCS_VALIDATE(ccs_map_set(map, d, ccs_object(obj)));
	return CCS_SUCCESS;
}

struct _ccs_object_deserialize_options_s {
	ccs_map_t handle_map;
	ccs_bool_t map_values;
	void * vector;
	void * data;
};
typedef struct _ccs_object_deserialize_options_s _ccs_object_deserialize_options_t;

#endif //_CONFIGSPACE_INTERNAL_H
