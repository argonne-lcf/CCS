#ifndef _TREE_SPACE_DESERIALIZE_H
#define _TREE_SPACE_DESERIALIZE_H
#include "tree_space_internal.h"

struct _ccs_tree_space_common_data_mock_s {
	ccs_tree_space_type_t type;
	const char           *name;
	ccs_rng_t             rng;
	ccs_tree_t            tree;
	ccs_object_t          feature_space_handle;
	ccs_feature_space_t   feature_space;
};
typedef struct _ccs_tree_space_common_data_mock_s
	_ccs_tree_space_common_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_ccs_tree_space_common_data(
	_ccs_tree_space_common_data_mock_t *data,
	uint32_t                            version,
	size_t                             *buffer_size,
	const char                        **buffer,
	_ccs_object_deserialize_options_t  *opts)
{
	_ccs_object_deserialize_options_t new_opts = *opts;
	new_opts.handle_map                        = NULL;
	new_opts.map_values                        = CCS_FALSE;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_tree_space_type(
		&data->type, buffer_size, buffer));
	CCS_VALIDATE(
		_ccs_deserialize_bin_string(&data->name, buffer_size, buffer));
	CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
		(ccs_object_t *)&data->rng, CCS_OBJECT_TYPE_RNG,
		CCS_SERIALIZE_FORMAT_BINARY, version, buffer_size, buffer,
		&new_opts));
	CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
		(ccs_object_t *)&data->tree, CCS_OBJECT_TYPE_TREE,
		CCS_SERIALIZE_FORMAT_BINARY, version, buffer_size, buffer,
		&new_opts));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object(
		&data->feature_space_handle, buffer_size, buffer));
	if (data->feature_space_handle) {
		CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
			(ccs_object_t *)&data->feature_space,
			CCS_OBJECT_TYPE_FEATURE_SPACE,
			CCS_SERIALIZE_FORMAT_BINARY, version, buffer_size,
			buffer, opts));
	}
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_tree_space_static(
	ccs_tree_space_t                   *tree_space_ret,
	uint32_t                            version,
	size_t                             *buffer_size,
	const char                        **buffer,
	_ccs_object_deserialize_options_t  *opts,
	_ccs_tree_space_common_data_mock_t *data)
{
	ccs_result_t res = CCS_RESULT_SUCCESS;
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_ccs_tree_space_common_data(
			data, version, buffer_size, buffer, opts),
		end);
	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_static_tree_space(
			data->name, data->tree, data->feature_space, data->rng,
			tree_space_ret),
		end);
end:
	if (data->feature_space)
		ccs_release_object(data->feature_space);
	if (data->rng)
		ccs_release_object(data->rng);
	if (data->tree)
		ccs_release_object(data->tree);
	return res;
}

static inline ccs_result_t
_ccs_deserialize_bin_tree_space_dynamic(
	ccs_tree_space_t                   *tree_space_ret,
	uint32_t                            version,
	size_t                             *buffer_size,
	const char                        **buffer,
	_ccs_object_deserialize_options_t  *opts,
	_ccs_tree_space_common_data_mock_t *data)
{
	_ccs_blob_t                      blob            = {0, NULL};
	ccs_dynamic_tree_space_vector_t *vector          = NULL;
	void                            *tree_space_data = NULL;
	ccs_result_t                     res             = CCS_RESULT_SUCCESS;

	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_ccs_tree_space_common_data(
			data, version, buffer_size, buffer, opts),
		end);
	CCS_VALIDATE_ERR_GOTO(
		res, _ccs_deserialize_bin_ccs_blob(&blob, buffer_size, buffer),
		end);

	CCS_VALIDATE_ERR_GOTO(
		res,
		opts->deserialize_vector_callback(
			CCS_OBJECT_TYPE_TREE_SPACE, data->name,
			opts->deserialize_vector_user_data, (void **)&vector,
			&tree_space_data),
		end);

	if (vector->deserialize_state)
		CCS_VALIDATE_ERR_GOTO(
			res,
			vector->deserialize_state(
				data->tree, data->feature_space, blob.sz,
				blob.blob, &tree_space_data),
			end);

	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_dynamic_tree_space(
			data->name, data->tree, data->feature_space, data->rng,
			vector, tree_space_data, tree_space_ret),
		end);
end:
	if (data->feature_space)
		ccs_release_object(data->feature_space);
	if (data->rng)
		ccs_release_object(data->rng);
	if (data->tree)
		ccs_release_object(data->tree);
	return res;
}

static inline ccs_result_t
_ccs_deserialize_bin_tree_space(
	ccs_tree_space_t                  *tree_space_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	ccs_result_t          res = CCS_RESULT_SUCCESS;

	ccs_tree_space_type_t stype;
	CCS_VALIDATE(
		_ccs_peek_bin_ccs_tree_space_type(&stype, buffer_size, buffer));
	if (stype == CCS_TREE_SPACE_TYPE_DYNAMIC)
		CCS_CHECK_PTR(opts->deserialize_vector_callback);

	_ccs_tree_space_common_data_mock_t data = {
		CCS_TREE_SPACE_TYPE_STATIC, NULL, NULL, NULL, NULL, NULL};

	switch (stype) {
	case CCS_TREE_SPACE_TYPE_STATIC:
		CCS_VALIDATE(_ccs_deserialize_bin_tree_space_static(
			tree_space_ret, version, buffer_size, buffer, opts,
			&data));
		break;
	case CCS_TREE_SPACE_TYPE_DYNAMIC:
		CCS_VALIDATE(_ccs_deserialize_bin_tree_space_dynamic(
			tree_space_ret, version, buffer_size, buffer, opts,
			&data));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_UNSUPPORTED_OPERATION,
			"Unsupported tree space type: %d", stype);
	}
	if (opts->map_values) {
		if (data.feature_space_handle)
			CCS_VALIDATE_ERR_GOTO(
				res,
				_ccs_object_handle_check_add(
					opts->handle_map,
					data.feature_space_handle,
					(ccs_object_t)data.feature_space),
				err_tree_space);
	}
	return CCS_RESULT_SUCCESS;
err_tree_space:
	ccs_release_object(*tree_space_ret);
	*tree_space_ret = NULL;
	return res;
}

static ccs_result_t
_ccs_tree_space_deserialize(
	ccs_tree_space_t                  *tree_space_ret,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_tree_space(
			tree_space_ret, version, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

#endif //_TREE_SPACE_DESERIALIZE_H
