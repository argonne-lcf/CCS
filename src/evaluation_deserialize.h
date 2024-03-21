#ifndef _EVALUATION_DESERIALIZE_H
#define _EVALUATION_DESERIALIZE_H
#include "cconfigspace_internal.h"
#include "evaluation_internal.h"
#include "configuration_deserialize.h"

static inline ccs_result_t
_ccs_deserialize_bin_ccs_evaluation_data(
	_ccs_evaluation_data_t            *data,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object(
		(ccs_object_t *)&data->objective_space, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->num_values, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->num_bindings, buffer_size, buffer));
	CCS_VALIDATE(_ccs_configuration_deserialize(
		&data->configuration, CCS_SERIALIZE_FORMAT_BINARY, version,
		buffer_size, buffer, opts));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_evaluation_result(
		&data->result, buffer_size, buffer));
	if (!(data->num_values || data->num_bindings))
		return CCS_RESULT_SUCCESS;

	uintptr_t mem = (uintptr_t)calloc(
		data->num_values * sizeof(ccs_datum_t) +
			data->num_bindings * sizeof(ccs_binding_t),
		1);
	CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	data->values = (ccs_datum_t *)mem;
	mem += data->num_values * sizeof(ccs_datum_t);
	data->bindings = (ccs_binding_t *)mem;
	mem += data->num_bindings * sizeof(ccs_binding_t);

	for (size_t i = 0; i < data->num_values; i++)
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_datum(
			data->values + i, buffer_size, buffer));

	for (size_t i = 0; i < data->num_bindings; i++)
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_object(
			(ccs_object_t *)data->bindings + i, buffer_size, buffer));

	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_ccs_evaluation(
	ccs_evaluation_t                  *evaluation_ret,
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
	ccs_objective_space_t             os;
	ccs_evaluation_t                  evaluation;
	ccs_result_t                      res = CCS_RESULT_SUCCESS;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object_internal(
		&obj, buffer_size, buffer, &handle));
	CCS_REFUTE(
		obj.type != CCS_OBJECT_TYPE_EVALUATION,
		CCS_RESULT_ERROR_INVALID_TYPE);

	new_opts.map_values         = CCS_FALSE;
	_ccs_evaluation_data_t data = {
		NULL, 0, NULL, NULL, CCS_RESULT_SUCCESS, 0, NULL};
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_ccs_evaluation_data(
			&data, version, buffer_size, buffer, &new_opts),
		end);

	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_map_get(opts->handle_map,
			ccs_object(data.objective_space), &d),
		end);
	CCS_REFUTE_ERR_GOTO(
		res, d.type != CCS_DATA_TYPE_OBJECT,
		CCS_RESULT_ERROR_INVALID_HANDLE, end);
	os = (ccs_objective_space_t)(d.value.o);
	for (size_t i = 0; i < data.num_bindings; i++) {
		CCS_VALIDATE_ERR_GOTO(
			res,
			ccs_map_get(opts->handle_map,
				ccs_object(data.bindings[i]), &d),
		end);
		CCS_REFUTE_ERR_GOTO(
			res,
			d.type != CCS_DATA_TYPE_OBJECT,
			CCS_RESULT_ERROR_INVALID_HANDLE,
			end);
		data.bindings[i] = (ccs_binding_t)(d.value.o);
	}

	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_evaluation(
			os, data.configuration, data.result,
			data.num_values, data.values,
			data.num_bindings, data.bindings,
			&evaluation),
		end);

	if (opts->map_values)
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_object_handle_check_add(
				opts->handle_map, handle,
				(ccs_object_t)evaluation),
			err_evaluation);
	*evaluation_ret = evaluation;
	goto end;

err_evaluation:
	ccs_release_object(evaluation);
end:
	if (data.configuration)
		ccs_release_object(data.configuration);
	if (data.values)
		free(data.values);
	return res;
}

static ccs_result_t
_ccs_evaluation_deserialize(
	ccs_evaluation_t                  *evaluation_ret,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_evaluation(
			evaluation_ret, version, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_deserialize_user_data(
		(ccs_object_t)*evaluation_ret, format, version, buffer_size,
		buffer, opts));
	return CCS_RESULT_SUCCESS;
}

#endif //_EVALUATION_DESERIALIZE_H
