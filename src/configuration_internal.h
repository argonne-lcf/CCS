#ifndef _CONFIGURATION_INTERNAL_H
#define _CONFIGURATION_INTERNAL_H
#include "binding_internal.h"

struct _ccs_configuration_ops_s {
	_ccs_object_ops_t obj_ops;
};
typedef struct _ccs_configuration_ops_s _ccs_configuration_ops_t;

struct _ccs_configuration_data_s;
typedef struct _ccs_configuration_data_s _ccs_configuration_data_t;

struct _ccs_configuration_s {
	_ccs_object_internal_t     obj;
	_ccs_configuration_data_t *data;
};

struct _ccs_configuration_data_s {
	void                      *user_data;
	ccs_configuration_space_t  configuration_space;
	size_t                     num_values;
	ccs_datum_t               *values;
};

#endif //_CONFIGURATION_INTERNAL_H
