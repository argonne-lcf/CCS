#ifndef _TREE_SPACE_DESERIALIZE_H
#define _TREE_SPACE_DESERIALIZE_H
#include "tree_space_internal.h"
#include "cconfigspace_json.h"

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
	if (opts->map_values && data->feature_space_handle)
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_object_handle_check_add(
				opts->handle_map, data->feature_space_handle,
				(ccs_object_t)data->feature_space),
			err_tree_space);
	goto end;
err_tree_space:
	ccs_release_object(*tree_space_ret);
	*tree_space_ret = NULL;
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
	if (opts->map_values && data->feature_space_handle)
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_object_handle_check_add(
				opts->handle_map, data->feature_space_handle,
				(ccs_object_t)data->feature_space),
			err_tree_space);
	goto end;
err_tree_space:
	ccs_release_object(*tree_space_ret);
	*tree_space_ret = NULL;
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
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_json_ccs_tree_space_common_data(
	_ccs_tree_space_common_data_mock_t *data,
	uint32_t                            version,
	cJSON                              *json,
	_ccs_object_deserialize_options_t  *opts)
{
	_ccs_object_deserialize_options_t new_opts = *opts;
	cJSON                            *j_fs_handle;
	const char                       *type_str;

	new_opts.handle_map = NULL;
	new_opts.map_values = CCS_FALSE;
	CCS_VALIDATE(
		_ccs_json_extract_string(json, "tree_space_type", &type_str));
	CCS_VALIDATE(
		_ccs_json_tree_space_type_from_string(type_str, &data->type));

	CCS_VALIDATE(_ccs_json_extract_string(json, "name", &data->name));

	CCS_VALIDATE(_ccs_json_extract_object(
		json, "rng", CCS_OBJECT_TYPE_RNG, version,
		(ccs_object_t *)&data->rng, &new_opts));

	CCS_VALIDATE(_ccs_json_extract_object(
		json, "tree", CCS_OBJECT_TYPE_TREE, version,
		(ccs_object_t *)&data->tree, &new_opts));

	j_fs_handle =
		cJSON_GetObjectItemCaseSensitive(json, "feature_space_handle");
	if (j_fs_handle) {
		const char *fs_handle_str;
		CCS_VALIDATE(_ccs_json_get_string(j_fs_handle, &fs_handle_str));
		CCS_REFUTE(
			strlen(fs_handle_str) != sizeof(ccs_object_t) * 2,
			CCS_RESULT_ERROR_INVALID_VALUE);
		CCS_REFUTE(
			_ccs_json_hex_decode_buf(
				fs_handle_str, sizeof(ccs_object_t) * 2,
				&data->feature_space_handle),
			CCS_RESULT_ERROR_INVALID_VALUE);
		CCS_VALIDATE(_ccs_json_extract_object(
			json, "feature_space", CCS_OBJECT_TYPE_FEATURE_SPACE,
			version, (ccs_object_t *)&data->feature_space, opts));
	}

	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_json_tree_space_static(
	ccs_tree_space_t                   *tree_space_ret,
	uint32_t                            version,
	cJSON                              *json,
	_ccs_object_deserialize_options_t  *opts,
	_ccs_tree_space_common_data_mock_t *data)
{
	ccs_result_t res = CCS_RESULT_SUCCESS;
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_json_ccs_tree_space_common_data(
			data, version, json, opts),
		end);
	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_static_tree_space(
			data->name, data->tree, data->feature_space, data->rng,
			tree_space_ret),
		end);
	if (opts->map_values && data->feature_space_handle)
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_object_handle_check_add(
				opts->handle_map, data->feature_space_handle,
				(ccs_object_t)data->feature_space),
			err_tree_space);
	goto end;
err_tree_space:
	ccs_release_object(*tree_space_ret);
	*tree_space_ret = NULL;
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
_ccs_deserialize_json_tree_space_dynamic(
	ccs_tree_space_t                   *tree_space_ret,
	uint32_t                            version,
	cJSON                              *json,
	_ccs_object_deserialize_options_t  *opts,
	_ccs_tree_space_common_data_mock_t *data)
{
	ccs_dynamic_tree_space_vector_t *vector          = NULL;
	void                            *tree_space_data = NULL;
	size_t                           blob_sz         = 0;
	unsigned char                   *blob_data       = NULL;
	ccs_result_t                     res             = CCS_RESULT_SUCCESS;
	cJSON                           *j_state;

	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_json_ccs_tree_space_common_data(
			data, version, json, opts),
		end);

	j_state = cJSON_GetObjectItemCaseSensitive(json, "user_state");
	if (j_state) {
		const char *state_str;
		CCS_VALIDATE_ERR_GOTO(
			res, _ccs_json_get_string(j_state, &state_str), end);
		blob_data = _ccs_json_hex_decode(state_str, &blob_sz);
		CCS_REFUTE_ERR_GOTO(
			res, !blob_data, CCS_RESULT_ERROR_OUT_OF_MEMORY, end);
	}

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
				data->tree, data->feature_space, blob_sz,
				blob_data, &tree_space_data),
			end);

	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_dynamic_tree_space(
			data->name, data->tree, data->feature_space, data->rng,
			vector, tree_space_data, tree_space_ret),
		end);
	if (opts->map_values && data->feature_space_handle)
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_object_handle_check_add(
				opts->handle_map, data->feature_space_handle,
				(ccs_object_t)data->feature_space),
			err_tree_space);
	goto end;
err_tree_space:
	ccs_release_object(*tree_space_ret);
	*tree_space_ret = NULL;
end:
	if (blob_data)
		free(blob_data);
	if (data->feature_space)
		ccs_release_object(data->feature_space);
	if (data->rng)
		ccs_release_object(data->rng);
	if (data->tree)
		ccs_release_object(data->tree);
	return res;
}

static inline ccs_result_t
_ccs_deserialize_json_tree_space(
	ccs_tree_space_t                  *tree_space_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	cJSON                             *json;
	ccs_tree_space_type_t              stype;
	_ccs_tree_space_common_data_mock_t data = {
		CCS_TREE_SPACE_TYPE_STATIC, NULL, NULL, NULL, NULL, NULL};
	const char *stype_str;

	(void)buffer_size;
	json = *(cJSON **)buffer;
	CCS_VALIDATE(
		_ccs_json_extract_string(json, "tree_space_type", &stype_str));
	CCS_VALIDATE(_ccs_json_tree_space_type_from_string(stype_str, &stype));

	if (stype == CCS_TREE_SPACE_TYPE_DYNAMIC)
		CCS_CHECK_PTR(opts->deserialize_vector_callback);

	switch (stype) {
	case CCS_TREE_SPACE_TYPE_STATIC:
		CCS_VALIDATE(_ccs_deserialize_json_tree_space_static(
			tree_space_ret, version, json, opts, &data));
		break;
	case CCS_TREE_SPACE_TYPE_DYNAMIC:
		CCS_VALIDATE(_ccs_deserialize_json_tree_space_dynamic(
			tree_space_ret, version, json, opts, &data));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_UNSUPPORTED_OPERATION,
			"Unsupported tree space type: %d", stype);
	}
	return CCS_RESULT_SUCCESS;
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
	case CCS_SERIALIZE_FORMAT_JSON:
		CCS_VALIDATE(_ccs_deserialize_json_tree_space(
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
