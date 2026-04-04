#ifndef _DISTRIBUTION_DESERIALIZE_H
#define _DISTRIBUTION_DESERIALIZE_H
#include "distribution_internal.h"
#include "cconfigspace_json.h"

struct _ccs_distribution_uniform_data_mock_s {
	_ccs_distribution_common_data_t common_data;
	ccs_numeric_type_t              data_type;
	ccs_scale_type_t                scale_type;
	ccs_numeric_t                   lower;
	ccs_numeric_t                   upper;
	ccs_numeric_t                   quantization;
};
typedef struct _ccs_distribution_uniform_data_mock_s
	_ccs_distribution_uniform_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_ccs_distribution_uniform_data(
	_ccs_distribution_uniform_data_mock_t *data,
	size_t                                *buffer_size,
	const char                           **buffer)
{
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_distribution_common_data(
		&data->common_data, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_numeric_type(
		&data->data_type, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_scale_type(
		&data->scale_type, buffer_size, buffer));
	if (data->data_type == CCS_NUMERIC_TYPE_FLOAT) {
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_float(
			&data->lower.f, buffer_size, buffer));
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_float(
			&data->upper.f, buffer_size, buffer));
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_float(
			&data->quantization.f, buffer_size, buffer));
	} else {
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_int(
			&data->lower.i, buffer_size, buffer));
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_int(
			&data->upper.i, buffer_size, buffer));
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_int(
			&data->quantization.i, buffer_size, buffer));
	}
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_distribution_uniform(
	ccs_distribution_t *distribution_ret,
	uint32_t            version,
	size_t             *buffer_size,
	const char        **buffer)
{
	(void)version;
	_ccs_distribution_uniform_data_mock_t data;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_distribution_uniform_data(
		&data, buffer_size, buffer));
	CCS_VALIDATE(ccs_create_uniform_distribution(
		data.data_type, data.lower, data.upper, data.scale_type,
		data.quantization, distribution_ret));
	return CCS_RESULT_SUCCESS;
}

struct _ccs_distribution_normal_data_mock_s {
	_ccs_distribution_common_data_t common_data;
	ccs_numeric_type_t              data_type;
	ccs_scale_type_t                scale_type;
	ccs_float_t                     mu;
	ccs_float_t                     sigma;
	ccs_numeric_t                   quantization;
};
typedef struct _ccs_distribution_normal_data_mock_s
	_ccs_distribution_normal_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_ccs_distribution_normal_data(
	_ccs_distribution_normal_data_mock_t *data,
	size_t                               *buffer_size,
	const char                          **buffer)
{
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_distribution_common_data(
		&data->common_data, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_numeric_type(
		&data->data_type, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_scale_type(
		&data->scale_type, buffer_size, buffer));
	CCS_VALIDATE(
		_ccs_deserialize_bin_ccs_float(&data->mu, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_float(
		&data->sigma, buffer_size, buffer));
	if (data->data_type == CCS_NUMERIC_TYPE_FLOAT)
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_float(
			&data->quantization.f, buffer_size, buffer));
	else
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_int(
			&data->quantization.i, buffer_size, buffer));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_distribution_normal(
	ccs_distribution_t *distribution_ret,
	uint32_t            version,
	size_t             *buffer_size,
	const char        **buffer)
{
	(void)version;
	_ccs_distribution_normal_data_mock_t data;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_distribution_normal_data(
		&data, buffer_size, buffer));
	CCS_VALIDATE(ccs_create_normal_distribution(
		data.data_type, data.mu, data.sigma, data.scale_type,
		data.quantization, distribution_ret));
	return CCS_RESULT_SUCCESS;
}

