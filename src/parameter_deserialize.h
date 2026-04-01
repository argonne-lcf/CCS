#ifndef _PARAMETER_DESERIALIZE_H
#define _PARAMETER_DESERIALIZE_H
#include "cconfigspace_internal.h"
#include "parameter_internal.h"

static inline ccs_result_t
_ccs_deserialize_bin_parameter_numerical(
	ccs_parameter_t *parameter_ret,
	uint32_t         version,
	size_t          *buffer_size,
	const char     **buffer)
{
	(void)version;
	_ccs_parameter_numerical_data_t data;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_parameter_numerical_data(
		&data, buffer_size, buffer));
	CCS_VALIDATE(ccs_create_numerical_parameter(
		data.common_data.name, data.common_data.interval.type,
		(data.common_data.interval.type == CCS_NUMERIC_TYPE_FLOAT ?
			 CCSF(data.common_data.interval.lower.f) :
			 CCSI(data.common_data.interval.lower.i)),
		(data.common_data.interval.type == CCS_NUMERIC_TYPE_FLOAT ?
			 CCSF(data.common_data.interval.upper.f) :
			 CCSI(data.common_data.interval.upper.i)),
		(data.common_data.interval.type == CCS_NUMERIC_TYPE_FLOAT ?
			 CCSF(data.quantization.f) :
			 CCSI(data.quantization.i)),
		(data.common_data.interval.type == CCS_NUMERIC_TYPE_FLOAT ?
			 CCSF(data.common_data.default_value.value.f) :
			 CCSI(data.common_data.default_value.value.i)),
		parameter_ret));
	return CCS_RESULT_SUCCESS;
}

struct _ccs_parameter_categorical_data_mock_s {
	_ccs_parameter_common_data_t common_data;
	size_t                       num_possible_values;
	ccs_datum_t                 *possible_values;
};
typedef struct _ccs_parameter_categorical_data_mock_s
	_ccs_parameter_categorical_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_ccs_parameter_categorical_data(
	_ccs_parameter_categorical_data_mock_t *data,
	size_t                                 *buffer_size,
	const char                            **buffer)
{
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_parameter_common_data(
		&data->common_data, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->num_possible_values, buffer_size, buffer));
	{
		size_t _sz;
		CCS_REFUTE(
			CCS_ALLOC_SIZE(
				&_sz, CCS_ALLOC_SIZE_ARRAY(
					      data->num_possible_values,
					      ccs_datum_t)),
			CCS_RESULT_ERROR_OUT_OF_MEMORY);
		data->possible_values = (ccs_datum_t *)calloc(1, _sz);
		CCS_REFUTE(
			!data->possible_values, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	}
	for (size_t i = 0; i < data->num_possible_values; i++)
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_datum(
			data->possible_values + i, buffer_size, buffer));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_parameter_categorical(
	ccs_parameter_t *parameter_ret,
	uint32_t         version,
	size_t          *buffer_size,
	const char     **buffer)
{
	(void)version;
	ccs_result_t                           res   = CCS_RESULT_SUCCESS;
	int                                    found = 0;
	_ccs_parameter_categorical_data_mock_t data;
	data.possible_values = NULL;
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_ccs_parameter_categorical_data(
			&data, buffer_size, buffer),
		end);
	size_t default_value_index;
	for (size_t i = 0; i < data.num_possible_values; i++)
		if (!ccs_datum_cmp(
			    data.common_data.default_value,
			    data.possible_values[i])) {
			found               = 1;
			default_value_index = i;
		}
	CCS_REFUTE_ERR_GOTO(res, !found, CCS_RESULT_ERROR_INVALID_VALUE, end);

	switch (data.common_data.type) {
	case CCS_PARAMETER_TYPE_CATEGORICAL:
		CCS_VALIDATE_ERR_GOTO(
			res,
			ccs_create_categorical_parameter(
				data.common_data.name, data.num_possible_values,
				data.possible_values, default_value_index,
				parameter_ret),
			end);
		break;
	case CCS_PARAMETER_TYPE_ORDINAL:
		CCS_VALIDATE_ERR_GOTO(
			res,
			ccs_create_ordinal_parameter(
				data.common_data.name, data.num_possible_values,
				data.possible_values, default_value_index,
				parameter_ret),
			end);
		break;
	case CCS_PARAMETER_TYPE_DISCRETE:
		CCS_VALIDATE_ERR_GOTO(
			res,
			ccs_create_discrete_parameter(
				data.common_data.name, data.num_possible_values,
				data.possible_values, default_value_index,
				parameter_ret),
			end);
		break;
	default:
		CCS_RAISE_ERR_GOTO(
			res, CCS_RESULT_ERROR_INVALID_TYPE, end,
			"Unsupport parameter type: %d", data.common_data.type);
	}
