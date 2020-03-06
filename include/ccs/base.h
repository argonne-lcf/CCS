#ifndef _CCS_BASE_H
#define _CCS_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef double  ccs_float_t;
typedef int64_t ccs_int_t;

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
	CCS_ENOMEM,
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
	CCS_INTEGER,
	CCS_FLOAT,
	CCS_STRING,
	CCS_OBJECT,
	CCS_DATA_TYPE_MAX,
	CCS_DATA_TYPE_FORCE_32BIT = INT_MAX
};

typedef enum ccs_data_type_e ccs_data_type_t;

union ccs_object_u {
	void                      *ptr;
	ccs_rng_t                  rng;
	ccs_configuration_space_t  configuration_space;
	ccs_configuration_t        configuration;
	ccs_distribution_t         distribution;
	ccs_hyperparameter_t       hyperparameter;
	ccs_expression_t           expression;
	ccs_condition_t            condition;
	ccs_forbidden_clause_t     forbidden_clause;
};

typedef union ccs_object_u ccs_object_t;


union ccs_value_u {
	ccs_float_t   f;
	ccs_int_t     i;
	const char   *s;
	ccs_object_t  o;
};

typedef union ccs_value_u ccs_value_t;


struct ccs_datum_u {
	ccs_value_t value;
	ccs_data_type_t type;
};

typedef struct ccs_datum_u ccs_datum_t;

extern ccs_error_t
ccs_init();

extern ccs_error_t
_ccs_retain_object(ccs_object_t object);

#define ccs_retain_object(o) _ccs_retain_object((ccs_object_t)(o))

extern ccs_error_t
_ccs_release_object(ccs_object_t object);

#define ccs_release_object(o) _ccs_release_object((ccs_object_t)(o))

extern ccs_error_t
_ccs_object_get_type(ccs_object_t       object,
                     ccs_object_type_t *type_ret);

#define ccs_object_get_type(o, t) _ccs_object_get_type((ccs_object_t)(o), t)

extern ccs_error_t
_ccs_object_get_refcount(ccs_object_t  object,
                         int32_t      *refcount_ret);

#define ccs_object_get_refcount(o, c) _ccs_object_get_refcount((ccs_object_t)(o), c)

#ifdef __cplusplus
}
#endif

#endif //_CCS_BASE_H
