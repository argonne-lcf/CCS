#ifndef _CCS_BASE_H
#define _CCS_BASE_H
#include <limits.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

typedef double  ccs_float_t;
typedef int64_t ccs_int_t;
typedef int32_t ccs_bool_t;

#define CCS_TRUE ((ccs_bool_t)(1))
#define CCS_FALSE ((ccs_bool_t)(0))

#define CCS_INT_MAX LLONG_MAX
#define CCS_INT_MIN LLONG_MIN
#define CCS_INFINITY INFINITY

typedef struct _ccs_rng_s                 *ccs_rng_t;
typedef struct _ccs_distribution_s        *ccs_distribution_t;
typedef struct _ccs_hyperparameter_s      *ccs_hyperparameter_t;
typedef struct _ccs_expression_s          *ccs_expression_t;
typedef struct _ccs_condition_s           *ccs_condition_t;
typedef struct _ccs_forbidden_clause_s    *ccs_forbidden_clause_t;
typedef struct _ccs_configuration_space_s *ccs_configuration_space_t;
typedef struct _ccs_configuration_s       *ccs_configuration_t;

enum ccs_error_e {
	CCS_SUCCESS,
	CCS_INVALID_OBJECT,
	CCS_INVALID_VALUE,
	CCS_INVALID_TYPE,
	CCS_INVALID_SCALE,
	CCS_INVALID_DISTRIBUTION,
	CCS_INVALID_HYPERPARAMETER,
	CCS_INVALID_CONFIGURATION,
	CCS_INVALID_NAME,
	CCS_TYPE_NOT_COMPARABLE,
	CCS_INVALID_BOUNDS,
	CCS_OUT_OF_BOUNDS,
	CCS_SAMPLING_UNSUCCESSFUL,
	CCS_ENOMEM,
	CCS_UNSUPPORTED_OPERATION,
	CCS_ERROR_MAX,
	CCS_ERROR_FORCE_32BIT = INT_MAX
};

typedef int ccs_error_t;

enum ccs_object_type_e {
	CCS_RNG,
	CCS_DISTRIBUTION,
	CCS_HYPERPARAMETER,
	CCS_EXPRESSION,
	CCS_CONDITION,
	CCS_FORBIDDEN_CLAUSE,
	CCS_CONFIGURATION_SPACE,
	CCS_CONFIGURATION,
	CCS_OBJECT_TYPE_MAX,
	CCS_OBJECT_TYPE_FORCE_32BIT = INT_MAX
};

typedef enum ccs_object_type_e ccs_object_type_t;

enum ccs_data_type_e {
	CCS_NONE,
	CCS_INTEGER,
	CCS_FLOAT,
	CCS_BOOLEAN,
	CCS_STRING,
	CCS_OBJECT,
	CCS_DATA_TYPE_MAX,
	CCS_DATA_TYPE_FORCE_64BIT = INT64_MAX
};

typedef enum ccs_data_type_e ccs_data_type_t;

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

struct ccs_interval_s {
	ccs_numeric_type_t type;
	ccs_numeric_t   lower;
	ccs_numeric_t   upper;
	ccs_bool_t      lower_included;
	ccs_bool_t      upper_included;
};

typedef struct ccs_interval_s ccs_interval_t;

struct ccs_datum_s {
	ccs_value_t value;
	ccs_data_type_t type;
};

typedef struct ccs_datum_s ccs_datum_t;

extern const ccs_datum_t ccs_none;

#define CCS_NONE_VAL {{0}, CCS_NONE}

extern ccs_error_t
ccs_init();

extern ccs_error_t
ccs_retain_object(ccs_object_t object);

extern ccs_error_t
ccs_release_object(ccs_object_t object);

extern ccs_error_t
ccs_object_get_type(ccs_object_t       object,
                    ccs_object_type_t *type_ret);

extern ccs_error_t
ccs_object_get_refcount(ccs_object_t  object,
                        int32_t      *refcount_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_BASE_H
