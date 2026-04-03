#ifndef _TREE_DESERIALIZE_H
#define _TREE_DESERIALIZE_H
#include "cconfigspace_internal.h"
#include "cconfigspace_json.h"
#include "tree_internal.h"

struct _ccs_tree_data_mock_s {
	size_t      arity;
	ccs_float_t weight;
	ccs_tree_t *children;
	ccs_float_t bias;
	ccs_datum_t value;
};
typedef struct _ccs_tree_data_mock_s _ccs_tree_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_ccs_tree_data(
	_ccs_tree_data_mock_t             *data,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	_ccs_object_deserialize_options_t new_opts = *opts;
	new_opts.handle_map                        = NULL;
	new_opts.map_values                        = CCS_FALSE;
	CCS_VALIDATE(
		_ccs_deserialize_bin_size(&data->arity, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_float(
		&data->weight, buffer_size, buffer));
	if (data->arity) {
		data->children =
			(ccs_tree_t *)calloc(data->arity, sizeof(ccs_tree_t));
		CCS_REFUTE(!data->children, CCS_RESULT_ERROR_OUT_OF_MEMORY);
		for (size_t i = 0; i < data->arity; i++) {
			ccs_bool_t present;
			CCS_VALIDATE(_ccs_deserialize_bin_ccs_bool(
				&present, buffer_size, buffer));
			if (present)
				CCS_VALIDATE(
					_ccs_object_deserialize_with_opts_check(
						(ccs_object_t *)data->children +
							i,
						CCS_OBJECT_TYPE_TREE,
						CCS_SERIALIZE_FORMAT_BINARY,
						version, buffer_size, buffer,
						&new_opts));
		}
	}
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_float(
		&data->bias, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_datum(
		&data->value, buffer_size, buffer));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_tree(
	ccs_tree_t                        *tree_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	ccs_result_t          res = CCS_RESULT_SUCCESS;

	ccs_tree_t            tree;
	_ccs_tree_data_mock_t data;
	data.children = NULL;
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_ccs_tree_data(
			&data, version, buffer_size, buffer, opts),
		end);
	CCS_VALIDATE_ERR_GOTO(
		res, ccs_create_tree(data.arity, data.value, &tree), end);
	CCS_VALIDATE_ERR_GOTO(
		res, ccs_tree_set_weight(tree, data.weight), err_tree);
	CCS_VALIDATE_ERR_GOTO(
		res, ccs_tree_set_bias(tree, data.bias), err_tree);
	for (size_t i = 0; i < data.arity; i++)
		if (data.children[i])
			CCS_VALIDATE_ERR_GOTO(
				res,
				ccs_tree_set_child(tree, i, data.children[i]),
				err_tree);

	*tree_ret = tree;
	goto end;
err_tree:
	ccs_release_object(tree);
end:
	if (data.children) {
		for (size_t i = 0; i < data.arity; i++)
			if (data.children[i])
				ccs_release_object(data.children[i]);
		free(data.children);
	}
	return res;
}

static inline ccs_result_t
_ccs_deserialize_json_tree(
	ccs_tree_t                        *tree_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	_ccs_object_deserialize_options_t new_opts = *opts;
	ccs_result_t                      res      = CCS_RESULT_SUCCESS;
	cJSON                            *json;
	cJSON                            *j_children;
	ccs_tree_t                        tree = NULL;
	_ccs_tree_data_mock_t             data;
	int                               num_children;
	int                               i;
	ccs_int_t                         arity_val;

	(void)buffer_size;
	new_opts.handle_map = NULL;
	new_opts.map_values = CCS_FALSE;
	data.children       = NULL;

	json                = *(cJSON **)buffer;
	CCS_VALIDATE(_ccs_json_extract_int(json, "arity", &arity_val));
	data.arity = (size_t)arity_val;

	CCS_VALIDATE(_ccs_json_extract_float(json, "weight", &data.weight));

	CCS_VALIDATE(_ccs_json_extract_float(json, "bias", &data.bias));

	CCS_VALIDATE_ERR_GOTO(
		res, _ccs_json_extract_datum(json, "value", &data.value), end);

	j_children = cJSON_GetObjectItemCaseSensitive(json, "children");
	CCS_REFUTE_ERR_GOTO(
		res, !j_children || !cJSON_IsArray(j_children),
		CCS_RESULT_ERROR_INVALID_VALUE, end);
	num_children = cJSON_GetArraySize(j_children);
	CCS_REFUTE_ERR_GOTO(
		res, (size_t)num_children != data.arity,
		CCS_RESULT_ERROR_INVALID_VALUE, end);

	if (data.arity) {
		data.children =
			(ccs_tree_t *)calloc(data.arity, sizeof(ccs_tree_t));
		CCS_REFUTE_ERR_GOTO(
			res, !data.children, CCS_RESULT_ERROR_OUT_OF_MEMORY,
			end);
		for (i = 0; i < num_children; i++) {
			cJSON *child_item = cJSON_GetArrayItem(j_children, i);
			if (!cJSON_IsNull(child_item))
				CCS_VALIDATE_ERR_GOTO(
					res,
					_ccs_json_deserialize_array_object(
						child_item,
						CCS_OBJECT_TYPE_TREE, version,
						(ccs_object_t *)data.children +
							i,
						&new_opts),
					end);
		}
	}

	CCS_VALIDATE_ERR_GOTO(
		res, ccs_create_tree(data.arity, data.value, &tree), end);
	CCS_VALIDATE_ERR_GOTO(
		res, ccs_tree_set_weight(tree, data.weight), err_tree);
	CCS_VALIDATE_ERR_GOTO(
		res, ccs_tree_set_bias(tree, data.bias), err_tree);
	for (i = 0; (size_t)i < data.arity; i++)
		if (data.children[i])
			CCS_VALIDATE_ERR_GOTO(
				res,
				ccs_tree_set_child(
					tree, (size_t)i, data.children[i]),
				err_tree);

	*tree_ret = tree;
	goto end;
err_tree:
	ccs_release_object(tree);
end:
	if (data.children) {
		for (i = 0; (size_t)i < data.arity; i++)
			if (data.children[i])
				ccs_release_object(data.children[i]);
		free(data.children);
	}
	return res;
}

static ccs_result_t
_ccs_tree_deserialize(
	ccs_tree_t                        *tree_ret,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_tree(
			tree_ret, version, buffer_size, buffer, opts));
		break;
	case CCS_SERIALIZE_FORMAT_JSON:
		CCS_VALIDATE(_ccs_deserialize_json_tree(
			tree_ret, version, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

#endif //_TREE_DESERIALIZE_H
