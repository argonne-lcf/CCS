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
		return CCS_INVALID_TYPE;

	ccs_hyperparameter_type_t htype;
	CCS_VALIDATE(_ccs_peek_bin_ccs_hyperparameter_type(
		&htype, buffer_size, buffer));
	switch (htype) {
	case CCS_HYPERPARAMETER_TYPE_NUMERICAL:
		CCS_VALIDATE(_ccs_deserialize_bin_hyperparameter_numerical(
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
