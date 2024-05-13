#ifndef _TREE_CONFIGURATION_DESERIALIZE_H
#define _TREE_CONFIGURATION_DESERIALIZE_H
#include "cconfigspace_internal.h"
#include "tree_configuration_internal.h"

struct _ccs_tree_configuration_data_mock_s {
	ccs_tree_space_t tree_space;
	size_t           position_size;
	size_t          *position;
	ccs_bool_t       features_present;
	ccs_features_t   features;
};
typedef struct _ccs_tree_configuration_data_mock_s
	_ccs_tree_configuration_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_tree_configuration_data(
	_ccs_tree_configuration_data_mock_t *data,
	uint32_t                             version,
	size_t                              *buffer_size,
	const char                         **buffer,
	_ccs_object_deserialize_options_t   *opts)
{
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object(
		(ccs_object_t *)&data->tree_space, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->position_size, buffer_size, buffer));
	if (data->position_size) {
		data->position =
			(size_t *)malloc(data->position_size * sizeof(size_t));
		CCS_REFUTE(!data->position, CCS_RESULT_ERROR_OUT_OF_MEMORY);
		for (size_t i = 0; i < data->position_size; i++)
			CCS_VALIDATE(_ccs_deserialize_bin_size(
				data->position + i, buffer_size, buffer));
	}
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_bool(
		&data->features_present, buffer_size, buffer));
	if (data->features_present)
		CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
			(ccs_object_t *)&data->features,
			CCS_OBJECT_TYPE_FEATURES, CCS_SERIALIZE_FORMAT_BINARY,
			version, buffer_size, buffer, opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_tree_configuration(
	ccs_tree_configuration_t          *configuration_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	CCS_CHECK_OBJ(opts->handle_map, CCS_OBJECT_TYPE_MAP);
	_ccs_object_deserialize_options_t new_opts = *opts;
	_ccs_object_internal_t            obj;
	ccs_object_t                      handle;
	ccs_datum_t                       d;
	ccs_tree_space_t                  tree_space;
	ccs_tree_configuration_t          configuration;
	ccs_result_t                      res = CCS_RESULT_SUCCESS;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object_internal(
		&obj, buffer_size, buffer, &handle));
	CCS_REFUTE(
		obj.type != CCS_OBJECT_TYPE_TREE_CONFIGURATION,
		CCS_RESULT_ERROR_INVALID_TYPE);

	new_opts.map_values                      = CCS_FALSE;
	_ccs_tree_configuration_data_mock_t data = {
		NULL, 0, NULL, CCS_FALSE, NULL};
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_tree_configuration_data(
			&data, version, buffer_size, buffer, &new_opts),
		end);

	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_map_get(opts->handle_map, ccs_object(data.tree_space), &d),
		end);
	CCS_REFUTE_ERR_GOTO(
		res, d.type != CCS_DATA_TYPE_OBJECT,
		CCS_RESULT_ERROR_INVALID_HANDLE, end);
	tree_space = (ccs_tree_space_t)(d.value.o);

	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_tree_configuration(
			tree_space, data.features, data.position_size,
			data.position, &configuration),
		end);

	if (opts->map_values)
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_object_handle_check_add(
				opts->handle_map, handle,
				(ccs_object_t)configuration),
			err_configuration);
	*configuration_ret = configuration;
	goto end;

err_configuration:
	ccs_release_object(configuration);
end:
	if (data.features)
		ccs_release_object(data.features);
	if (data.position)
		free(data.position);
	return res;
}

static ccs_result_t
_ccs_tree_configuration_deserialize(
	ccs_tree_configuration_t          *configuration_ret,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_tree_configuration(
			configuration_ret, version, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_deserialize_user_data(
		(ccs_object_t)*configuration_ret, format, version, buffer_size,
		buffer, opts));
	return CCS_RESULT_SUCCESS;
}

#endif //_TREE_CONFIGURATION_DESERIALIZE_H
