#ifndef _TREE_TUNER_DESERIALIZE_H
#define _TREE_TUNER_DESERIALIZE_H
#include "tree_tuner_internal.h"
#include "tree_space_deserialize.h"
#include "objective_space_deserialize.h"
#include "tree_evaluation_deserialize.h"

struct _ccs_random_tree_tuner_data_mock_s {
	_ccs_tree_tuner_common_data_t common_data;
	size_t                        size_history;
	size_t                        size_optima;
	ccs_tree_evaluation_t        *history;
	ccs_tree_evaluation_t        *optima;
};
typedef struct _ccs_random_tree_tuner_data_mock_s
	_ccs_random_tree_tuner_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_size_ccs_tree_tuner_common_data(
	_ccs_tree_tuner_common_data_t     *data,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_tree_tuner_type(
		&data->type, buffer_size, buffer));
	CCS_VALIDATE(
		_ccs_deserialize_bin_string(&data->name, buffer_size, buffer));
	CCS_VALIDATE(_ccs_tree_space_deserialize(
		&data->tree_space, CCS_SERIALIZE_FORMAT_BINARY, version,
		buffer_size, buffer, opts));
	CCS_VALIDATE(_ccs_objective_space_deserialize(
		&data->objective_space, CCS_SERIALIZE_FORMAT_BINARY, version,
		buffer_size, buffer, opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_ccs_random_tree_tuner_data(
	_ccs_random_tree_tuner_data_mock_t *data,
	uint32_t                            version,
	size_t                             *buffer_size,
	const char                        **buffer,
	_ccs_object_deserialize_options_t  *opts)
{
	uintptr_t mem;

	CCS_VALIDATE(_ccs_deserialize_bin_size_ccs_tree_tuner_common_data(
		&data->common_data, version, buffer_size, buffer, opts));
	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->size_history, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->size_optima, buffer_size, buffer));

	if (!(data->size_history + data->size_optima))
		return CCS_RESULT_SUCCESS;
	mem = (uintptr_t)calloc(
		(data->size_history + data->size_optima),
		sizeof(ccs_tree_evaluation_t));
	CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);

	data->history = (ccs_tree_evaluation_t *)mem;
	mem += data->size_history * sizeof(ccs_tree_evaluation_t);
	data->optima = (ccs_tree_evaluation_t *)mem;

	for (size_t i = 0; i < data->size_history; i++)
		CCS_VALIDATE(_ccs_tree_evaluation_deserialize(
			data->history + i, CCS_SERIALIZE_FORMAT_BINARY, version,
			buffer_size, buffer, opts));

	for (size_t i = 0; i < data->size_optima; i++)
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_object(
			(ccs_object_t *)data->optima + i, buffer_size, buffer));
	for (size_t i = 0; i < data->size_optima; i++) {
		ccs_datum_t d;
		CCS_VALIDATE(ccs_map_get(
			opts->handle_map, ccs_object(data->optima[i]), &d));
		CCS_REFUTE(
			d.type != CCS_DATA_TYPE_OBJECT,
			CCS_RESULT_ERROR_INVALID_HANDLE);
		data->optima[i] = (ccs_tree_evaluation_t)(d.value.o);
	}
	return CCS_RESULT_SUCCESS;
}

struct _ccs_random_tree_tuner_data_clone_s {
	_ccs_tree_tuner_common_data_t common_data;
	UT_array                     *history;
	UT_array                     *optima;
	UT_array                     *old_optima;
};
typedef struct _ccs_random_tree_tuner_data_clone_s
	_ccs_random_tree_tuner_data_clone_t;

#undef utarray_oom
#define utarray_oom()                                                          \
	{                                                                      \
		CCS_RAISE_ERR_GOTO(                                            \
			res, CCS_RESULT_ERROR_OUT_OF_MEMORY, tuner,            \
			"Out of memory to allocate array");                    \
	}

static inline ccs_result_t
_ccs_deserialize_bin_random_tree_tuner(
	ccs_tree_tuner_t                  *tuner_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	_ccs_random_tree_tuner_data_mock_t data = {
		{(ccs_tree_tuner_type_t)0, NULL, NULL, NULL}, 0, 0, NULL, NULL};
	_ccs_random_tree_tuner_data_clone_t *odata = NULL;
	ccs_result_t                         res   = CCS_RESULT_SUCCESS;
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_ccs_random_tree_tuner_data(
			&data, version, buffer_size, buffer, opts),
		evaluations);
	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_random_tree_tuner(
			data.common_data.name, data.common_data.tree_space,
			data.common_data.objective_space, tuner_ret),
		evaluations);
	odata = (_ccs_random_tree_tuner_data_clone_t *)((*tuner_ret)->data);
	for (size_t i = 0; i < data.size_history; i++)
		utarray_push_back(odata->history, data.history + i);
	for (size_t i = 0; i < data.size_optima; i++)
		utarray_push_back(odata->optima, data.optima + i);
	goto end;
tuner:
	ccs_release_object(*tuner_ret);
	*tuner_ret = NULL;
evaluations:
	if (data.history)
		for (size_t i = 0; i < data.size_history; i++)
			if (data.history[i])
				ccs_release_object(data.history[i]);
