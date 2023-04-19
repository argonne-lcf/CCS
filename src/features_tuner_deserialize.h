#ifndef _FEATURES_TUNER_DESERIALIZE_H
#define _FEATURES_TUNER_DESERIALIZE_H
#include "features_tuner_internal.h"
#include "configuration_space_deserialize.h"
#include "objective_space_deserialize.h"
#include "features_space_deserialize.h"
#include "features_evaluation_deserialize.h"

struct _ccs_random_features_tuner_data_mock_s {
	_ccs_features_tuner_common_data_t common_data;
	size_t                            size_history;
	size_t                            size_optimums;
	ccs_features_evaluation_t        *history;
	ccs_features_evaluation_t        *optimums;
};
typedef struct _ccs_random_features_tuner_data_mock_s
	_ccs_random_features_tuner_data_mock_t;

static inline ccs_error_t
_ccs_deserialize_bin_ccs_features_tuner_common_data(
	_ccs_features_tuner_common_data_t *data,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_features_tuner_type(
		&data->type, buffer_size, buffer));
	CCS_VALIDATE(
		_ccs_deserialize_bin_string(&data->name, buffer_size, buffer));
	CCS_VALIDATE(_ccs_configuration_space_deserialize(
		&data->configuration_space, CCS_SERIALIZE_FORMAT_BINARY,
		version, buffer_size, buffer, opts));
	CCS_VALIDATE(_ccs_objective_space_deserialize(
		&data->objective_space, CCS_SERIALIZE_FORMAT_BINARY, version,
		buffer_size, buffer, opts));
	CCS_VALIDATE(_ccs_features_space_deserialize(
		&data->features_space, CCS_SERIALIZE_FORMAT_BINARY, version,
		buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_deserialize_bin_ccs_random_features_tuner_data(
	_ccs_random_features_tuner_data_mock_t *data,
	uint32_t                                version,
	size_t                                 *buffer_size,
	const char                            **buffer,
	_ccs_object_deserialize_options_t      *opts)
{
	uintptr_t mem;

	CCS_VALIDATE(_ccs_deserialize_bin_ccs_features_tuner_common_data(
		&data->common_data, version, buffer_size, buffer, opts));
	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->size_history, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->size_optimums, buffer_size, buffer));

	if (!(data->size_history + data->size_optimums))
		return CCS_SUCCESS;
	mem = (uintptr_t)calloc(
		(data->size_history + data->size_optimums),
		sizeof(ccs_features_evaluation_t));
	CCS_REFUTE(!mem, CCS_OUT_OF_MEMORY);

	data->history = (ccs_features_evaluation_t *)mem;
	mem += data->size_history * sizeof(ccs_features_evaluation_t);
	data->optimums = (ccs_features_evaluation_t *)mem;

	for (size_t i = 0; i < data->size_history; i++)
		CCS_VALIDATE(_ccs_features_evaluation_deserialize(
			data->history + i, CCS_SERIALIZE_FORMAT_BINARY, version,
			buffer_size, buffer, opts));

	for (size_t i = 0; i < data->size_optimums; i++)
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_object(
			(ccs_object_t *)data->optimums + i, buffer_size,
			buffer));
	for (size_t i = 0; i < data->size_optimums; i++) {
		ccs_datum_t d;
		CCS_VALIDATE(ccs_map_get(
			opts->handle_map, ccs_object(data->optimums[i]), &d));
		CCS_REFUTE(d.type != CCS_OBJECT, CCS_INVALID_HANDLE);
		data->optimums[i] = (ccs_features_evaluation_t)(d.value.o);
	}
	return CCS_SUCCESS;
}

struct _ccs_random_features_tuner_data_clone_s {
	_ccs_features_tuner_common_data_t common_data;
	UT_array                         *history;
	UT_array                         *optimums;
	UT_array                         *old_optimums;
};
typedef struct _ccs_random_features_tuner_data_clone_s
	_ccs_random_features_tuner_data_clone_t;

#undef utarray_oom
#define utarray_oom()                                                          \
	{                                                                      \
		CCS_RAISE_ERR_GOTO(                                            \
			res, CCS_OUT_OF_MEMORY, features_tuner,                \
			"Out of memory to allocate array");                    \
	}

static inline ccs_error_t
_ccs_deserialize_bin_random_features_tuner(
	ccs_features_tuner_t              *features_tuner_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	_ccs_random_features_tuner_data_mock_t data = {
		{(ccs_features_tuner_type_t)0, NULL, NULL, NULL, NULL},
		0,
		0,
		NULL,
		NULL};
	_ccs_random_features_tuner_data_clone_t *odata = NULL;
	ccs_error_t                              res   = CCS_SUCCESS;
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_ccs_random_features_tuner_data(
			&data, version, buffer_size, buffer, opts),
		end);
	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_random_features_tuner(
			data.common_data.name,
			data.common_data.configuration_space,
			data.common_data.features_space,
			data.common_data.objective_space, features_tuner_ret),
		features_evaluations);
	odata = (_ccs_random_features_tuner_data_clone_t
			 *)((*features_tuner_ret)->data);
	for (size_t i = 0; i < data.size_history; i++)
		utarray_push_back(odata->history, data.history + i);
	for (size_t i = 0; i < data.size_optimums; i++)
		utarray_push_back(odata->optimums, data.optimums + i);
	goto end;
features_tuner:
	ccs_release_object(*features_tuner_ret);
	*features_tuner_ret = NULL;
features_evaluations:
	for (size_t i = 0; i < data.size_history; i++)
		ccs_release_object(data.history[i]);
