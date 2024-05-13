#ifndef _SEARCH_CONFIGURATION_INTERNAL_H
#define _SEARCH_CONFIGURATION_INTERNAL_H
#include "configuration_internal.h"
#include "tree_configuration_internal.h"

struct _ccs_search_configuration_data_s;
typedef struct _ccs_search_configuration_data_s _ccs_search_configuration_data_t;

struct _ccs_search_configuration_ops_s {
	_ccs_object_ops_t obj_ops;

	ccs_result_t (*hash)(
		ccs_search_configuration_t configuration,
		ccs_hash_t                *hash_ret);

	ccs_result_t (*cmp)(
		ccs_search_configuration_t configuration,
		ccs_search_configuration_t other,
		int                       *cmp_ret);
};
typedef struct _ccs_search_configuration_ops_s _ccs_search_configuration_ops_t;

struct _ccs_search_configuration_s {
	_ccs_object_internal_t            obj;
	_ccs_search_configuration_data_t *data;
};

struct _ccs_search_configuration_data_s {
	ccs_search_space_t space;
};

static inline ccs_result_t
_ccs_search_configuration_hash(
	ccs_search_configuration_t configuration,
	ccs_hash_t                *hash_ret)
{
	_ccs_search_configuration_ops_t *ops =
		(_ccs_search_configuration_ops_t *)configuration->obj.ops;
	CCS_VALIDATE(ops->hash(configuration, hash_ret));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_search_configuration_cmp(
	ccs_search_configuration_t configuration,
	ccs_search_configuration_t other,
	int                       *cmp_ret)
{
	_ccs_search_configuration_ops_t *ops =
		(_ccs_search_configuration_ops_t *)configuration->obj.ops;
	CCS_VALIDATE(ops->cmp(configuration, other, cmp_ret));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_search_configuration_get_features(
	ccs_search_configuration_t configuration,
	ccs_features_t            *features_ret)
{
	switch (CCS_OBJ_TYPE(configuration)) {
	case CCS_OBJECT_TYPE_CONFIGURATION: {
		ccs_configuration_t config = (ccs_configuration_t)configuration;
		*features_ret              = config->data->features;
	} break;
	case CCS_OBJECT_TYPE_TREE_CONFIGURATION: {
		ccs_tree_configuration_t config =
			(ccs_tree_configuration_t)configuration;
		*features_ret = config->data->features;
	} break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_OBJECT,
			"Unsupported object type: %d",
			CCS_OBJ_TYPE(configuration));
	}
	return CCS_RESULT_SUCCESS;
}

#endif //_SEARCH_CONFIGURATION_INTERNAL_H
