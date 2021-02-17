#ifndef _CCS_BASE_H
#define _CCS_BASE_H
#include <limits.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

typedef double   ccs_float_t;
typedef int64_t  ccs_int_t;
typedef int32_t  ccs_bool_t;
typedef uint32_t ccs_hash_t;

typedef struct {
	uint16_t revision;
	uint16_t patch;
	uint16_t minor;
	uint16_t major;
} ccs_version_t;

extern const ccs_version_t ccs_version;

#define CCS_TRUE ((ccs_bool_t)(1))
#define CCS_FALSE ((ccs_bool_t)(0))

#define CCS_INT_MAX LLONG_MAX
#define CCS_INT_MIN LLONG_MIN
#define CCS_INFINITY INFINITY

typedef struct _ccs_rng_s                 *ccs_rng_t;
typedef struct _ccs_distribution_s        *ccs_distribution_t;
typedef struct _ccs_hyperparameter_s      *ccs_hyperparameter_t;
typedef struct _ccs_expression_s          *ccs_expression_t;
typedef struct _ccs_context_s             *ccs_context_t;
typedef struct _ccs_configuration_space_s *ccs_configuration_space_t;
typedef struct _ccs_configuration_s       *ccs_configuration_t;
typedef struct _ccs_objective_space_s     *ccs_objective_space_t;
typedef struct _ccs_evaluation_s          *ccs_evaluation_t;
typedef struct _ccs_tuner_s               *ccs_tuner_t;

enum ccs_error_e {
	CCS_SUCCESS,
	CCS_INVALID_OBJECT,
	CCS_INVALID_VALUE,
	CCS_INVALID_TYPE,
	CCS_INVALID_SCALE,
	CCS_INVALID_DISTRIBUTION,
	CCS_INVALID_EXPRESSION,
	CCS_INVALID_HYPERPARAMETER,
	CCS_INVALID_CONFIGURATION,
	CCS_INVALID_NAME,
	CCS_INVALID_CONDITION,
	CCS_INVALID_TUNER,
	CCS_INVALID_GRAPH,
	CCS_TYPE_NOT_COMPARABLE,
	CCS_INVALID_BOUNDS,
	CCS_OUT_OF_BOUNDS,
	CCS_SAMPLING_UNSUCCESSFUL,
	CCS_INACTIVE_HYPERPARAMETER,
	CCS_OUT_OF_MEMORY,
	CCS_UNSUPPORTED_OPERATION,
	CCS_ERROR_MAX,
	CCS_ERROR_FORCE_32BIT = INT32_MAX
};
typedef enum ccs_error_e ccs_error_t;

typedef int32_t ccs_result_t;

enum ccs_object_type_e {
	CCS_RNG,
	CCS_DISTRIBUTION,
	CCS_HYPERPARAMETER,
	CCS_EXPRESSION,
	CCS_CONFIGURATION_SPACE,
	CCS_CONFIGURATION,
	CCS_OBJECTIVE_SPACE,
	CCS_EVALUATION,
	CCS_TUNER,
	CCS_OBJECT_TYPE_MAX,
	CCS_OBJECT_TYPE_FORCE_32BIT = INT32_MAX
};

typedef enum ccs_object_type_e ccs_object_type_t;

enum ccs_data_type_e {
	CCS_NONE,
	CCS_INTEGER,
	CCS_FLOAT,
	CCS_BOOLEAN,
	CCS_STRING,
	CCS_INACTIVE,
	CCS_OBJECT,
	CCS_DATA_TYPE_MAX,
	CCS_DATA_TYPE_FORCE_32BIT = INT32_MAX
};

typedef enum ccs_data_type_e ccs_data_type_t;

typedef uint32_t ccs_datum_flags_t;

enum ccs_datum_flag_e {
	CCS_FLAG_DEFAULT = 0,
	CCS_DATUM_FLAG_FORCE_32BIT = INT32_MAX
};

typedef enum ccs_datum_flag_e ccs_datum_flag_t;

enum ccs_numeric_type_e {
	CCS_NUM_INTEGER = CCS_INTEGER,
	CCS_NUM_FLOAT = CCS_FLOAT,
	CCS_NUM_TYPE_MAX,
	CCS_NUM_TYPE_FORCE_64BIT = INT64_MAX
};

typedef enum ccs_numeric_type_e ccs_numeric_type_t;

typedef void * ccs_object_t;

