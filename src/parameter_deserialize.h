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
	const char        *data_type_str;
	const char        *name_str;
	ccs_numeric_type_t data_type;
	ccs_datum_t        default_datum;
	CCS_VALIDATE(
		_ccs_json_extract_string(json, "data_type", &data_type_str));
	CCS_VALIDATE(_ccs_json_extract_string(json, "name", &name_str));

	CCS_VALIDATE(
		_ccs_json_numeric_type_from_string(data_type_str, &data_type));
	CCS_VALIDATE(
		_ccs_json_extract_datum(json, "default_value", &default_datum));

	ccs_numeric_t lower, upper, quantization, default_value;
	if (data_type == CCS_NUMERIC_TYPE_FLOAT) {
		double fl, fu, fq;
		CCS_VALIDATE(_ccs_json_extract_float(json, "lower", &fl));
		CCS_VALIDATE(_ccs_json_extract_float(json, "upper", &fu));
		CCS_VALIDATE(
			_ccs_json_extract_float(json, "quantization", &fq));
		lower.f         = fl;
		upper.f         = fu;
		quantization.f  = fq;
		default_value.f = default_datum.value.f;
	} else {
		CCS_VALIDATE(_ccs_json_extract_int(json, "lower", &lower.i));
		CCS_VALIDATE(_ccs_json_extract_int(json, "upper", &upper.i));
		CCS_VALIDATE(_ccs_json_extract_int(
			json, "quantization", &quantization.i));
		default_value.i = default_datum.value.i;
	}
	CCS_VALIDATE(ccs_create_numerical_parameter(
		name_str, data_type, lower, upper, quantization, default_value,
		parameter_ret));
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
	cJSON       *j_possible_values;

	const char  *name_str;
	CCS_VALIDATE(_ccs_json_extract_string(json, "name", &name_str));
	CCS_VALIDATE(_ccs_json_extract_array(
		json, "possible_values", &j_possible_values,
		&num_possible_values));
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
		res,
		_ccs_json_extract_datum(json, "default_value", &default_datum),
		end);
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
				name_str, num_possible_values, possible_values,
				default_value_index, parameter_ret),
			end);
		break;
	case CCS_PARAMETER_TYPE_ORDINAL:
		CCS_VALIDATE_ERR_GOTO(
			res,
			ccs_create_ordinal_parameter(
				name_str, num_possible_values, possible_values,
				default_value_index, parameter_ret),
			end);
		break;
	case CCS_PARAMETER_TYPE_DISCRETE:
		CCS_VALIDATE_ERR_GOTO(
			res,
			ccs_create_discrete_parameter(
				name_str, num_possible_values, possible_values,
				default_value_index, parameter_ret),
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
	const char *name_str;
	CCS_VALIDATE(_ccs_json_extract_string(json, "name", &name_str));
	CCS_VALIDATE(ccs_create_string_parameter(name_str, parameter_ret));
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
	cJSON               *json;
	const char          *ptype_str;
	ccs_parameter_type_t ptype;

	(void)opts;
	json = *(cJSON **)buffer;
	CCS_VALIDATE(
		_ccs_json_extract_string(json, "parameter_type", &ptype_str));
	CCS_VALIDATE(_ccs_json_parameter_type_from_string(ptype_str, &ptype));

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
