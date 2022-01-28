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
	default:
		return -CCS_UNSUPPORTED_OPERATION;
	}
	ccs_object_set_user_data(*distribution_ret, obj.user_data);

	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_distribution_deserialize(
		ccs_distribution_t    *distribution_ret,
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
