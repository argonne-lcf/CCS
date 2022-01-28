#ifndef _HYPERPARAMETER_DESERIALIZE_H
#define _HYPERPARAMETER_DESERIALIZE_H
#include "cconfigspace_internal.h"
#include "hyperparameter_internal.h"

static inline ccs_result_t
_ccs_deserialize_bin_hyperparameter_numerical(
		ccs_hyperparameter_t    *hyperparameter_ret,
		uint32_t                 version,
		size_t                  *buffer_size,
		const char             **buffer) {
	(void)version;
	_ccs_hyperparameter_numerical_data_t data;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_hyperparameter_numerical_data(
		&data, buffer_size, buffer));
	CCS_VALIDATE(ccs_create_numerical_hyperparameter(
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
		NULL, hyperparameter_ret));
	return CCS_SUCCESS;
}

struct _ccs_hyperparameter_categorical_data_mock_s {
	_ccs_hyperparameter_common_data_t  common_data;
	size_t                             num_possible_values;
	ccs_datum_t                       *possible_values;
};
typedef struct _ccs_hyperparameter_categorical_data_mock_s _ccs_hyperparameter_categorical_data_mock_t;

static inline size_t
_ccs_deserialize_bin_ccs_hyperparameter_categorical_data(
		_ccs_hyperparameter_categorical_data_mock_t  *data,
		size_t                                       *buffer_size,
		const char                                  **buffer) {
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_hyperparameter_common_data(
		&data->common_data, buffer_size, buffer));
	uint64_t num_possible_values;
	CCS_VALIDATE(_ccs_deserialize_bin_uint64(
		&num_possible_values, buffer_size, buffer));
	data->num_possible_values = num_possible_values;
	data->possible_values =
		(ccs_datum_t *)malloc(num_possible_values*sizeof(ccs_datum_t));
	if (!data->possible_values)
		return -CCS_OUT_OF_MEMORY;
	for (size_t i = 0; i < num_possible_values; i++)
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_datum(
			data->possible_values + i, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_hyperparameter_categorical(
		ccs_hyperparameter_t    *hyperparameter_ret,
		uint32_t                 version,
		size_t                  *buffer_size,
		const char             **buffer) {
	(void)version;
	ccs_result_t res = CCS_SUCCESS;
	_ccs_hyperparameter_categorical_data_mock_t data;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_hyperparameter_categorical_data(
		&data, buffer_size, buffer));
	size_t default_value_index;
	int found = 0;
	for (size_t i = 0; i < data.num_possible_values; i++)
		if (!ccs_datum_cmp(data.common_data.default_value, data.possible_values[i])) {
			found = 1;
			default_value_index = i;
		}
	if (!found) {
		res = -CCS_INVALID_VALUE;
		goto end;
	}

	switch (data.common_data.type) {
	case CCS_HYPERPARAMETER_TYPE_CATEGORICAL:
		CCS_VALIDATE_ERR_GOTO(res, ccs_create_categorical_hyperparameter(
			data.common_data.name,
			data.num_possible_values,
			data.possible_values,
			default_value_index,
			NULL,
			hyperparameter_ret), end);
		break;
	case CCS_HYPERPARAMETER_TYPE_ORDINAL:
		CCS_VALIDATE_ERR_GOTO(res, ccs_create_ordinal_hyperparameter(
			data.common_data.name,
			data.num_possible_values,
			data.possible_values,
			default_value_index,
			NULL,
			hyperparameter_ret), end);
		break;
	case CCS_HYPERPARAMETER_TYPE_DISCRETE:
		CCS_VALIDATE_ERR_GOTO(res, ccs_create_discrete_hyperparameter(
			data.common_data.name,
			data.num_possible_values,
			data.possible_values,
			default_value_index,
			NULL,
			hyperparameter_ret), end);
		break;
	default:
		res = -CCS_INVALID_TYPE;
		goto end;
	}
end:
	free(data.possible_values);
	return res;
}

typedef _ccs_hyperparameter_common_data_t _ccs_hyperparameter_string_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_ccs_hyperparameter_string_data(
		_ccs_hyperparameter_string_data_mock_t  *data,
		size_t                                  *buffer_size,
		const char                             **buffer) {
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_hyperparameter_common_data(
		data, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_hyperparameter_string(
		ccs_hyperparameter_t    *hyperparameter_ret,
		uint32_t                 version,
		size_t                  *buffer_size,
		const char             **buffer) {
	(void)version;
	_ccs_hyperparameter_string_data_mock_t data;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_hyperparameter_string_data(
		&data, buffer_size, buffer));
	CCS_VALIDATE(ccs_create_string_hyperparameter(
		data.name,
		NULL,
		hyperparameter_ret));
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_hyperparameter(
		ccs_hyperparameter_t    *hyperparameter_ret,
		uint32_t                 version,
		size_t                  *buffer_size,
		const char             **buffer) {
	_ccs_object_internal_t obj;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object_internal(
		&obj, buffer_size, buffer));
	if (CCS_UNLIKELY(obj.type != CCS_HYPERPARAMETER))
		return -CCS_INVALID_TYPE;

	ccs_hyperparameter_type_t htype;
	CCS_VALIDATE(_ccs_peek_bin_ccs_hyperparameter_type(
		&htype, buffer_size, buffer));
	switch (htype) {
	case CCS_HYPERPARAMETER_TYPE_NUMERICAL:
		CCS_VALIDATE(_ccs_deserialize_bin_hyperparameter_numerical(
			hyperparameter_ret, version, buffer_size, buffer));
		break;
	case CCS_HYPERPARAMETER_TYPE_CATEGORICAL:
	case CCS_HYPERPARAMETER_TYPE_ORDINAL:
	case CCS_HYPERPARAMETER_TYPE_DISCRETE:
		CCS_VALIDATE(_ccs_deserialize_bin_hyperparameter_categorical(
			hyperparameter_ret, version, buffer_size, buffer));
		break;
	case CCS_HYPERPARAMETER_TYPE_STRING:
		CCS_VALIDATE(_ccs_deserialize_bin_hyperparameter_string(
			hyperparameter_ret, version, buffer_size, buffer));
		break;
	default:
		return -CCS_UNSUPPORTED_OPERATION;
	}
	ccs_object_set_user_data(*hyperparameter_ret, obj.user_data);

	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_hyperparameter_deserialize(
		ccs_hyperparameter_t    *hyperparameter_ret,
		ccs_serialize_format_t   format,
		uint32_t                 version,
		size_t                  *buffer_size,
		const char             **buffer) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_hyperparameter(
			hyperparameter_ret, version, buffer_size, buffer));
		break;
	default:
		return -CCS_INVALID_VALUE;
	}
	return CCS_SUCCESS;
}

#endif //_HYPERPARAMETER_DESERIALIZE_H