struct _ccs_distribution_roulette_data_mock_s {
	_ccs_distribution_common_data_t common_data;
	size_t                          num_areas;
	ccs_float_t                    *areas;
};
typedef struct _ccs_distribution_roulette_data_mock_s
	_ccs_distribution_roulette_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_ccs_distribution_roulette_data(
	_ccs_distribution_roulette_data_mock_t *data,
	size_t                                 *buffer_size,
	const char                            **buffer)
{
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_distribution_common_data(
		&data->common_data, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->num_areas, buffer_size, buffer));
	{
		size_t _sz;
		CCS_REFUTE(
			CCS_ALLOC_SIZE(
				&_sz, CCS_ALLOC_SIZE_ARRAY(
					      data->num_areas, ccs_float_t)),
			CCS_RESULT_ERROR_OUT_OF_MEMORY);
		data->areas = (ccs_float_t *)calloc(1, _sz);
		CCS_REFUTE(!data->areas, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	}
	for (size_t i = 0; i < data->num_areas; i++)
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_float(
			data->areas + i, buffer_size, buffer));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_distribution_roulette(
	ccs_distribution_t *distribution_ret,
	uint32_t            version,
	size_t             *buffer_size,
	const char        **buffer)
{
	(void)version;
	ccs_result_t                           res = CCS_RESULT_SUCCESS;
	_ccs_distribution_roulette_data_mock_t data;
	data.areas = NULL;
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_ccs_distribution_roulette_data(
			&data, buffer_size, buffer),
		end);
	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_roulette_distribution(
			data.num_areas, data.areas, distribution_ret),
		end);
end:
	if (data.areas)
		free(data.areas);
	return res;
}

struct _ccs_distribution_mixture_data_mock_s {
	_ccs_distribution_common_data_t common_data;
	size_t                          num_distributions;
	ccs_distribution_t             *distributions;
	ccs_float_t                    *weights;
};
typedef struct _ccs_distribution_mixture_data_mock_s
	_ccs_distribution_mixture_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_ccs_distribution_mixture_data(
	_ccs_distribution_mixture_data_mock_t *data,
	uint32_t                               version,
	size_t                                *buffer_size,
	const char                           **buffer,
	_ccs_object_deserialize_options_t     *opts)
{
	_ccs_object_deserialize_options_t new_opts = *opts;
	new_opts.handle_map                        = NULL;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_distribution_common_data(
		&data->common_data, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->num_distributions, buffer_size, buffer));
	data->distributions = (ccs_distribution_t *)calloc(
		data->num_distributions, sizeof(ccs_distribution_t));
	CCS_REFUTE(!data->distributions, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	{
		size_t _sz;
		CCS_REFUTE(
			CCS_ALLOC_SIZE(
				&_sz,
				CCS_ALLOC_SIZE_ARRAY(
					data->num_distributions, ccs_float_t)),
			CCS_RESULT_ERROR_OUT_OF_MEMORY);
		data->weights = (ccs_float_t *)calloc(1, _sz);
		CCS_REFUTE(!data->weights, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	}
	for (size_t i = 0; i < data->num_distributions; i++) {
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_float(
			data->weights + i, buffer_size, buffer));
		CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
			(ccs_object_t *)data->distributions + i,
			CCS_OBJECT_TYPE_DISTRIBUTION,
			CCS_SERIALIZE_FORMAT_BINARY, version, buffer_size,
			buffer, &new_opts));
	}
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_distribution_mixture(
	ccs_distribution_t                *distribution_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	ccs_result_t                          res = CCS_RESULT_SUCCESS;
	_ccs_distribution_mixture_data_mock_t data;
	data.distributions = NULL;
	data.weights       = NULL;
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_ccs_distribution_mixture_data(
			&data, version, buffer_size, buffer, opts),
		end);
	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_mixture_distribution(
			data.num_distributions, data.distributions,
			data.weights, distribution_ret),
		end);
end:
	if (data.distributions) {
		for (size_t i = 0; i < data.num_distributions; i++)
			if (data.distributions[i])
				ccs_release_object(data.distributions[i]);
		free(data.distributions);
	}
	if (data.weights)
		free(data.weights);
	return res;
}

