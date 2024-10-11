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

typedef void *ccs_map_checkpoint_t;

ccs_result_t
_ccs_map_get_checkpoint(ccs_map_t map, ccs_map_checkpoint_t *checkpoint_ret);

ccs_result_t
_ccs_map_rewind(ccs_map_t map, ccs_map_checkpoint_t checkpoint);

#endif //_MAP_INTERNAL_H