end:
	if (data.common_data.tree_space)
		ccs_release_object(data.common_data.tree_space);
	if (data.common_data.objective_space)
		ccs_release_object(data.common_data.objective_space);
	if (data.history)
		free(data.history);
	return res;
}

#undef utarray_oom

struct _ccs_user_defined_tree_tuner_data_mock_s {
	_ccs_random_tree_tuner_data_mock_t base_data;
	_ccs_blob_t                        blob;
};
typedef struct _ccs_user_defined_tree_tuner_data_mock_s
	_ccs_user_defined_tree_tuner_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_ccs_user_defined_tree_tuner_data(
	_ccs_user_defined_tree_tuner_data_mock_t *data,
	uint32_t                                  version,
	size_t                                   *buffer_size,
	const char                              **buffer,
	_ccs_object_deserialize_options_t        *opts)
{
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_random_tree_tuner_data(
		&data->base_data, version, buffer_size, buffer, opts));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_blob(
		&data->blob, buffer_size, buffer));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_user_defined_tree_tuner(
	ccs_tree_tuner_t                  *tuner_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	_ccs_user_defined_tree_tuner_data_mock_t data = {
		{{(ccs_tree_tuner_type_t)0, NULL, NULL, NULL}, 0, 0, NULL, NULL},
		{0, NULL}};
	ccs_user_defined_tree_tuner_vector_t *vector =
		(ccs_user_defined_tree_tuner_vector_t *)opts->vector;
	ccs_result_t res = CCS_RESULT_SUCCESS;
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_ccs_user_defined_tree_tuner_data(
			&data, version, buffer_size, buffer, opts),
		end);
	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_user_defined_tree_tuner(
			data.base_data.common_data.name,
			data.base_data.common_data.tree_space,
			data.base_data.common_data.objective_space, vector,
			opts->data, tuner_ret),
		end);
	if (vector->deserialize_state)
		CCS_VALIDATE_ERR_GOTO(
			res,
			vector->deserialize_state(
				*tuner_ret, data.base_data.size_history,
				data.base_data.history,
				data.base_data.size_optima,
				data.base_data.optima, data.blob.sz,
				data.blob.blob),
			tuner);
	else
		CCS_VALIDATE_ERR_GOTO(
			res,
			vector->tell(
				*tuner_ret, data.base_data.size_history,
				data.base_data.history),
			tuner);
	goto end;
tuner:
	ccs_release_object(*tuner_ret);
	*tuner_ret = NULL;
end:
	if (data.base_data.common_data.tree_space)
		ccs_release_object(data.base_data.common_data.tree_space);
	if (data.base_data.common_data.objective_space)
		ccs_release_object(data.base_data.common_data.objective_space);
	if (data.base_data.history) {
		for (size_t i = 0; i < data.base_data.size_history; i++)
			if (data.base_data.history[i])
				ccs_release_object(data.base_data.history[i]);
		free(data.base_data.history);
	}
	return res;
}

static inline ccs_result_t
_ccs_deserialize_bin_tree_tuner(
	ccs_tree_tuner_t                  *tuner_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	_ccs_object_deserialize_options_t new_opts = *opts;
	_ccs_object_internal_t            obj;
	ccs_object_t                      handle;
	ccs_result_t                      res;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object_internal(
		&obj, buffer_size, buffer, &handle));
	CCS_REFUTE(
		obj.type != CCS_OBJECT_TYPE_TREE_TUNER,
		CCS_RESULT_ERROR_INVALID_TYPE);

	ccs_tree_tuner_type_t ttype;
	CCS_VALIDATE(
		_ccs_peek_bin_ccs_tree_tuner_type(&ttype, buffer_size, buffer));
	if (ttype == CCS_TREE_TUNER_TYPE_USER_DEFINED)
		CCS_CHECK_PTR(opts->vector);

	new_opts.map_values = CCS_TRUE;
	CCS_VALIDATE(ccs_create_map(&new_opts.handle_map));

	switch (ttype) {
	case CCS_TREE_TUNER_TYPE_RANDOM:
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_deserialize_bin_random_tree_tuner(
				tuner_ret, version, buffer_size, buffer,
				&new_opts),
			end);
		break;
	case CCS_TREE_TUNER_TYPE_USER_DEFINED:
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_deserialize_bin_user_defined_tree_tuner(
				tuner_ret, version, buffer_size, buffer,
				&new_opts),
			end);
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_TYPE,
			"Unsupported tuner type: %d", ttype);
	}
	if (opts->handle_map)
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_object_handle_check_add(
				opts->handle_map, handle,
				(ccs_object_t)*tuner_ret),
			err_tuner);

	res = CCS_RESULT_SUCCESS;
	goto end;
err_tuner:
	ccs_release_object(*tuner_ret);
end:
	ccs_release_object(new_opts.handle_map);
	return res;
}

static ccs_result_t
_ccs_tree_tuner_deserialize(
	ccs_tree_tuner_t                  *tuner_ret,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_tree_tuner(
			tuner_ret, version, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_deserialize_user_data(
		(ccs_object_t)*tuner_ret, format, version, buffer_size, buffer,
		opts));
	return CCS_RESULT_SUCCESS;
}

#endif //_TREE_TUNER_DESERIALIZE_H
