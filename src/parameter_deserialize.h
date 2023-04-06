#ifndef _PARAMETER_DESERIALIZE_H
#define _PARAMETER_DESERIALIZE_H
#include "cconfigspace_internal.h"
#include "parameter_internal.h"

static inline ccs_error_t
_ccs_deserialize_bin_parameter_numerical(
		ccs_parameter_t    *parameter_ret,
		uint32_t                 version,
		size_t                  *buffer_size,
		const char             **buffer) {
	(void)version;
	_ccs_parameter_numerical_data_t data;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_parameter_numerical_data(
		&data, buffer_size, buffer));
	CCS_VALIDATE(ccs_create_numerical_parameter(
		data.common_data.name, data.common_data.interval.type,
		(data.common_data.interval.type == CCS_NUM_FLOAT ?
			CCSF(data.common_data.interval.lower.f) :
			CCSI(data.common_data.interval.lower.i)),
		(data.common_data.interval.type == CCS_NUM_FLOAT ?
			CCSF(data.common_data.interval.upper.f) :
			CCSI(data.common_data.interval.upper.i)),
		(data.common_data.interval.type == CCS_NUM_FLOAT ?
			CCSF(data.quantization.f) :
			CCSI(data.quantization.i)),
		(data.common_data.interval.type == CCS_NUM_FLOAT ?
			CCSF(data.common_data.default_value.value.f) :
			CCSI(data.common_data.default_value.value.i)),
		parameter_ret));
	return CCS_SUCCESS;
}

struct _ccs_parameter_categorical_data_mock_s {
	_ccs_parameter_common_data_t  common_data;
	size_t                             num_possible_values;
	ccs_datum_t                       *possible_values;
};
typedef struct _ccs_parameter_categorical_data_mock_s _ccs_parameter_categorical_data_mock_t;