struct _ccs_distribution_multivariate_data_mock_s {
	_ccs_distribution_common_data_t common_data;
	size_t                          num_distributions;
	ccs_distribution_t             *distributions;
};
typedef struct _ccs_distribution_multivariate_data_mock_s
	_ccs_distribution_multivariate_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_ccs_distribution_multivariate_data(
	_ccs_distribution_multivariate_data_mock_t *data,
	uint32_t                                    version,
	size_t                                     *buffer_size,
	const char                                **buffer,
	_ccs_object_deserialize_options_t          *opts)
{
	_ccs_object_deserialize_options_t new_opts = *opts;
	new_opts.handle_map                        = NULL;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_distribution_common_data(
		&data->common_data, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->num_distributions, buffer_size, buffer));
	data->distributions = (ccs_distribution_t *)calloc(
		data->num_distributions, sizeof(ccs_distribution_t));
	CCS_REFUTE(!data->distributions, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	for (size_t i = 0; i < data->num_distributions; i++)
		CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
			(ccs_object_t *)data->distributions + i,
			CCS_OBJECT_TYPE_DISTRIBUTION,
			CCS_SERIALIZE_FORMAT_BINARY, version, buffer_size,
			buffer, &new_opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_distribution_multivariate(
	ccs_distribution_t                *distribution_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	ccs_result_t                               res = CCS_RESULT_SUCCESS;
	_ccs_distribution_multivariate_data_mock_t data;
	data.distributions = NULL;
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_ccs_distribution_multivariate_data(
			&data, version, buffer_size, buffer, opts),
		end);
	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_multivariate_distribution(
			data.num_distributions, data.distributions,
			distribution_ret),
		end);
end:
	if (data.distributions) {
		for (size_t i = 0; i < data.num_distributions; i++)
			if (data.distributions[i])
				ccs_release_object(data.distributions[i]);
		free(data.distributions);
	}
	return res;
}

/*============================================================================
 * JSON deserialization
 *============================================================================*/

