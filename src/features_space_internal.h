#ifndef _FEATURES_SPACE_INTERNAL_H
#define _FEATURES_SPACE_INTERNAL_H
#include "context_internal.h"

struct _ccs_features_space_data_s;
typedef struct _ccs_features_space_data_s _ccs_features_space_data_t;

struct _ccs_features_space_ops_s {
	_ccs_context_ops_t ops;
};
typedef struct _ccs_features_space_ops_s _ccs_features_space_ops_t;

struct _ccs_features_space_s {
	_ccs_object_internal_t           obj;
	_ccs_features_space_data_t *data;
};

struct _ccs_features_space_data_s {
	const char                    *name;
	void                          *user_data;
	UT_array                      *hyperparameters;
	_ccs_hyperparameter_wrapper_t *name_hash;
	_ccs_hyperparameter_wrapper_t *handle_hash;
};

#endif //_FEATURES_SPACE_INTERNAL_H