static inline ccs_error_t
_ccs_deserialize_bin_ccs_parameter_categorical_data(
		_ccs_parameter_categorical_data_mock_t  *data,
		size_t                                       *buffer_size,
		const char                                  **buffer) {
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_parameter_common_data(
		&data->common_data, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->num_possible_values, buffer_size, buffer));
	data->possible_values =
		(ccs_datum_t *)malloc(data->num_possible_values*sizeof(ccs_datum_t));
	CCS_REFUTE(!data->possible_values, CCS_OUT_OF_MEMORY);
	for (size_t i = 0; i < data->num_possible_values; i++)
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_datum(
			data->possible_values + i, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_deserialize_bin_parameter_categorical(
		ccs_parameter_t    *parameter_ret,
		uint32_t                 version,
		size_t                  *buffer_size,
		const char             **buffer) {
	(void)version;
	ccs_error_t res = CCS_SUCCESS;
	int found = 0;
	_ccs_parameter_categorical_data_mock_t data;
	data.possible_values = NULL;
	CCS_VALIDATE_ERR_GOTO(res, _ccs_deserialize_bin_ccs_parameter_categorical_data(
		&data, buffer_size, buffer), end);
	size_t default_value_index;
	for (size_t i = 0; i < data.num_possible_values; i++)
		if (!ccs_datum_cmp(data.common_data.default_value, data.possible_values[i])) {
			found = 1;
			default_value_index = i;
		}
	CCS_REFUTE_ERR_GOTO(res, !found, CCS_INVALID_VALUE, end);

	switch (data.common_data.type) {
	case CCS_PARAMETER_TYPE_CATEGORICAL:
		CCS_VALIDATE_ERR_GOTO(res, ccs_create_categorical_parameter(
			data.common_data.name,
			data.num_possible_values,
			data.possible_values,
			default_value_index,
			parameter_ret), end);
		break;
	case CCS_PARAMETER_TYPE_ORDINAL:
		CCS_VALIDATE_ERR_GOTO(res, ccs_create_ordinal_parameter(
			data.common_data.name,
			data.num_possible_values,
			data.possible_values,
			default_value_index,
			parameter_ret), end);
		break;
	case CCS_PARAMETER_TYPE_DISCRETE:
		CCS_VALIDATE_ERR_GOTO(res, ccs_create_discrete_parameter(
			data.common_data.name,
			data.num_possible_values,
			data.possible_values,
			default_value_index,
			parameter_ret), end);
		break;
	default:
		CCS_RAISE_ERR_GOTO(res, CCS_INVALID_TYPE, end, "Unsupport parameter type: %d", data.common_data.type);
	}
end:
	if (data.possible_values)
		free(data.possible_values);
	return res;
}

typedef _ccs_parameter_common_data_t _ccs_parameter_string_data_mock_t;

static inline ccs_error_t
_ccs_deserialize_bin_ccs_parameter_string_data(
		_ccs_parameter_string_data_mock_t  *data,
		size_t                                  *buffer_size,
		const char                             **buffer) {
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_parameter_common_data(
		data, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_deserialize_bin_parameter_string(
		ccs_parameter_t  *parameter_ret,
		uint32_t               version,
		size_t                *buffer_size,
		const char           **buffer) {
	(void)version;
	_ccs_parameter_string_data_mock_t data;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_parameter_string_data(
		&data, buffer_size, buffer));
	CCS_VALIDATE(ccs_create_string_parameter(
		data.name,
		parameter_ret));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_deserialize_bin_parameter(
		ccs_parameter_t               *parameter_ret,
		uint32_t                            version,
		size_t                             *buffer_size,
		const char                        **buffer,
		_ccs_object_deserialize_options_t  *opts) {
	_ccs_object_internal_t obj;
	ccs_object_t handle;
	ccs_error_t res;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object_internal(
		&obj, buffer_size, buffer, &handle));
	CCS_REFUTE(obj.type != CCS_PARAMETER, CCS_INVALID_TYPE);

	ccs_parameter_type_t htype;
	CCS_VALIDATE(_ccs_peek_bin_ccs_parameter_type(
		&htype, buffer_size, buffer));
	switch (htype) {
	case CCS_PARAMETER_TYPE_NUMERICAL:
		CCS_VALIDATE(_ccs_deserialize_bin_parameter_numerical(
			parameter_ret, version, buffer_size, buffer));
		break;
	case CCS_PARAMETER_TYPE_CATEGORICAL:
	case CCS_PARAMETER_TYPE_ORDINAL:
	case CCS_PARAMETER_TYPE_DISCRETE:
		CCS_VALIDATE(_ccs_deserialize_bin_parameter_categorical(
			parameter_ret, version, buffer_size, buffer));
		break;
	case CCS_PARAMETER_TYPE_STRING:
		CCS_VALIDATE(_ccs_deserialize_bin_parameter_string(
			parameter_ret, version, buffer_size, buffer));
		break;
	default:
		CCS_RAISE(CCS_INVALID_TYPE, "Unsupport parameter type: %d", htype);
	}
	if (opts->handle_map)
		CCS_VALIDATE_ERR_GOTO(res,
			_ccs_object_handle_check_add(
				opts->handle_map, handle,
				(ccs_object_t)*parameter_ret),
			err_parameter);

	return CCS_SUCCESS;
err_parameter:
	ccs_release_object(*parameter_ret);
	return res;
}

static ccs_error_t
_ccs_parameter_deserialize(
		ccs_parameter_t               *parameter_ret,
		ccs_serialize_format_t              format,
		uint32_t                            version,
		size_t                             *buffer_size,
		const char                        **buffer,
		_ccs_object_deserialize_options_t  *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_parameter(
			parameter_ret, version, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(CCS_INVALID_VALUE, "Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_deserialize_user_data(
		(ccs_object_t)*parameter_ret, format, version, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

#endif //_PARAMETER_DESERIALIZE_H