static inline ccs_result_t
_ccs_deserialize_json_distribution_uniform(
	ccs_distribution_t *distribution_ret,
	cJSON              *json)
{
	const char        *data_type_str;
	const char        *scale_type_str;
	ccs_numeric_type_t data_type;
	ccs_scale_type_t   scale_type;
	ccs_numeric_t      lower, upper, quantization;
	CCS_VALIDATE(
		_ccs_json_extract_string(json, "data_type", &data_type_str));
	CCS_VALIDATE(
		_ccs_json_extract_string(json, "scale_type", &scale_type_str));
	CCS_VALIDATE(
		_ccs_json_numeric_type_from_string(data_type_str, &data_type));
	CCS_VALIDATE(
		_ccs_json_scale_type_from_string(scale_type_str, &scale_type));

	if (data_type == CCS_NUMERIC_TYPE_FLOAT) {
		double fl, fu, fq;
		CCS_VALIDATE(_ccs_json_extract_float(json, "lower", &fl));
		CCS_VALIDATE(_ccs_json_extract_float(json, "upper", &fu));
		CCS_VALIDATE(
			_ccs_json_extract_float(json, "quantization", &fq));
		lower.f        = fl;
		upper.f        = fu;
		quantization.f = fq;
	} else {
		CCS_VALIDATE(_ccs_json_extract_int(json, "lower", &lower.i));
		CCS_VALIDATE(_ccs_json_extract_int(json, "upper", &upper.i));
		CCS_VALIDATE(_ccs_json_extract_int(
			json, "quantization", &quantization.i));
	}
	CCS_VALIDATE(ccs_create_uniform_distribution(
		data_type, lower, upper, scale_type, quantization,
		distribution_ret));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_json_distribution_normal(
	ccs_distribution_t *distribution_ret,
	cJSON              *json)
{
	const char        *data_type_str;
	const char        *scale_type_str;
	ccs_numeric_type_t data_type;
	ccs_scale_type_t   scale_type;
	double             mu_val, sigma_val;
	ccs_numeric_t      quantization;
	CCS_VALIDATE(
		_ccs_json_extract_string(json, "data_type", &data_type_str));
	CCS_VALIDATE(
		_ccs_json_extract_string(json, "scale_type", &scale_type_str));
	CCS_VALIDATE(
		_ccs_json_numeric_type_from_string(data_type_str, &data_type));
	CCS_VALIDATE(
		_ccs_json_scale_type_from_string(scale_type_str, &scale_type));
	CCS_VALIDATE(_ccs_json_extract_float(json, "mu", &mu_val));
	CCS_VALIDATE(_ccs_json_extract_float(json, "sigma", &sigma_val));

	if (data_type == CCS_NUMERIC_TYPE_FLOAT) {
		double fq;
		CCS_VALIDATE(
			_ccs_json_extract_float(json, "quantization", &fq));
		quantization.f = fq;
	} else {
		CCS_VALIDATE(_ccs_json_extract_int(
			json, "quantization", &quantization.i));
	}
	CCS_VALIDATE(ccs_create_normal_distribution(
		data_type, mu_val, sigma_val, scale_type, quantization,
		distribution_ret));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_json_distribution_roulette(
	ccs_distribution_t *distribution_ret,
	cJSON              *json)
{
	ccs_result_t res   = CCS_RESULT_SUCCESS;
	ccs_float_t *areas = NULL;
	cJSON       *j_areas;
	size_t       num_areas;
	CCS_VALIDATE(
		_ccs_json_extract_array(json, "areas", &j_areas, &num_areas));
	areas = (ccs_float_t *)calloc(num_areas, sizeof(ccs_float_t));
	CCS_REFUTE(!areas, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	for (size_t i = 0; i < num_areas; i++) {
		cJSON *item = cJSON_GetArrayItem(j_areas, (int)i);
		CCS_REFUTE_ERR_GOTO(
			res, !item, CCS_RESULT_ERROR_INVALID_VALUE, end);
		CCS_VALIDATE_ERR_GOTO(
			res, _ccs_json_get_float(item, &areas[i]), end);
	}
	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_roulette_distribution(
			num_areas, areas, distribution_ret),
		end);
end:
	free(areas);
	return res;
}

static inline ccs_result_t
_ccs_deserialize_json_distribution_mixture(
	ccs_distribution_t                *distribution_ret,
	uint32_t                           version,
	cJSON                             *json,
	_ccs_object_deserialize_options_t *opts)
{
	ccs_result_t                      res           = CCS_RESULT_SUCCESS;
	ccs_distribution_t               *distributions = NULL;
	ccs_float_t                      *weights       = NULL;

	_ccs_object_deserialize_options_t new_opts      = *opts;
	new_opts.handle_map                             = NULL;

	cJSON *j_weights;
	cJSON *j_distributions;
	size_t num;
	size_t num_weights;
	CCS_VALIDATE(_ccs_json_extract_array(
		json, "distributions", &j_distributions, &num));
	CCS_VALIDATE(_ccs_json_extract_array(
		json, "weights", &j_weights, &num_weights));
	CCS_REFUTE(num_weights != num, CCS_RESULT_ERROR_INVALID_VALUE);

	distributions =
		(ccs_distribution_t *)calloc(num, sizeof(ccs_distribution_t));
	CCS_REFUTE(!distributions, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	weights = (ccs_float_t *)calloc(num, sizeof(ccs_float_t));
	if (!weights) {
		free(distributions);
		CCS_RAISE(CCS_RESULT_ERROR_OUT_OF_MEMORY, "calloc failed");
	}

	for (size_t i = 0; i < num; i++) {
		cJSON *w = cJSON_GetArrayItem(j_weights, (int)i);
		CCS_REFUTE_ERR_GOTO(
			res, !w, CCS_RESULT_ERROR_INVALID_VALUE, end);
		CCS_VALIDATE_ERR_GOTO(
			res, _ccs_json_get_float(w, &weights[i]), end);
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_json_deserialize_array_object(
				cJSON_GetArrayItem(j_distributions, (int)i),
				CCS_OBJECT_TYPE_DISTRIBUTION, version,
				(ccs_object_t *)distributions + i, &new_opts),
			end);
	}
	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_mixture_distribution(
			num, distributions, weights, distribution_ret),
		end);
end:
	if (distributions) {
		for (size_t i = 0; i < num; i++)
			if (distributions[i])
				ccs_release_object(distributions[i]);
		free(distributions);
	}
	free(weights);
	return res;
}

static inline ccs_result_t
_ccs_deserialize_json_distribution_multivariate(
	ccs_distribution_t                *distribution_ret,
	uint32_t                           version,
	cJSON                             *json,
	_ccs_object_deserialize_options_t *opts)
{
	ccs_result_t                      res           = CCS_RESULT_SUCCESS;
	ccs_distribution_t               *distributions = NULL;

	_ccs_object_deserialize_options_t new_opts      = *opts;
	new_opts.handle_map                             = NULL;

	cJSON *j_distributions;
	size_t num;
	CCS_VALIDATE(_ccs_json_extract_array(
		json, "distributions", &j_distributions, &num));
	distributions =
		(ccs_distribution_t *)calloc(num, sizeof(ccs_distribution_t));
	CCS_REFUTE(!distributions, CCS_RESULT_ERROR_OUT_OF_MEMORY);

	for (size_t i = 0; i < num; i++)
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_json_deserialize_array_object(
				cJSON_GetArrayItem(j_distributions, (int)i),
				CCS_OBJECT_TYPE_DISTRIBUTION, version,
				(ccs_object_t *)distributions + i, &new_opts),
			end);
	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_multivariate_distribution(
			num, distributions, distribution_ret),
		end);
