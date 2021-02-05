#ifndef _FEATURES_INTERNAL_H
#define _FEATURES_INTERNAL_H

struct _ccs_features_ops_s {
	_ccs_object_ops_t obj_ops;
};
typedef struct _ccs_features_ops_s _ccs_features_ops_t;

struct _ccs_features_data_s;
typedef struct _ccs_features_data_s _ccs_features_data_t;

struct _ccs_features_s {
	_ccs_object_internal_t     obj;
	_ccs_features_data_t *data;
};

struct _ccs_features_data_s {
	void                      *user_data;
	ccs_features_space_t  features_space;
	size_t                     num_values;
	ccs_datum_t               *values;
};

#endif //_FEATURES_INTERNAL_H
