#ifndef _CONTEXT_DESERIALIZE_H
#define _CONTEXT_DESERIALIZE_H
#include "cconfigspace_internal.h"
#include "hyperparameter_deserialize.h"

struct _ccs_context_data_mock_s {
	const char           *name;
	size_t                num_hyperparameters;
	ccs_hyperparameter_t *hyperparameters;
};
typedef struct _ccs_context_data_mock_s _ccs_context_data_mock_t;

static inline ccs_error_t
_ccs_deserialize_bin_ccs_context_data(
		_ccs_context_data_mock_t           *data,
		uint32_t                            version,
		size_t                             *buffer_size,
		const char                        **buffer,
		_ccs_object_deserialize_options_t  *opts) {
	data->num_hyperparameters = 0;
	data->hyperparameters = NULL;
	CCS_VALIDATE(_ccs_deserialize_bin_string(
		&data->name, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->num_hyperparameters, buffer_size, buffer));
	if (data->num_hyperparameters) {
		data->hyperparameters = (ccs_hyperparameter_t *)
			calloc(data->num_hyperparameters, sizeof(ccs_hyperparameter_t));
		CCS_REFUTE(!data->hyperparameters, CCS_OUT_OF_MEMORY);
		for (size_t i = 0; i < data->num_hyperparameters; i++)
			CCS_VALIDATE(_ccs_hyperparameter_deserialize(
				data->hyperparameters + i, CCS_SERIALIZE_FORMAT_BINARY, version, buffer_size, buffer, opts));
	}
	return CCS_SUCCESS;
}

#endif //_CONTEXT_DESERIALIZE_H