end:
	if (distributions) {
		for (size_t i = 0; i < num; i++)
			if (distributions[i])
				ccs_release_object(distributions[i]);
		free(distributions);
	}
	return res;
}

static inline ccs_result_t
_ccs_deserialize_json_distribution(
	ccs_distribution_t                *distribution_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	(void)buffer_size;
	cJSON                  *json = *(cJSON **)buffer;
	ccs_distribution_type_t dtype;
	cJSON                  *j_dtype =
		cJSON_GetObjectItemCaseSensitive(json, "distribution_type");
	const char *dtype_str;
	CCS_VALIDATE(_ccs_json_get_string(j_dtype, &dtype_str));
	CCS_VALIDATE(
		_ccs_json_distribution_type_from_string(dtype_str, &dtype));
	switch (dtype) {
	case CCS_DISTRIBUTION_TYPE_UNIFORM:
		CCS_VALIDATE(_ccs_deserialize_json_distribution_uniform(
			distribution_ret, json));
		break;
	case CCS_DISTRIBUTION_TYPE_NORMAL:
		CCS_VALIDATE(_ccs_deserialize_json_distribution_normal(
			distribution_ret, json));
		break;
	case CCS_DISTRIBUTION_TYPE_ROULETTE:
		CCS_VALIDATE(_ccs_deserialize_json_distribution_roulette(
			distribution_ret, json));
		break;
	case CCS_DISTRIBUTION_TYPE_MIXTURE:
		CCS_VALIDATE(_ccs_deserialize_json_distribution_mixture(
			distribution_ret, version, json, opts));
		break;
	case CCS_DISTRIBUTION_TYPE_MULTIVARIATE:
		CCS_VALIDATE(_ccs_deserialize_json_distribution_multivariate(
			distribution_ret, version, json, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_UNSUPPORTED_OPERATION,
			"Unsupported distribution type: %s",
			j_dtype->valuestring);
	}
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_distribution(
	ccs_distribution_t                *distribution_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	ccs_distribution_type_t dtype;
	CCS_VALIDATE(_ccs_peek_bin_ccs_distribution_type(
		&dtype, buffer_size, buffer));
	switch (dtype) {
	case CCS_DISTRIBUTION_TYPE_UNIFORM:
		CCS_VALIDATE(_ccs_deserialize_bin_distribution_uniform(
			distribution_ret, version, buffer_size, buffer));
		break;
	case CCS_DISTRIBUTION_TYPE_NORMAL:
		CCS_VALIDATE(_ccs_deserialize_bin_distribution_normal(
			distribution_ret, version, buffer_size, buffer));
		break;
	case CCS_DISTRIBUTION_TYPE_ROULETTE:
		CCS_VALIDATE(_ccs_deserialize_bin_distribution_roulette(
			distribution_ret, version, buffer_size, buffer));
		break;
	case CCS_DISTRIBUTION_TYPE_MIXTURE:
		CCS_VALIDATE(_ccs_deserialize_bin_distribution_mixture(
			distribution_ret, version, buffer_size, buffer, opts));
		break;
	case CCS_DISTRIBUTION_TYPE_MULTIVARIATE:
		CCS_VALIDATE(_ccs_deserialize_bin_distribution_multivariate(
			distribution_ret, version, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_UNSUPPORTED_OPERATION,
			"Unsupported distribution type: %d", dtype);
	}
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_distribution_deserialize(
	ccs_distribution_t                *distribution_ret,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_distribution(
			distribution_ret, version, buffer_size, buffer, opts));
		break;
	case CCS_SERIALIZE_FORMAT_JSON:
		CCS_VALIDATE(_ccs_deserialize_json_distribution(
			distribution_ret, version, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

#endif //_DISTRIBUTION_DESERIALIZE_H