end:
	if (data.common_data.configuration_space)
		ccs_release_object(data.common_data.configuration_space);
	if (data.common_data.objective_space)
		ccs_release_object(data.common_data.objective_space);
	if (data.common_data.features_space)
		ccs_release_object(data.common_data.features_space);
	if (data.history)
		free(data.history);
	return res;
}

#undef utarray_oom

struct _ccs_user_defined_features_tuner_data_mock_s {
	_ccs_random_features_tuner_data_mock_t base_data;
	_ccs_blob_t                            blob;
};
typedef struct _ccs_user_defined_features_tuner_data_mock_s
	_ccs_user_defined_features_tuner_data_mock_t;

static inline ccs_error_t
_ccs_deserialize_bin_ccs_user_defined_features_tuner_data(
	_ccs_user_defined_features_tuner_data_mock_t *data,
	uint32_t                                      version,
	size_t                                       *buffer_size,
	const char                                  **buffer,
	_ccs_object_deserialize_options_t            *opts)
{
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_random_features_tuner_data(
		&data->base_data, version, buffer_size, buffer, opts));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_blob(
		&data->blob, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_deserialize_bin_user_defined_features_tuner(
	ccs_features_tuner_t              *features_tuner_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	_ccs_user_defined_features_tuner_data_mock_t data = {
		{{(ccs_features_tuner_type_t)0, NULL, NULL, NULL, NULL},
		 0,
		 0,
		 NULL,
		 NULL},
		{0, NULL}};
	ccs_user_defined_features_tuner_vector_t *vector =
		(ccs_user_defined_features_tuner_vector_t *)opts->vector;
	ccs_error_t res = CCS_SUCCESS;
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_ccs_user_defined_features_tuner_data(
			&data, version, buffer_size, buffer, opts),
		end);
	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_user_defined_features_tuner(
			data.base_data.common_data.name,
			data.base_data.common_data.configuration_space,
			data.base_data.common_data.features_space,
			data.base_data.common_data.objective_space, vector,
			opts->data, features_tuner_ret),
		evaluations);
	if (vector->deserialize_state)
		CCS_VALIDATE_ERR_GOTO(
			res,
			vector->deserialize_state(
				*features_tuner_ret,
				data.base_data.size_history,
				data.base_data.history,
				data.base_data.size_optimums,
				data.base_data.optimums, data.blob.sz,
				data.blob.blob),
			features_tuner);
	else
		CCS_VALIDATE_ERR_GOTO(
			res,
			vector->tell(
				*features_tuner_ret,
				data.base_data.size_history,
				data.base_data.history),
			features_tuner);
	goto evaluations;
features_tuner:
	ccs_release_object(*features_tuner_ret);
	*features_tuner_ret = NULL;
evaluations:
	for (size_t i = 0; i < data.base_data.size_history; i++)
		ccs_release_object(data.base_data.history[i]);
end:
	if (data.base_data.common_data.configuration_space)
		ccs_release_object(
			data.base_data.common_data.configuration_space);
	if (data.base_data.common_data.objective_space)
		ccs_release_object(data.base_data.common_data.objective_space);
	if (data.base_data.common_data.features_space)
		ccs_release_object(data.base_data.common_data.features_space);
	if (data.base_data.history)
		free(data.base_data.history);
	return res;
}

static inline ccs_error_t
_ccs_deserialize_bin_features_tuner(
	ccs_features_tuner_t              *features_tuner_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	_ccs_object_deserialize_options_t new_opts = *opts;
	_ccs_object_internal_t            obj;
	ccs_object_t                      handle;
	ccs_error_t                       res;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object_internal(
		&obj, buffer_size, buffer, &handle));
	CCS_REFUTE(
		obj.type != CCS_OBJECT_TYPE_FEATURES_TUNER, CCS_INVALID_TYPE);

	ccs_features_tuner_type_t ttype;
	CCS_VALIDATE(_ccs_peek_bin_ccs_features_tuner_type(
		&ttype, buffer_size, buffer));
	if (ttype == CCS_FEATURES_TUNER_USER_DEFINED)
		CCS_CHECK_PTR(opts->vector);

	new_opts.map_values = CCS_TRUE;
	CCS_VALIDATE(ccs_create_map(&new_opts.handle_map));

	switch (ttype) {
	case CCS_FEATURES_TUNER_RANDOM:
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_deserialize_bin_random_features_tuner(
				features_tuner_ret, version, buffer_size,
				buffer, &new_opts),
			end);
		break;
	case CCS_FEATURES_TUNER_USER_DEFINED:
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_deserialize_bin_user_defined_features_tuner(
				features_tuner_ret, version, buffer_size,
				buffer, &new_opts),
			end);
		break;
	default:
		CCS_RAISE(
			CCS_UNSUPPORTED_OPERATION,
			"Unsuported features tuner type: %d", ttype);
	}
	if (opts->handle_map)
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_object_handle_check_add(
				opts->handle_map, handle,
				(ccs_object_t)*features_tuner_ret),
			err_features_tuner);

	res = CCS_SUCCESS;
	goto end;
err_features_tuner:
	ccs_release_object(*features_tuner_ret);
end:
	ccs_release_object(new_opts.handle_map);
	return res;
}

static ccs_error_t
_ccs_features_tuner_deserialize(
	ccs_features_tuner_t              *features_tuner_ret,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_features_tuner(
			features_tuner_ret, version, buffer_size, buffer,
			opts));
		break;
	default:
		CCS_RAISE(
			CCS_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_deserialize_user_data(
		(ccs_object_t)*features_tuner_ret, format, version, buffer_size,
		buffer, opts));
	return CCS_SUCCESS;
}

#endif //_FEATURES_TUNER_DESERIALIZE_H
