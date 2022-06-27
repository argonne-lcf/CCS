#include "cconfigspace_internal.h"
#include "tree_space_internal.h"
#include "tree_internal.h"

struct _ccs_tree_space_static_data_s {
	_ccs_tree_space_common_data_t  common_data;
};
typedef struct _ccs_tree_space_static_data_s _ccs_tree_space_static_data_t;

static ccs_error_t
_ccs_tree_space_static_del(ccs_object_t o) {
	struct _ccs_tree_space_static_data_s *data =
		(struct _ccs_tree_space_static_data_s *)(((ccs_tree_space_t)o)->data);
	ccs_release_object(data->common_data.rng);
	ccs_release_object(data->common_data.tree);
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_size_ccs_tree_space_static_data(
		_ccs_tree_space_static_data_t   *data,
		size_t                          *cum_size,
		_ccs_object_serialize_options_t *opts) {
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tree_space_common_data(
		&data->common_data, cum_size, opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_tree_space_static_data(
		_ccs_tree_space_static_data_t    *data,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	CCS_VALIDATE(_ccs_serialize_bin_ccs_tree_space_common_data(
		&data->common_data, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t 
_ccs_serialize_bin_size_ccs_tree_space_static(
		ccs_tree_space_t                 tree_space,
		size_t                          *cum_size,
		_ccs_object_serialize_options_t *opts) {
	_ccs_tree_space_static_data_t *data =
		(_ccs_tree_space_static_data_t *)tree_space->data;
	*cum_size += _ccs_serialize_bin_size_ccs_object_internal(
		(_ccs_object_internal_t *)tree_space);
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tree_space_static_data(
		data, cum_size, opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_tree_space_static(
		ccs_tree_space_t                  tree_space,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	_ccs_tree_space_static_data_t *data =
		(_ccs_tree_space_static_data_t *)tree_space->data;
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		 (_ccs_object_internal_t *)tree_space, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_tree_space_static_data(
		data, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tree_space_static_serialize_size(
		ccs_object_t                     object,
		ccs_serialize_format_t           format,
		size_t                          *cum_size,
		_ccs_object_serialize_options_t *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tree_space_static(
			(ccs_tree_space_t)object, cum_size, opts));
		break;
	default:
		CCS_RAISE(CCS_INVALID_VALUE, "Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data_size(
		object, format, cum_size, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tree_space_static_serialize(
		ccs_object_t                      object,
		ccs_serialize_format_t            format,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_tree_space_static(
		    (ccs_tree_space_t)object, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(CCS_INVALID_VALUE, "Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data(
		object, format, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static _ccs_tree_space_ops_t _ccs_tree_space_static_ops = {
	{ &_ccs_tree_space_static_del,
	  &_ccs_tree_space_static_serialize_size,
	  &_ccs_tree_space_static_serialize }
};

ccs_error_t
ccs_create_static_tree_space(
		const char       *name,
		ccs_tree_t        tree,
		ccs_tree_space_t *tree_space_ret) {
	CCS_CHECK_PTR(name);
	CCS_CHECK_TREE(tree, CCS_TREE_TYPE_STATIC);
	CCS_CHECK_PTR(tree_space_ret);
	ccs_error_t err;
	uintptr_t mem = (uintptr_t)calloc(1,
		sizeof(struct _ccs_tree_space_s) +
		sizeof(struct _ccs_tree_space_static_data_s) +
		strlen(name) + 1);
	CCS_REFUTE(!mem, CCS_OUT_OF_MEMORY);
	ccs_rng_t rng;
	CCS_VALIDATE_ERR_GOTO(err, ccs_create_rng(&rng), errmem);
	CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(tree), err_rng);

	ccs_tree_space_t tree_space;
	tree_space = (ccs_tree_space_t)mem;
	_ccs_object_init(&(tree_space->obj), CCS_TREE_SPACE,
		(_ccs_object_ops_t *)&_ccs_tree_space_static_ops);
	_ccs_tree_space_static_data_t *data;
	data = (struct _ccs_tree_space_static_data_s *)(mem +
		sizeof(struct _ccs_tree_space_s));
	data->common_data.type = CCS_TREE_SPACE_TYPE_STATIC;
	data->common_data.name = (const char *)(mem +
		sizeof(struct _ccs_tree_space_s) +
		sizeof(struct _ccs_tree_space_static_data_s));
	data->common_data.rng = rng;
	data->common_data.tree = tree;
	strcpy((char *)(data->common_data.name), name);
	tree_space->data = (_ccs_tree_space_data_t *)data;
	*tree_space_ret = tree_space;
	return CCS_SUCCESS;
err_rng:
	ccs_release_object(rng);
errmem:
	free((void *)mem);
	return err;
}
