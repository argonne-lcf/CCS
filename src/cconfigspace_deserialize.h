#ifndef _CCONFIG_SPACE_SPACE_DESERIALIZE_H
#define _CCONFIG_SPACE_SPACE_DESERIALIZE_H

static inline ccs_result_t
_ccs_object_deserialize_with_opts_check(
	ccs_object_t                      *object_ret,
	ccs_object_type_t                  expected_type,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts);

static inline ccs_result_t
_ccs_object_deserialize_with_opts(
	ccs_object_t                      *object_ret,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts);

#include "rng_deserialize.h"
#include "distribution_deserialize.h"
#include "parameter_deserialize.h"
#include "expression_deserialize.h"
#include "feature_space_deserialize.h"
#include "configuration_space_deserialize.h"
#include "distribution_space_deserialize.h"
#include "objective_space_deserialize.h"
#include "configuration_deserialize.h"
#include "evaluation_deserialize.h"
#include "features_deserialize.h"
#include "tuner_deserialize.h"
#include "map_deserialize.h"
#include "tree_deserialize.h"
#include "tree_space_deserialize.h"
#include "tree_configuration_deserialize.h"

static inline ccs_result_t
_ccs_object_deserialize_options(
	ccs_serialize_format_t             format,
	ccs_serialize_operation_t          operation,
	va_list                            args,
	_ccs_object_deserialize_options_t *opts)
{
	(void)format;
	ccs_deserialize_option_t opt =
		(ccs_deserialize_option_t)va_arg(args, int32_t);
	while (opt != CCS_DESERIALIZE_OPTION_END) {
		switch (opt) {
		case CCS_DESERIALIZE_OPTION_HANDLE_MAP:
			opts->handle_map = va_arg(args, ccs_map_t);
			CCS_CHECK_OBJ(opts->handle_map, CCS_OBJECT_TYPE_MAP);
			break;
		case CCS_DESERIALIZE_OPTION_VECTOR:
			opts->vector = va_arg(args, void *);
			CCS_CHECK_PTR(opts->vector);
			break;
		case CCS_DESERIALIZE_OPTION_DATA:
			opts->data = va_arg(args, void *);
			break;
		case CCS_DESERIALIZE_OPTION_NON_BLOCKING:
			CCS_REFUTE(
				operation !=
					CCS_SERIALIZE_OPERATION_FILE_DESCRIPTOR,
				CCS_RESULT_ERROR_INVALID_VALUE);
			opts->ppfd_state =
				va_arg(args, _ccs_file_descriptor_state_t **);
			CCS_CHECK_PTR(opts->ppfd_state);
			break;
		case CCS_DESERIALIZE_OPTION_CALLBACK:
			opts->deserialize_callback =
				va_arg(args, ccs_object_deserialize_callback_t);
			CCS_CHECK_PTR(opts->deserialize_callback);
			opts->deserialize_user_data = va_arg(args, void *);
			break;
		default:
			CCS_RAISE(
				CCS_RESULT_ERROR_INVALID_VALUE,
				"Unsupported deserialization option: %d", opt);
		}
		opt = (ccs_deserialize_option_t)va_arg(args, int32_t);
	}
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_object_deserialize_with_opts_type(
	ccs_object_t                      *object_ret,
	ccs_object_type_t                  otype,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	switch (otype) {
	case CCS_OBJECT_TYPE_RNG:
		CCS_VALIDATE(_ccs_rng_deserialize(
			(ccs_rng_t *)object_ret, format, version, buffer_size,
			buffer, opts));
		break;
	case CCS_OBJECT_TYPE_DISTRIBUTION:
		CCS_VALIDATE(_ccs_distribution_deserialize(
			(ccs_distribution_t *)object_ret, format, version,
			buffer_size, buffer, opts));
		break;
	case CCS_OBJECT_TYPE_PARAMETER:
		CCS_VALIDATE(_ccs_parameter_deserialize(
			(ccs_parameter_t *)object_ret, format, version,
			buffer_size, buffer, opts));
		break;
	case CCS_OBJECT_TYPE_EXPRESSION:
		CCS_VALIDATE(_ccs_expression_deserialize(
			(ccs_expression_t *)object_ret, format, version,
			buffer_size, buffer, opts));
		break;
	case CCS_OBJECT_TYPE_FEATURE_SPACE:
		CCS_VALIDATE(_ccs_feature_space_deserialize(
			(ccs_feature_space_t *)object_ret, format, version,
			buffer_size, buffer, opts));
		break;
	case CCS_OBJECT_TYPE_CONFIGURATION_SPACE:
		CCS_VALIDATE(_ccs_configuration_space_deserialize(
			(ccs_configuration_space_t *)object_ret, format,
			version, buffer_size, buffer, opts));
		break;
	case CCS_OBJECT_TYPE_OBJECTIVE_SPACE:
		CCS_VALIDATE(_ccs_objective_space_deserialize(
			(ccs_objective_space_t *)object_ret, format, version,
			buffer_size, buffer, opts));
		break;
	case CCS_OBJECT_TYPE_CONFIGURATION:
		CCS_VALIDATE(_ccs_configuration_deserialize(
			(ccs_configuration_t *)object_ret, format, version,
			buffer_size, buffer, opts));
		break;
	case CCS_OBJECT_TYPE_EVALUATION:
		CCS_VALIDATE(_ccs_evaluation_deserialize(
			(ccs_evaluation_t *)object_ret, format, version,
			buffer_size, buffer, opts));
		break;
	case CCS_OBJECT_TYPE_FEATURES:
		CCS_VALIDATE(_ccs_features_deserialize(
			(ccs_features_t *)object_ret, format, version,
			buffer_size, buffer, opts));
		break;
	case CCS_OBJECT_TYPE_TUNER:
		CCS_VALIDATE(_ccs_tuner_deserialize(
			(ccs_tuner_t *)object_ret, format, version, buffer_size,
			buffer, opts));
		break;
	case CCS_OBJECT_TYPE_MAP:
		CCS_VALIDATE(_ccs_map_deserialize(
			(ccs_map_t *)object_ret, format, version, buffer_size,
			buffer, opts));
		break;
	case CCS_OBJECT_TYPE_TREE:
		CCS_VALIDATE(_ccs_tree_deserialize(
			(ccs_tree_t *)object_ret, format, version, buffer_size,
			buffer, opts));
		break;
	case CCS_OBJECT_TYPE_TREE_SPACE:
		CCS_VALIDATE(_ccs_tree_space_deserialize(
			(ccs_tree_space_t *)object_ret, format, version,
			buffer_size, buffer, opts));
		break;
	case CCS_OBJECT_TYPE_TREE_CONFIGURATION:
		CCS_VALIDATE(_ccs_tree_configuration_deserialize(
			(ccs_tree_configuration_t *)object_ret, format, version,
			buffer_size, buffer, opts));
		break;
	case CCS_OBJECT_TYPE_DISTRIBUTION_SPACE:
		CCS_VALIDATE(_ccs_distribution_space_deserialize(
			(ccs_distribution_space_t *)object_ret, format, version,
			buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_UNSUPPORTED_OPERATION,
			"Unsupported object type: %d", otype);
	}
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_object_deserialize_with_opts_check(
	ccs_object_t                      *object_ret,
	ccs_object_type_t                  expected_type,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY: {
		ccs_object_type_t otype;
		CCS_VALIDATE(_ccs_peek_bin_ccs_object_type(
			&otype, buffer_size, buffer));
		CCS_REFUTE(
			otype != expected_type, CCS_RESULT_ERROR_INVALID_TYPE);
		CCS_VALIDATE(_ccs_object_deserialize_with_opts_type(
			object_ret, expected_type, format, version, buffer_size,
			buffer, opts));
	} break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_object_deserialize_with_opts(
	ccs_object_t                      *object_ret,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY: {
		ccs_object_type_t otype;
		CCS_VALIDATE(_ccs_peek_bin_ccs_object_type(
			&otype, buffer_size, buffer));
		CCS_VALIDATE(_ccs_object_deserialize_with_opts_type(
			object_ret, otype, format, version, buffer_size, buffer,
			opts));
	} break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

#endif //_CCONFIG_SPACE_SPACE_DESERIALIZE_H
