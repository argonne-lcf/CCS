#ifndef _CONTEXT_DESERIALIZE_H
#define _CONTEXT_DESERIALIZE_H

struct _ccs_context_data_mock_s {
	const char      *name;
	size_t           num_parameters;
	ccs_parameter_t *parameters;
};
typedef struct _ccs_context_data_mock_s _ccs_context_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_ccs_context_data(
	_ccs_context_data_mock_t          *data,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	data->num_parameters = 0;
	data->parameters     = NULL;
	CCS_VALIDATE(
		_ccs_deserialize_bin_string(&data->name, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->num_parameters, buffer_size, buffer));
	if (data->num_parameters) {
		data->parameters = (ccs_parameter_t *)calloc(
			data->num_parameters, sizeof(ccs_parameter_t));
		CCS_REFUTE(!data->parameters, CCS_RESULT_ERROR_OUT_OF_MEMORY);
		for (size_t i = 0; i < data->num_parameters; i++)
			CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
				(ccs_object_t *)data->parameters + i,
				CCS_OBJECT_TYPE_PARAMETER,
				CCS_SERIALIZE_FORMAT_BINARY, version,
				buffer_size, buffer, opts));
	}
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_json_ccs_context_data(
	_ccs_context_data_mock_t          *data,
	uint32_t                           version,
	cJSON                             *json,
	_ccs_object_deserialize_options_t *opts)
{
	size_t num;
	cJSON *j_params = cJSON_GetObjectItemCaseSensitive(json, "parameters");
	const char *name_str;
	data->num_parameters = 0;
	data->parameters     = NULL;
	CCS_VALIDATE(_ccs_json_extract_string(json, "name", &name_str));
	CCS_REFUTE(
		!j_params || !cJSON_IsArray(j_params),
		CCS_RESULT_ERROR_INVALID_VALUE);
	data->name           = name_str;
	num                  = (size_t)cJSON_GetArraySize(j_params);
	data->num_parameters = num;
	if (num) {
		data->parameters =
			(ccs_parameter_t *)calloc(num, sizeof(ccs_parameter_t));
		CCS_REFUTE(!data->parameters, CCS_RESULT_ERROR_OUT_OF_MEMORY);
		for (size_t i = 0; i < num; i++) {
			cJSON *child     = cJSON_GetArrayItem(j_params, (int)i);
			const char *cbuf = (const char *)child;
			size_t      dummy = 0;
			CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
				(ccs_object_t *)data->parameters + i,
				CCS_OBJECT_TYPE_PARAMETER,
				CCS_SERIALIZE_FORMAT_JSON, version, &dummy,
				&cbuf, opts));
		}
	}
	return CCS_RESULT_SUCCESS;
}

#endif //_CONTEXT_DESERIALIZE_H