end:
	if (data.possible_values)
		free(data.possible_values);
	return res;
}

typedef _ccs_parameter_common_data_t _ccs_parameter_string_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_ccs_parameter_string_data(
	_ccs_parameter_string_data_mock_t *data,
	size_t                            *buffer_size,
	const char                       **buffer)
{
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_parameter_common_data(
		data, buffer_size, buffer));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_parameter_string(
	ccs_parameter_t *parameter_ret,
	uint32_t         version,
	size_t          *buffer_size,
	const char     **buffer)
{
	(void)version;
	_ccs_parameter_string_data_mock_t data;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_parameter_string_data(
		&data, buffer_size, buffer));
	CCS_VALIDATE(ccs_create_string_parameter(data.name, parameter_ret));
	return CCS_RESULT_SUCCESS;
}

/*============================================================================
 * JSON deserialization
 *============================================================================*/

static inline ccs_result_t
_ccs_deserialize_json_parameter_numerical(
	ccs_parameter_t *parameter_ret,
	cJSON           *json)
{
	cJSON *j_data_type =
		cJSON_GetObjectItemCaseSensitive(json, "data_type");
	cJSON *j_name  = cJSON_GetObjectItemCaseSensitive(json, "name");
	cJSON *j_lower = cJSON_GetObjectItemCaseSensitive(json, "lower");
	cJSON *j_upper = cJSON_GetObjectItemCaseSensitive(json, "upper");
	cJSON *j_quantization =
		cJSON_GetObjectItemCaseSensitive(json, "quantization");
	cJSON *j_default_value =
		cJSON_GetObjectItemCaseSensitive(json, "default_value");

	CCS_REFUTE(
		!j_data_type || !cJSON_IsString(j_data_type),
		CCS_RESULT_ERROR_INVALID_VALUE);
	CCS_REFUTE(
		!j_name || !cJSON_IsString(j_name),
		CCS_RESULT_ERROR_INVALID_VALUE);
	CCS_REFUTE(
		!j_lower || !cJSON_IsNumber(j_lower),
		CCS_RESULT_ERROR_INVALID_VALUE);
	CCS_REFUTE(
		!j_upper || !cJSON_IsNumber(j_upper),
		CCS_RESULT_ERROR_INVALID_VALUE);
	CCS_REFUTE(
		!j_quantization || !cJSON_IsNumber(j_quantization),
		CCS_RESULT_ERROR_INVALID_VALUE);
	CCS_REFUTE(!j_default_value, CCS_RESULT_ERROR_INVALID_VALUE);

	ccs_numeric_type_t data_type;
	ccs_datum_t        default_datum;
	CCS_VALIDATE(_ccs_json_numeric_type_from_string(
		j_data_type->valuestring, &data_type));
	CCS_VALIDATE(_ccs_json_get_datum(j_default_value, &default_datum));

	ccs_numeric_t lower, upper, quantization, default_value;
	if (data_type == CCS_NUMERIC_TYPE_FLOAT) {
		lower.f         = j_lower->valuedouble;
		upper.f         = j_upper->valuedouble;
		quantization.f  = j_quantization->valuedouble;
		default_value.f = default_datum.value.f;
	} else {
		lower.i         = (ccs_int_t)j_lower->valuedouble;
		upper.i         = (ccs_int_t)j_upper->valuedouble;
		quantization.i  = (ccs_int_t)j_quantization->valuedouble;
		default_value.i = default_datum.value.i;
	}
	CCS_VALIDATE(ccs_create_numerical_parameter(
		j_name->valuestring, data_type, lower, upper, quantization,
		default_value, parameter_ret));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_json_parameter_categorical(
	ccs_parameter_t     *parameter_ret,
	ccs_parameter_type_t ptype,
	cJSON               *json)
{
	ccs_result_t res                 = CCS_RESULT_SUCCESS;
	ccs_datum_t *possible_values     = NULL;
	size_t       num_possible_values = 0;
	ccs_datum_t  default_datum;
	int          found               = 0;
	size_t       default_value_index = 0;
	cJSON       *j_name = cJSON_GetObjectItemCaseSensitive(json, "name");
	cJSON       *j_default_value =
		cJSON_GetObjectItemCaseSensitive(json, "default_value");
	cJSON *j_possible_values =
		cJSON_GetObjectItemCaseSensitive(json, "possible_values");

	CCS_REFUTE(
		!j_name || !cJSON_IsString(j_name),
		CCS_RESULT_ERROR_INVALID_VALUE);
	CCS_REFUTE(!j_default_value, CCS_RESULT_ERROR_INVALID_VALUE);
	CCS_REFUTE(
		!j_possible_values || !cJSON_IsArray(j_possible_values),
		CCS_RESULT_ERROR_INVALID_VALUE);

	num_possible_values = (size_t)cJSON_GetArraySize(j_possible_values);
	possible_values =
		(ccs_datum_t *)calloc(num_possible_values, sizeof(ccs_datum_t));
	CCS_REFUTE(!possible_values, CCS_RESULT_ERROR_OUT_OF_MEMORY);

	for (size_t i = 0; i < num_possible_values; i++) {
		cJSON *item = cJSON_GetArrayItem(j_possible_values, (int)i);
		CCS_VALIDATE_ERR_GOTO(
			res, _ccs_json_get_datum(item, possible_values + i),
			end);
	}

	CCS_VALIDATE_ERR_GOTO(
		res, _ccs_json_get_datum(j_default_value, &default_datum), end);
	for (size_t i = 0; i < num_possible_values; i++)
		if (!ccs_datum_cmp(default_datum, possible_values[i])) {
			found               = 1;
			default_value_index = i;
		}
	CCS_REFUTE_ERR_GOTO(res, !found, CCS_RESULT_ERROR_INVALID_VALUE, end);

	switch (ptype) {
	case CCS_PARAMETER_TYPE_CATEGORICAL:
		CCS_VALIDATE_ERR_GOTO(
			res,
			ccs_create_categorical_parameter(
				j_name->valuestring, num_possible_values,
				possible_values, default_value_index,
				parameter_ret),
			end);
		break;
	case CCS_PARAMETER_TYPE_ORDINAL:
		CCS_VALIDATE_ERR_GOTO(
			res,
			ccs_create_ordinal_parameter(
				j_name->valuestring, num_possible_values,
				possible_values, default_value_index,
				parameter_ret),
			end);
		break;
	case CCS_PARAMETER_TYPE_DISCRETE:
		CCS_VALIDATE_ERR_GOTO(
			res,
			ccs_create_discrete_parameter(
				j_name->valuestring, num_possible_values,
				possible_values, default_value_index,
				parameter_ret),
			end);
		break;
	default:
		CCS_RAISE_ERR_GOTO(
			res, CCS_RESULT_ERROR_INVALID_TYPE, end,
			"Unsupport parameter type: %d", ptype);
	}
end:
	if (possible_values)
		free(possible_values);
	return res;
}

static inline ccs_result_t
_ccs_deserialize_json_parameter_string(
	ccs_parameter_t *parameter_ret,
	cJSON           *json)
{
	cJSON *j_name = cJSON_GetObjectItemCaseSensitive(json, "name");
	CCS_REFUTE(
		!j_name || !cJSON_IsString(j_name),
		CCS_RESULT_ERROR_INVALID_VALUE);
	CCS_VALIDATE(ccs_create_string_parameter(
		j_name->valuestring, parameter_ret));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_json_parameter(
	ccs_parameter_t                   *parameter_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	(void)version;
	(void)buffer_size;
	(void)opts;
	cJSON *json = *(cJSON **)buffer;

	cJSON *j_ptype =
		cJSON_GetObjectItemCaseSensitive(json, "parameter_type");
	CCS_REFUTE(
		!j_ptype || !cJSON_IsString(j_ptype),
		CCS_RESULT_ERROR_INVALID_VALUE);

	ccs_parameter_type_t ptype;
	CCS_VALIDATE(_ccs_json_parameter_type_from_string(
		j_ptype->valuestring, &ptype));

	switch (ptype) {
	case CCS_PARAMETER_TYPE_NUMERICAL:
		CCS_VALIDATE(_ccs_deserialize_json_parameter_numerical(
			parameter_ret, json));
		break;
	case CCS_PARAMETER_TYPE_CATEGORICAL:
	case CCS_PARAMETER_TYPE_ORDINAL:
	case CCS_PARAMETER_TYPE_DISCRETE:
		CCS_VALIDATE(_ccs_deserialize_json_parameter_categorical(
			parameter_ret, ptype, json));
		break;
	case CCS_PARAMETER_TYPE_STRING:
		CCS_VALIDATE(_ccs_deserialize_json_parameter_string(
			parameter_ret, json));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_TYPE,
			"Unsupport parameter type: %d", ptype);
	}
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_parameter(
	ccs_parameter_t                   *parameter_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	ccs_parameter_type_t htype;

	(void)opts;
	CCS_VALIDATE(
		_ccs_peek_bin_ccs_parameter_type(&htype, buffer_size, buffer));
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
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_TYPE,
			"Unsupport parameter type: %d", htype);
	}
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_parameter_deserialize(
	ccs_parameter_t                   *parameter_ret,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_parameter(
			parameter_ret, version, buffer_size, buffer, opts));
		break;
	case CCS_SERIALIZE_FORMAT_JSON:
		CCS_VALIDATE(_ccs_deserialize_json_parameter(
			parameter_ret, version, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

#endif //_PARAMETER_DESERIALIZE_H
