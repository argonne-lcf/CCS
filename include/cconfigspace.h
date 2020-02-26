#include "ccs/configuation_space.h"

typedef struct ccs_configuration_space_s *ccs_configuration_space_t;
typedef struct ccs_configuration_s       *ccs_configuration_t;
typedef struct ccs_hyper_parameter_s     *ccs_hyper_parameter_t;
typedef struct ccs_condition_s           *ccs_condition_t;
typedef struct ccs_forbidden_clause_s    *ccs_forbidden_clause_t;

enum ccs_error_e {
  CCS_SUCCESS
};

typedef enum ccs_error_e ccs_error_t;

union ccs_data_u {
  double      d;
  int         i;
  const char *s;
};

typedef union ccs_data_u ccs_data_t;



