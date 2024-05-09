#ifndef _SEARCH_CONFIGURATION_INTERNAL_H
#define _SEARCH_CONFIGURATION_INTERNAL_H

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

#endif //_SEARCH_CONFIGURATION_INTERNAL_H
