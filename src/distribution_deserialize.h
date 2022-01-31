#ifndef _DISTRIBUTION_DESERIALIZE_H
#define _DISTRIBUTION_DESERIALIZE_H
#include "cconfigspace_internal.h"
#include "distribution_internal.h"

struct _ccs_distribution_uniform_data_mock_s {
	_ccs_distribution_common_data_t common_data;
	ccs_numeric_type_t              data_type;
	ccs_scale_type_t                scale_type;
	ccs_numeric_t                   lower;
	ccs_numeric_t                   upper;
	ccs_numeric_t                   quantization;
};
typedef struct _ccs_distribution_uniform_data_mock_s _ccs_distribution_uniform_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_ccs_distribution_uniform_data(
		_ccs_distribution_uniform_data_mock_t  *data,
		size_t                                 *buffer_size,
		const char                            **buffer) {
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_distribution_common_data(
		&data->common_data, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_numeric_type(
		&data->data_type, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_scale_type(
		&data->scale_type, buffer_size, buffer));
	if (data->data_type == CCS_NUM_FLOAT) {
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
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_distribution_uniform(
		ccs_distribution_t  *distribution_ret,
		uint32_t             version,
		size_t              *buffer_size,
		const char         **buffer) {
	(void)version;
	_ccs_distribution_uniform_data_mock_t data;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_distribution_uniform_data(
		&data, buffer_size, buffer));
	CCS_VALIDATE(ccs_create_uniform_distribution(
		data.data_type,
		data.lower,
		data.upper,
		data.scale_type,
		data.quantization,
		distribution_ret));
	return CCS_SUCCESS;
}

struct _ccs_distribution_normal_data_mock_s {
	_ccs_distribution_common_data_t common_data;
	ccs_numeric_type_t              data_type;
	ccs_scale_type_t                scale_type;
	ccs_float_t                     mu;
	ccs_float_t                     sigma;
	ccs_numeric_t                   quantization;
};
typedef struct _ccs_distribution_normal_data_mock_s _ccs_distribution_normal_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_ccs_distribution_normal_data(
		_ccs_distribution_normal_data_mock_t  *data,
		size_t                                 *buffer_size,
		const char                            **buffer) {
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_distribution_common_data(
		&data->common_data, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_numeric_type(
		&data->data_type, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_scale_type(
		&data->scale_type, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_float(
		&data->mu, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_float(
		&data->sigma, buffer_size, buffer));
	if (data->data_type == CCS_NUM_FLOAT)
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_float(
			&data->quantization.f, buffer_size, buffer));
	else
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_int(
			&data->quantization.i, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_distribution_normal(
		ccs_distribution_t  *distribution_ret,
		uint32_t             version,
		size_t              *buffer_size,
		const char         **buffer) {
	(void)version;
	_ccs_distribution_normal_data_mock_t data;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_distribution_normal_data(
		&data, buffer_size, buffer));
	CCS_VALIDATE(ccs_create_normal_distribution(
		data.data_type,
		data.mu,
		data.sigma,
		data.scale_type,
		data.quantization,
		distribution_ret));
	return CCS_SUCCESS;
}

struct _ccs_distribution_roulette_data_mock_s {
	_ccs_distribution_common_data_t  common_data;
	size_t                           num_areas;
	ccs_float_t                     *areas;
};
typedef struct _ccs_distribution_roulette_data_mock_s _ccs_distribution_roulette_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_ccs_distribution_roulette_data(
		_ccs_distribution_roulette_data_mock_t  *data,
		size_t                                  *buffer_size,
		const char                             **buffer) {
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_distribution_common_data(
		&data->common_data, buffer_size, buffer));
	uint64_t num_areas;
	CCS_VALIDATE(_ccs_deserialize_bin_uint64(
		&num_areas, buffer_size, buffer));
	data->num_areas = num_areas;
	data->areas = (ccs_float_t *)malloc(num_areas*sizeof(ccs_float_t));
	if (!data->areas)
		return -CCS_OUT_OF_MEMORY;
	for (size_t i = 0; i < data->num_areas; i++)
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_float(
			data->areas + i, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_distribution_roulette(
		ccs_distribution_t  *distribution_ret,
		uint32_t             version,
		size_t              *buffer_size,
		const char         **buffer) {
	(void)version;
	ccs_result_t res = CCS_SUCCESS;
	_ccs_distribution_roulette_data_mock_t data;
	data.areas = NULL;
	CCS_VALIDATE_ERR_GOTO(res, _ccs_deserialize_bin_ccs_distribution_roulette_data(
		&data, buffer_size, buffer), end);
	CCS_VALIDATE_ERR_GOTO(res, ccs_create_roulette_distribution(
		data.num_areas,
		data.areas,
		distribution_ret), end);
end:
	if (data.areas)
		free(data.areas);
	return res;
}

static inline ccs_result_t
_ccs_deserialize_bin_distribution(
		ccs_distribution_t  *distribution_ret,
		uint32_t             version,
		size_t              *buffer_size,
		const char         **buffer);

struct _ccs_distribution_mixture_data_mock_s {
	_ccs_distribution_common_data_t  common_data;
	size_t                           num_distributions;
	ccs_distribution_t              *distributions;
        ccs_float_t                     *weights;
};
typedef struct _ccs_distribution_mixture_data_mock_s _ccs_distribution_mixture_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_ccs_distribution_mixture_data(
		_ccs_distribution_mixture_data_mock_t  *data,
		uint32_t                                version,
		size_t                                 *buffer_size,
		const char                            **buffer) {
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_distribution_common_data(
		&data->common_data, buffer_size, buffer));
	uint64_t num_distributions;
	CCS_VALIDATE(_ccs_deserialize_bin_uint64(
		&num_distributions, buffer_size, buffer));
	data->num_distributions = num_distributions;
	data->distributions = (ccs_distribution_t *)
		calloc(data->num_distributions, sizeof(ccs_distribution_t));
	if (!data->distributions)
		return -CCS_OUT_OF_MEMORY;
	data->weights = (ccs_float_t *)
		malloc(sizeof(ccs_float_t) * data->num_distributions);
	if (!data->weights)
		return -CCS_OUT_OF_MEMORY;
	for (size_t i = 0; i < data->num_distributions; i++) {
		CCS_VALIDATE(_ccs_deserialize_bin_ccs_float(
			data->weights + i, buffer_size, buffer));
		CCS_VALIDATE(_ccs_deserialize_bin_distribution(
			data->distributions + i, version, buffer_size, buffer));
	}
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_distribution_mixture(
		ccs_distribution_t  *distribution_ret,
		uint32_t             version,
		size_t              *buffer_size,
		const char         **buffer) {
	ccs_result_t res = CCS_SUCCESS;
	_ccs_distribution_mixture_data_mock_t data;
	data.distributions = NULL;
	data.weights = NULL;
	CCS_VALIDATE_ERR_GOTO(res, _ccs_deserialize_bin_ccs_distribution_mixture_data(
		&data, version, buffer_size, buffer), end);
	CCS_VALIDATE_ERR_GOTO(res, ccs_create_mixture_distribution(
		data.num_distributions,
		data.distributions,
		data.weights,
		distribution_ret), end);
end:
	if (data.distributions) {
		for (size_t i = 0; i < data.num_distributions; i++)
			if(data.distributions[i])
				ccs_release_object(data.distributions[i]);
		free(data.distributions);
	}
	if (data.weights)
		free(data.weights);
	return res;
}

struct _ccs_distribution_multivariate_data_mock_s {
	_ccs_distribution_common_data_t  common_data;
	size_t                           num_distributions;
	ccs_distribution_t              *distributions;
};
typedef struct _ccs_distribution_multivariate_data_mock_s _ccs_distribution_multivariate_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_ccs_distribution_multivariate_data(
		_ccs_distribution_multivariate_data_mock_t  *data,
		uint32_t                                     version,
		size_t                                      *buffer_size,
		const char                                 **buffer) {
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_distribution_common_data(
		&data->common_data, buffer_size, buffer));
	uint64_t num_distributions;
	CCS_VALIDATE(_ccs_deserialize_bin_uint64(
		&num_distributions, buffer_size, buffer));
	data->num_distributions = num_distributions;
	data->distributions = (ccs_distribution_t *)
		calloc(data->num_distributions, sizeof(ccs_distribution_t));
	if (!data->distributions)
		return -CCS_OUT_OF_MEMORY;
	for (size_t i = 0; i < data->num_distributions; i++)
		CCS_VALIDATE(_ccs_deserialize_bin_distribution(
			data->distributions + i, version, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_distribution_multivariate(
		ccs_distribution_t  *distribution_ret,
		uint32_t             version,
		size_t              *buffer_size,
		const char         **buffer) {
	ccs_result_t res = CCS_SUCCESS;
	_ccs_distribution_multivariate_data_mock_t data;
	data.distributions = NULL;
	CCS_VALIDATE_ERR_GOTO(res, _ccs_deserialize_bin_ccs_distribution_multivariate_data(
		&data, version, buffer_size, buffer), end);
	CCS_VALIDATE_ERR_GOTO(res, ccs_create_multivariate_distribution(
		data.num_distributions,
		data.distributions,
		distribution_ret), end);
end:
	if (data.distributions) {
		for (size_t i = 0; i < data.num_distributions; i++)
			if(data.distributions[i])
				ccs_release_object(data.distributions[i]);
		free(data.distributions);
	}
	return res;
}

static inline ccs_result_t
_ccs_deserialize_bin_distribution(
		ccs_distribution_t  *distribution_ret,
		uint32_t             version,
		size_t              *buffer_size,
		const char         **buffer) {
	_ccs_object_internal_t obj;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object_internal(
		&obj, buffer_size, buffer));
	if (CCS_UNLIKELY(obj.type != CCS_DISTRIBUTION))
		return -CCS_INVALID_TYPE;

	ccs_distribution_type_t dtype;
	CCS_VALIDATE(_ccs_peek_bin_ccs_distribution_type(
		&dtype, buffer_size, buffer));
	switch (dtype) {
	case CCS_UNIFORM:
		CCS_VALIDATE(_ccs_deserialize_bin_distribution_uniform(
			distribution_ret, version, buffer_size, buffer));
		break;
	case CCS_NORMAL:
		CCS_VALIDATE(_ccs_deserialize_bin_distribution_normal(
			distribution_ret, version, buffer_size, buffer));
		break;
	case CCS_ROULETTE:
		CCS_VALIDATE(_ccs_deserialize_bin_distribution_roulette(
			distribution_ret, version, buffer_size, buffer));
		break;
	case CCS_MIXTURE:
		CCS_VALIDATE(_ccs_deserialize_bin_distribution_mixture(
			distribution_ret, version, buffer_size, buffer));
		break;
	case CCS_MULTIVARIATE:
		CCS_VALIDATE(_ccs_deserialize_bin_distribution_multivariate(
			distribution_ret, version, buffer_size, buffer));
		break;
	default:
		return -CCS_UNSUPPORTED_OPERATION;
	}
	ccs_object_set_user_data(*distribution_ret, obj.user_data);

	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_distribution_deserialize(
		ccs_distribution_t      *distribution_ret,
		ccs_serialize_format_t   format,
		uint32_t                 version,
		size_t                  *buffer_size,
		const char             **buffer) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_distribution(
			distribution_ret, version, buffer_size, buffer));
		break;
	default:
		return -CCS_INVALID_VALUE;
	}
	return CCS_SUCCESS;
}

#endif //_DISTRIBUTION_DESERIALIZE_H
