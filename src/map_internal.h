#ifndef _MAP_INTERNAL_H
#define _MAP_INTERNAL_H

struct _ccs_map_ops_s {
	_ccs_object_ops_t obj_ops;
};
typedef struct _ccs_map_ops_s _ccs_map_ops_t;

struct _ccs_map_data_s;
typedef struct _ccs_map_data_s _ccs_map_data_t;

struct _ccs_map_s {
	_ccs_object_internal_t obj;
	_ccs_map_data_t       *data;
};

#endif //_MAP_INTERNAL_H
