#ifndef _SEARCH_SPACE_INTERNAL_H
#define _SEARCH_SPACE_INTERNAL_H
#include "configuration_space_internal.h"
#include "tree_space_internal.h"

struct _ccs_search_space_data_s;
typedef struct _ccs_search_space_data_s _ccs_search_space_data_t;

struct _ccs_search_space_s {
	_ccs_object_internal_t    obj;
	_ccs_search_space_data_t *data;
};

static inline ccs_result_t
_ccs_search_space_get_feature_space(
	ccs_search_space_t   search_space,
	ccs_feature_space_t *feature_space_ret)
{
	switch (CCS_OBJ_TYPE(search_space)) {
	case CCS_OBJECT_TYPE_TREE_SPACE: {
		ccs_tree_space_t tree_space = (ccs_tree_space_t)search_space;
		*feature_space_ret =
			((_ccs_tree_space_common_data_t *)(tree_space->data))
				->feature_space;
	} break;
	case CCS_OBJECT_TYPE_CONFIGURATION_SPACE: {
		ccs_configuration_space_t configuration_space =
			(ccs_configuration_space_t)search_space;
		*feature_space_ret = configuration_space->data->feature_space;
	} break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_OBJECT,
			"Unsupported object type: %d",
			CCS_OBJ_TYPE(search_space));
	}
	return CCS_RESULT_SUCCESS;
}
#endif //_SEARCH_SPACE_INTERNAL_H