union ccs_numeric_u {
	ccs_float_t   f;
	ccs_int_t     i;
#ifdef __cplusplus
	ccs_numeric_u() : i(0L) {}
	ccs_numeric_u(float v) : f((ccs_float_t)v) {}
	ccs_numeric_u(int v) : i((ccs_int_t)v) {}
	ccs_numeric_u(ccs_int_t v) : i(v) {}
	ccs_numeric_u(ccs_float_t v) : f(v) {}
#endif
};

typedef union ccs_numeric_u ccs_numeric_t;

#ifdef __cplusplus
#define CCSF(v) v
#define CCSI(v) v
#else
#define CCSF(v) ((ccs_numeric_t){ .f = v })
#define CCSI(v) ((ccs_numeric_t){ .i = v })
#endif

union ccs_value_u {
	ccs_float_t   f;
	ccs_int_t     i;
	const char   *s;
	ccs_object_t  o;
#ifdef __cplusplus
	ccs_value_u() : i(0L) {}
	ccs_value_u(float v) : f((ccs_float_t)v) {}
	ccs_value_u(int v) : i((ccs_int_t)v) {}
	ccs_value_u(ccs_float_t v) : f(v) {}
	ccs_value_u(ccs_int_t v) : i(v) {}
	ccs_value_u(char *v) : s(v) {}
	ccs_value_u(ccs_object_t v) : o(v) {}
#endif
};

typedef union ccs_value_u ccs_value_t;

struct ccs_datum_s {
	ccs_value_t value;
	ccs_data_type_t type;
	ccs_datum_flags_t flags;
};

typedef struct ccs_datum_s ccs_datum_t;

static inline ccs_datum_t
ccs_bool(ccs_bool_t v) {
	ccs_datum_t d;
	d.type = CCS_BOOLEAN;
	d.value.i = v;
	d.flags = CCS_FLAG_DEFAULT;
	return d;
}

static inline ccs_datum_t
ccs_float(ccs_float_t v) {
	ccs_datum_t d;
	d.type = CCS_FLOAT;
	d.value.f = v;
	d.flags = CCS_FLAG_DEFAULT;
	return d;
}

static inline ccs_datum_t
ccs_int(ccs_int_t v) {
	ccs_datum_t d;
	d.type = CCS_INTEGER;
	d.value.i = v;
	d.flags = CCS_FLAG_DEFAULT;
	return d;
}

static inline ccs_datum_t
ccs_object(ccs_object_t v) {
	ccs_datum_t d;
	d.type = CCS_OBJECT;
	d.value.o = v;
	d.flags = CCS_FLAG_DEFAULT;
	return d;
}

static inline ccs_datum_t
ccs_string(const char *v) {
	ccs_datum_t d;
	d.type = CCS_STRING;
	d.value.s = v;
	d.flags = CCS_FLAG_DEFAULT;
	return d;
}

extern const ccs_datum_t ccs_none;
extern const ccs_datum_t ccs_inactive;
extern const ccs_datum_t ccs_true;
extern const ccs_datum_t ccs_false;

#define CCS_NONE_VAL {{0}, CCS_NONE, CCS_FLAG_DEFAULT}
#define CCS_INACTIVE_VAL {{0}, CCS_INACTIVE, CCS_FLAG_DEFAULT}
#ifdef __cplusplus
#define CCS_TRUE_VAL {{(ccs_int_t)CCS_TRUE}, CCS_BOOLEAN, CCS_FLAG_DEFAULT}
#define CCS_FALSE_VAL {{(ccs_int_t)CCS_FALSE}, CCS_BOOLEAN, CCS_FLAG_DEFAULT}
#else
#define CCS_TRUE_VAL {{.i = CCS_TRUE}, CCS_BOOLEAN, CCS_FLAG_DEFAULT}
#define CCS_FALSE_VAL {{.i = CCS_FALSE}, CCS_BOOLEAN, CCS_FLAG_DEFAULT}
#endif

extern ccs_result_t
ccs_init();

extern ccs_version_t
ccs_get_version();

extern ccs_result_t
ccs_retain_object(ccs_object_t object);

extern ccs_result_t
ccs_release_object(ccs_object_t object);

extern ccs_result_t
ccs_object_get_type(ccs_object_t       object,
                    ccs_object_type_t *type_ret);

extern ccs_result_t
ccs_object_get_refcount(ccs_object_t  object,
                        int32_t      *refcount_ret);

typedef void (*ccs_object_release_callback_t)(ccs_object_t object, void *user_data);

extern ccs_result_t
ccs_object_set_destroy_callback(ccs_object_t  object,
                                void (*callback)(
                                  ccs_object_t object,
                                  void *user_data),
                                void *user_data);

#ifdef __cplusplus
}
#endif

#endif //_CCS_BASE_H
