#include <stdint.h>
#include "ccs/configuration_space.h"
#include "ccs/configuration.h"
#include "ccs/hyperparameter.h"
#include "ccs/condition.h"

typedef struct _ccs_configuration_space_s *ccs_configuration_space_t;
typedef struct _ccs_configuration_s       *ccs_configuration_t;
typedef struct _ccs_distribution_s        *ccs_distribution_t;
typedef struct _ccs_hyperparameter_s      *ccs_hyperparameter_t;
typedef struct _ccs_expression_s          *ccs_expression_t;
typedef struct _ccs_condition_s           *ccs_condition_t;
typedef struct _ccs_forbidden_clause_s    *ccs_forbidden_clause_t;

enum ccs_error_e {
	CCS_SUCCESS,
	CCS_ERROR_MAX,
	CCS_ERROR_FORCE_32BIT = MAX_INT
};

typedef enum ccs_error_e ccs_error_t;

enum ccs_object_type_e {
	CCS_DISTRIBUTION,
	CCS_HYPERPARAMETER,
	CCS_EXPRESSION,
	CCS_CONDITION,
	CCS_FORBIDDEN_CLAUSE,
	CCS_CONFIGURATION_SPACE,
	CCS_CONFIGURATION,
	CCS_OBJECT_TYPE_MAX,
	CCS_OBJECT_TYPE_FORCE_32BIT = MAX_INT
};

typedef enum ccs_object_type_e ccs_object_type_t;

enum ccs_data_type_e {
	CCS_INTEGER,
	CCS_FLOAT,
	CCS_STRING,
	CCS_OBJECT,
	CCS_DATA_TYPE_MAX,
	CCS_DATA_TYPE_FORCE_32BIT = MAX_INT
};

typedef enum ccs_data_type_e ccs_data_type_t;

union ccs_object_u {
	ccs_configuration_space_t configuration_space;
	ccs_configuration_t       configuration;
	ccs_distribution_t        distribution;
	ccs_hyperparameter_t      hyperparameter;
	ccs_expression_t          expression;
	ccs_condition_t           condition;
	ccs_forbidden_clause_t    forbidden_clause;
};

typedef union ccs_object_u ccs_object_t;


union ccs_value_u {
{
	double        d;
	int64_t       i;
	const char   *s;
	ccs_object_t  o;
};

typedef union ccs_value_u ccs_value_t;


struct ccs_datum_u {
	ccs_value_u value;
	ccs_data_type_t type;
};

typedef struct ccs_datum_u ccs_datum_t;

extern ccs_error_t
ccs_retain_object(ccs_object_t object);

extern ccs_error_t
ccs_release_object(ccs_object_t object);

extern ccs_error_t
ccs_object_get_type(ccs_object_t       object,
                    ccs_object_type_t *type_ret);
