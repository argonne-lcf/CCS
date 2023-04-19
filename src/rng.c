#include "cconfigspace_internal.h"
#include "rng_internal.h"
#include <stdlib.h>

static ccs_error_t
_ccs_rng_del(ccs_object_t object)
{
	gsl_rng_free(((ccs_rng_t)object)->data->rng);
	((ccs_rng_t)object)->data->rng = NULL;
	return CCS_SUCCESS;
}

static inline size_t
_ccs_serialize_bin_size_ccs_rng_data(_ccs_rng_data_t *data)
{
	_ccs_blob_t b = {gsl_rng_size(data->rng), gsl_rng_state(data->rng)};
	return _ccs_serialize_bin_size_string(gsl_rng_name(data->rng)) +
	       _ccs_serialize_bin_size_ccs_bool(ccs_is_little_endian()) +
	       _ccs_serialize_bin_size_ccs_blob(&b);
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_rng_data(
	_ccs_rng_data_t *data,
	size_t          *buffer_size,
	char           **buffer)
{
	CCS_VALIDATE(_ccs_serialize_bin_string(
		gsl_rng_name(data->rng), buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_bool(
		ccs_is_little_endian(), buffer_size, buffer));
	_ccs_blob_t b = {gsl_rng_size(data->rng), gsl_rng_state(data->rng)};
	CCS_VALIDATE(_ccs_serialize_bin_ccs_blob(&b, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline size_t
_ccs_serialize_bin_size_ccs_rng(ccs_rng_t rng)
{
	_ccs_rng_data_t *data = (_ccs_rng_data_t *)(rng->data);
	return _ccs_serialize_bin_size_ccs_object_internal(
		       (_ccs_object_internal_t *)rng) +
	       _ccs_serialize_bin_size_ccs_rng_data(data);
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_rng(ccs_rng_t rng, size_t *buffer_size, char **buffer)
{
	_ccs_rng_data_t *data = (_ccs_rng_data_t *)(rng->data);
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)rng, buffer_size, buffer));
	CCS_VALIDATE(
		_ccs_serialize_bin_ccs_rng_data(data, buffer_size, buffer));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_rng_serialize_size(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		*cum_size += _ccs_serialize_bin_size_ccs_rng((ccs_rng_t)object);
		break;
	default:
		CCS_RAISE(
			CCS_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data_size(
		object, format, cum_size, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_rng_serialize(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_rng(
			(ccs_rng_t)object, buffer_size, buffer));
		break;
	default:
		CCS_RAISE(
			CCS_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data(
		object, format, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static struct _ccs_rng_ops_s _rng_ops = {
	{&_ccs_rng_del, &_ccs_rng_serialize_size, &_ccs_rng_serialize}};

ccs_error_t
ccs_create_rng_with_type(const gsl_rng_type *rng_type, ccs_rng_t *rng_ret)
{
	ccs_error_t res = CCS_SUCCESS;
	ccs_rng_t   rng;
	CCS_CHECK_PTR(rng_type);
	CCS_CHECK_PTR(rng_ret);
	gsl_rng *grng = gsl_rng_alloc(rng_type);

	CCS_REFUTE(!grng, CCS_OUT_OF_MEMORY);
	uintptr_t mem = (uintptr_t)calloc(
		1, sizeof(struct _ccs_rng_s) + sizeof(struct _ccs_rng_data_s));
	CCS_REFUTE_ERR_GOTO(res, !mem, CCS_OUT_OF_MEMORY, err_rng);
	rng = (ccs_rng_t)mem;
	_ccs_object_init(
		&(rng->obj), CCS_OBJECT_TYPE_RNG,
		(_ccs_object_ops_t *)&_rng_ops);
	rng->data = (struct _ccs_rng_data_s *)(mem + sizeof(struct _ccs_rng_s));
	rng->data->rng_type = rng_type;
	rng->data->rng      = grng;
	*rng_ret            = rng;
	return CCS_SUCCESS;
err_rng:
	gsl_rng_free(grng);
	return res;
}

ccs_error_t
ccs_create_rng(ccs_rng_t *rng_ret)
{
	return ccs_create_rng_with_type(gsl_rng_default, rng_ret);
}

ccs_error_t
ccs_rng_get_type(ccs_rng_t rng, const gsl_rng_type **rng_type_ret)
{
	CCS_CHECK_OBJ(rng, CCS_OBJECT_TYPE_RNG);
	*rng_type_ret = rng->data->rng_type;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_rng_set_seed(ccs_rng_t rng, unsigned long int seed)
{
	CCS_CHECK_OBJ(rng, CCS_OBJECT_TYPE_RNG);
	gsl_rng_set(rng->data->rng, seed);
	return CCS_SUCCESS;
}

ccs_error_t
ccs_rng_get(ccs_rng_t rng, unsigned long int *value_ret)
{
	CCS_CHECK_OBJ(rng, CCS_OBJECT_TYPE_RNG);
	CCS_CHECK_PTR(value_ret);
	*value_ret = gsl_rng_get(rng->data->rng);
	return CCS_SUCCESS;
}

ccs_error_t
ccs_rng_uniform(ccs_rng_t rng, ccs_float_t *value_ret)
{
	CCS_CHECK_OBJ(rng, CCS_OBJECT_TYPE_RNG);
	CCS_CHECK_PTR(value_ret);
	*value_ret = gsl_rng_uniform(rng->data->rng);
	return CCS_SUCCESS;
}

ccs_error_t
ccs_rng_get_gsl_rng(ccs_rng_t rng, gsl_rng **gsl_rng_ret)
{
	CCS_CHECK_OBJ(rng, CCS_OBJECT_TYPE_RNG);
	CCS_CHECK_PTR(gsl_rng_ret);
	*gsl_rng_ret = rng->data->rng;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_rng_min(ccs_rng_t rng, unsigned long int *value_ret)
{
	CCS_CHECK_OBJ(rng, CCS_OBJECT_TYPE_RNG);
	CCS_CHECK_PTR(value_ret);
	*value_ret = gsl_rng_min(rng->data->rng);
	return CCS_SUCCESS;
}

ccs_error_t
ccs_rng_max(ccs_rng_t rng, unsigned long int *value_ret)
{
	CCS_CHECK_OBJ(rng, CCS_OBJECT_TYPE_RNG);
	CCS_CHECK_PTR(value_ret);
	*value_ret = gsl_rng_max(rng->data->rng);
	return CCS_SUCCESS;
}
