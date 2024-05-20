#include "cconfigspace_internal.h"
#include "tree_space_internal.h"
#include "tree_internal.h"

struct _ccs_tree_space_static_data_s {
	_ccs_tree_space_common_data_t common_data;
};
typedef struct _ccs_tree_space_static_data_s _ccs_tree_space_static_data_t;

static ccs_result_t
_ccs_tree_space_static_del(ccs_object_t o)
{
	struct _ccs_tree_space_static_data_s *data =
		(struct _ccs_tree_space_static_data_s *)(((ccs_tree_space_t)o)
								 ->data);
	if (data->common_data.rng)
		ccs_release_object(data->common_data.rng);
	if (data->common_data.tree)
		ccs_release_object(data->common_data.tree);
	if (data->common_data.feature_space)
		ccs_release_object(data->common_data.feature_space);
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_tree_space_static_data(
	_ccs_tree_space_static_data_t   *data,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tree_space_common_data(
		&data->common_data, cum_size, opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_tree_space_static_data(
	_ccs_tree_space_static_data_t   *data,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	CCS_VALIDATE(_ccs_serialize_bin_ccs_tree_space_common_data(
		&data->common_data, buffer_size, buffer, opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_tree_space_static(
	ccs_tree_space_t                 tree_space,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	_ccs_tree_space_static_data_t *data =
		(_ccs_tree_space_static_data_t *)tree_space->data;
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tree_space_static_data(
		data, cum_size, opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_tree_space_static(
	ccs_tree_space_t                 tree_space,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	_ccs_tree_space_static_data_t *data =
		(_ccs_tree_space_static_data_t *)tree_space->data;
	CCS_VALIDATE(_ccs_serialize_bin_ccs_tree_space_static_data(
		data, buffer_size, buffer, opts));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_tree_space_static_serialize_size(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tree_space_static(
			(ccs_tree_space_t)object, cum_size, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_tree_space_static_serialize(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_tree_space_static(
			(ccs_tree_space_t)object, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_tree_space_static_get_node_at_position(
	ccs_tree_space_t tree_space,
	size_t           position_size,
	const size_t    *position,
	ccs_tree_t      *tree_ret)
{
	_ccs_tree_space_static_data_t *data =
		(_ccs_tree_space_static_data_t *)tree_space->data;
	CCS_VALIDATE(ccs_tree_get_node_at_position(
		data->common_data.tree, position_size, position, tree_ret));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_tree_space_static_get_values_at_position(
	ccs_tree_space_t tree_space,
	size_t           position_size,
	const size_t    *position,
	size_t           num_values,
	ccs_datum_t     *values)
{
	_ccs_tree_space_static_data_t *data =
		(_ccs_tree_space_static_data_t *)tree_space->data;
	CCS_VALIDATE(ccs_tree_get_values_at_position(
		data->common_data.tree, position_size, position, num_values,
		values));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_tree_space_static_check_position(
	ccs_tree_space_t tree_space,
	size_t           position_size,
	const size_t    *position,
	ccs_bool_t      *is_valid_ret)
{
	_ccs_tree_space_static_data_t *data =
		(_ccs_tree_space_static_data_t *)tree_space->data;
	CCS_VALIDATE(ccs_tree_position_is_valid(
		data->common_data.tree, position_size, position, is_valid_ret));
	return CCS_RESULT_SUCCESS;
}

static _ccs_tree_space_ops_t _ccs_tree_space_static_ops = {
	{&_ccs_tree_space_static_del, &_ccs_tree_space_static_serialize_size,
	 &_ccs_tree_space_static_serialize},
	&_ccs_tree_space_static_get_node_at_position,
	&_ccs_tree_space_static_get_values_at_position,
	&_ccs_tree_space_static_check_position};

ccs_result_t
ccs_create_static_tree_space(
	const char         *name,
	ccs_tree_t          tree,
	ccs_feature_space_t feature_space,
	ccs_rng_t           rng,
	ccs_tree_space_t   *tree_space_ret)
{
	CCS_CHECK_PTR(name);
	CCS_CHECK_OBJ(tree, CCS_OBJECT_TYPE_TREE);
	if (feature_space)
		CCS_CHECK_OBJ(feature_space, CCS_OBJECT_TYPE_FEATURE_SPACE);
	if (rng)
		CCS_CHECK_OBJ(rng, CCS_OBJECT_TYPE_RNG);
	CCS_CHECK_PTR(tree_space_ret);
	ccs_result_t err;
	uintptr_t    mem = (uintptr_t)calloc(
                1, sizeof(struct _ccs_tree_space_s) +
                           sizeof(struct _ccs_tree_space_static_data_s) +
                           strlen(name) + 1);
	CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	uintptr_t                      mem_orig = mem;

	ccs_tree_space_t               tree_space;
	_ccs_tree_space_static_data_t *data;
	tree_space = (ccs_tree_space_t)mem;
	mem += sizeof(struct _ccs_tree_space_s);
	_ccs_object_init(
		&(tree_space->obj), CCS_OBJECT_TYPE_TREE_SPACE,
		(_ccs_object_ops_t *)&_ccs_tree_space_static_ops);
	data = (struct _ccs_tree_space_static_data_s *)mem;
	mem += sizeof(struct _ccs_tree_space_static_data_s);
	data->common_data.type = CCS_TREE_SPACE_TYPE_STATIC;
	data->common_data.name = (const char *)mem;
	tree_space->data       = (_ccs_tree_space_data_t *)data;
	if (!rng)
		CCS_VALIDATE_ERR_GOTO(err, ccs_create_rng(&rng), errinit);
	else
		CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(rng), errinit);
	data->common_data.rng = rng;
	CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(tree), errinit);
	data->common_data.tree = tree;
	if (feature_space)
		CCS_VALIDATE_ERR_GOTO(
			err, ccs_retain_object(feature_space), errinit);
	data->common_data.feature_space = feature_space;
	strcpy((char *)(data->common_data.name), name);
	*tree_space_ret = tree_space;
	return CCS_RESULT_SUCCESS;
errinit:
	_ccs_tree_space_static_del(tree_space);
	_ccs_object_deinit(&(tree_space->obj));
	free((void *)mem_orig);
	return err;
}
