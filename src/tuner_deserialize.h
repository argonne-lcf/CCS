#ifndef _TUNER_DESERIALIZE_H
#define _TUNER_DESERIALIZE_H
#include "tuner_internal.h"

struct _ccs_random_tuner_data_mock_s {
	_ccs_tuner_common_data_t common_data;
	size_t                   size_history;
	size_t                   size_optima;
	ccs_evaluation_t        *history;
	ccs_evaluation_t        *optima;
};
typedef struct _ccs_random_tuner_data_mock_s _ccs_random_tuner_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_ccs_tuner_common_data(
	_ccs_tuner_common_data_t          *data,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_tuner_type(
		&data->type, buffer_size, buffer));
	CCS_VALIDATE(
		_ccs_deserialize_bin_string(&data->name, buffer_size, buffer));
	CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
		(ccs_object_t *)&data->objective_space,
		CCS_OBJECT_TYPE_OBJECTIVE_SPACE, CCS_SERIALIZE_FORMAT_BINARY,
		version, buffer_size, buffer, opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_ccs_random_tuner_data(
	_ccs_random_tuner_data_mock_t     *data,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	uintptr_t mem;

	CCS_VALIDATE(_ccs_deserialize_bin_ccs_tuner_common_data(
		&data->common_data, version, buffer_size, buffer, opts));
	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->size_history, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->size_optima, buffer_size, buffer));

	if (!(data->size_history + data->size_optima))
		return CCS_RESULT_SUCCESS;
	mem = (uintptr_t)calloc(
		(data->size_history + data->size_optima),
		sizeof(ccs_evaluation_t));
	CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);

	data->history = (ccs_evaluation_t *)mem;
	mem += data->size_history * sizeof(ccs_evaluation_t);
	data->optima = (ccs_evaluation_t *)mem;

	for (size_t i = 0; i < data->size_history; i++)
		CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
			(ccs_object_t *)data->history + i,
			CCS_OBJECT_TYPE_EVALUATION, CCS_SERIALIZE_FORMAT_BINARY,
			version, buffer_size, buffer, opts));

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
		data->optima[i] = (ccs_evaluation_t)(d.value.o);
	}
	return CCS_RESULT_SUCCESS;
}

struct _ccs_random_tuner_data_clone_s {
	_ccs_tuner_common_data_t common_data;
	UT_array                *history;
	UT_array                *optima;
	UT_array                *old_optima;
};
typedef struct _ccs_random_tuner_data_clone_s _ccs_random_tuner_data_clone_t;

#undef utarray_oom
#define utarray_oom()                                                          \
	{                                                                      \
		CCS_RAISE_ERR_GOTO(                                            \
			res, CCS_RESULT_ERROR_OUT_OF_MEMORY, tuner,            \
			"Out of memory to allocate array");                    \
	}

static inline ccs_result_t
_ccs_deserialize_bin_random_tuner(
	ccs_tuner_t                       *tuner_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	_ccs_random_tuner_data_mock_t data = {
		{(ccs_tuner_type_t)0, NULL, NULL, NULL, NULL}, 0, 0, NULL, NULL};
	_ccs_random_tuner_data_clone_t *odata = NULL;
	ccs_result_t                    res   = CCS_RESULT_SUCCESS;
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_ccs_random_tuner_data(
			&data, version, buffer_size, buffer, opts),
		evaluations);
	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_random_tuner(
			data.common_data.name, data.common_data.objective_space,
			tuner_ret),
		evaluations);
	odata = (_ccs_random_tuner_data_clone_t *)((*tuner_ret)->data);
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
	if (data.common_data.objective_space)
		ccs_release_object(data.common_data.objective_space);
	if (data.history)
		free(data.history);
	return res;
}

#undef utarray_oom

struct _ccs_user_defined_tuner_data_mock_s {
	_ccs_random_tuner_data_mock_t base_data;
	_ccs_blob_t                   blob;
};
typedef struct _ccs_user_defined_tuner_data_mock_s
	_ccs_user_defined_tuner_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_ccs_user_defined_tuner_data(
	_ccs_user_defined_tuner_data_mock_t *data,
	uint32_t                             version,
	size_t                              *buffer_size,
	const char                         **buffer,
	_ccs_object_deserialize_options_t   *opts)
{
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_random_tuner_data(
		&data->base_data, version, buffer_size, buffer, opts));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_blob(
		&data->blob, buffer_size, buffer));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_user_defined_tuner(
	ccs_tuner_t                       *tuner_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	_ccs_user_defined_tuner_data_mock_t data = {
		{{(ccs_tuner_type_t)0, NULL, NULL, NULL, NULL},
		 0,
		 0,
		 NULL,
		 NULL},
		{0, NULL}};
	ccs_user_defined_tuner_vector_t *vector =
		(ccs_user_defined_tuner_vector_t *)opts->vector;
	ccs_result_t res = CCS_RESULT_SUCCESS;
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_ccs_user_defined_tuner_data(
			&data, version, buffer_size, buffer, opts),
		end);

	void *tuner_data;
	if (vector->deserialize_state)
		CCS_VALIDATE_ERR_GOTO(
			res,
			vector->deserialize_state(
				data.base_data.common_data.objective_space,
				data.base_data.size_history,
				data.base_data.history,
				data.base_data.size_optima,
				data.base_data.optima, data.blob.sz,
				data.blob.blob, &tuner_data),
			end);
	else
		tuner_data = opts->data;

	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_user_defined_tuner(
			data.base_data.common_data.name,
			data.base_data.common_data.objective_space,
			vector, tuner_data, tuner_ret),
		end);
	if (!vector->deserialize_state)
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
_ccs_deserialize_bin_tuner(
	ccs_tuner_t                       *tuner_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	_ccs_object_deserialize_options_t new_opts = *opts;
	ccs_result_t                      res      = CCS_RESULT_SUCCESS;

	ccs_tuner_type_t                  ttype;
	CCS_VALIDATE(_ccs_peek_bin_ccs_tuner_type(&ttype, buffer_size, buffer));
	if (ttype == CCS_TUNER_TYPE_USER_DEFINED)
		CCS_CHECK_PTR(opts->vector);

	new_opts.map_values = CCS_TRUE;
	CCS_VALIDATE(ccs_create_map(&new_opts.handle_map));

	switch (ttype) {
	case CCS_TUNER_TYPE_RANDOM:
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_deserialize_bin_random_tuner(
				tuner_ret, version, buffer_size, buffer,
				&new_opts),
			end);
		break;
	case CCS_TUNER_TYPE_USER_DEFINED:
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_deserialize_bin_user_defined_tuner(
				tuner_ret, version, buffer_size, buffer,
				&new_opts),
			end);
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_TYPE,
			"Unsupported tuner type: %d", ttype);
	}

end:
	ccs_release_object(new_opts.handle_map);
	return res;
}

static ccs_result_t
_ccs_tuner_deserialize(
	ccs_tuner_t                       *tuner_ret,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_tuner(
			tuner_ret, version, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

#endif //_TUNER_DESERIALIZE_H
