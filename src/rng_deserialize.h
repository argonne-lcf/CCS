#ifndef _RNG_DESERIALIZE_H
#define _RNG_DESERIALIZE_H
#include "cconfigspace_internal.h"
#include "rng_internal.h"

static const gsl_rng_type **_ccs_gsl_rng_types = NULL;

static inline ccs_result_t
_ccs_deserialize_bin_rng(
		ccs_rng_t                          *rng_ret,
		uint32_t                            version,
		size_t                             *buffer_size,
		const char                        **buffer,
		_ccs_object_deserialize_options_t  *opts) {
	(void)version;
	ccs_result_t res;
	_ccs_object_internal_t obj;
	ccs_object_t handle;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object_internal(
		&obj, buffer_size, buffer, &handle));
	if (CCS_UNLIKELY(obj.type != CCS_RNG))
		return -CCS_INVALID_TYPE;

	const char *name;
	ccs_bool_t little_endian;
	_ccs_blob_t b;
	CCS_VALIDATE(_ccs_deserialize_bin_string(&name, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_bool(&little_endian, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_blob(&b, buffer_size, buffer));

	if (!_ccs_gsl_rng_types)
		_ccs_gsl_rng_types = gsl_rng_types_setup();

	const gsl_rng_type **t;

	for (t = _ccs_gsl_rng_types; *t != NULL; t++)
		if (!strcmp(name, (*t)->name))
			break;
	if (CCS_UNLIKELY(!*t))
		return -CCS_INVALID_VALUE;

	CCS_VALIDATE(ccs_rng_create_with_type(*t, rng_ret));
	/* try to restore the state of the rng, might be non portable,
	   so silently bail if failure */
	if (b.sz == gsl_rng_size((*rng_ret)->data->rng) &&
	    little_endian == ccs_is_little_endian())
		memcpy(gsl_rng_state((*rng_ret)->data->rng), b.blob, b.sz);
	CCS_VALIDATE_ERR_GOTO(res,
		ccs_object_set_user_data(*rng_ret, obj.user_data),
		err_rng);
	if (opts->handle_map)
		CCS_VALIDATE_ERR_GOTO(res,
			_ccs_object_handle_check_add(
				opts->handle_map, handle,
				(ccs_object_t)*rng_ret),
			err_rng);

	return CCS_SUCCESS;
err_rng:
	ccs_release_object(*rng_ret);
	*rng_ret = NULL;
	return res;
}

static ccs_result_t
_ccs_rng_deserialize(
		ccs_rng_t                          *rng_ret,
		ccs_serialize_format_t              format,
		uint32_t                            version,
		size_t                             *buffer_size,
		const char                        **buffer,
		_ccs_object_deserialize_options_t  *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_rng(
			rng_ret, version, buffer_size, buffer, opts));
		break;
	default:
		return -CCS_INVALID_VALUE;
	}
	return CCS_SUCCESS;
}

#endif //_RNG_DESERIALIZE_H
