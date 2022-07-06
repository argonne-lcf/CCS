#include "cconfigspace_internal.h"
#include "tree_internal.h"

struct _ccs_tree_dynamic_data_s {
	_ccs_tree_common_data_t  common_data;
};
typedef struct _ccs_tree_dynamic_data_s _ccs_tree_dynamic_data_t;

static ccs_error_t
_ccs_tree_dynamic_del(ccs_object_t o) {
	struct _ccs_tree_dynamic_data_s *data =
		(struct _ccs_tree_dynamic_data_s *)(((ccs_tree_t)o)->data);
	for (size_t i = 0; i < data->common_data.arity; i++)
		if (data->common_data.children[i]) {
			struct _ccs_tree_dynamic_data_s *cd =
				(struct _ccs_tree_dynamic_data_s *)(data->common_data.children[i]->data);
			cd->common_data.parent = NULL;
			cd->common_data.index = 0;
			ccs_release_object(data->common_data.children[i]);
		}
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_size_ccs_tree_dynamic_data(
		_ccs_tree_dynamic_data_t        *data,
		size_t                          *cum_size,
		_ccs_object_serialize_options_t *opts) {
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tree_common_data(
		&data->common_data, cum_size, opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_tree_dynamic_data(
		_ccs_tree_dynamic_data_t         *data,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	CCS_VALIDATE(_ccs_serialize_bin_ccs_tree_common_data(
		&data->common_data, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_size_ccs_tree_dynamic(
		ccs_tree_t                       tree,
		size_t                          *cum_size,
		_ccs_object_serialize_options_t *opts) {
	_ccs_tree_dynamic_data_t *data =
		(_ccs_tree_dynamic_data_t *)tree->data;
	*cum_size += _ccs_serialize_bin_size_ccs_object_internal(
		(_ccs_object_internal_t *)tree);
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tree_dynamic_data(
		data, cum_size, opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_tree_dynamic(
		ccs_tree_t                        tree,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	_ccs_tree_dynamic_data_t *data =
		(_ccs_tree_dynamic_data_t *)tree->data;
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		 (_ccs_object_internal_t *)tree, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_tree_dynamic_data(
		data, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tree_dynamic_serialize_size(
		ccs_object_t                     object,
		ccs_serialize_format_t           format,
		size_t                          *cum_size,
		_ccs_object_serialize_options_t *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tree_dynamic(
			(ccs_tree_t)object, cum_size, opts));
		break;
	default:
		CCS_RAISE(CCS_INVALID_VALUE, "Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data_size(
		object, format, cum_size, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tree_dynamic_serialize(
		ccs_object_t                      object,
		ccs_serialize_format_t            format,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_tree_dynamic(
		    (ccs_tree_t)object, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(CCS_INVALID_VALUE, "Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data(
		object, format, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static _ccs_tree_ops_t _ccs_tree_dynamic_ops = {
	{ &_ccs_tree_dynamic_del,
	  &_ccs_tree_dynamic_serialize_size,
	  &_ccs_tree_dynamic_serialize }
};

ccs_error_t
ccs_create_dynamic_tree(
		size_t       arity,
		ccs_datum_t  value,
		ccs_tree_t  *tree_ret) {
	CCS_CHECK_PTR(tree_ret);
	CCS_REFUTE(value.type > CCS_STRING, CCS_INVALID_VALUE);
	CCS_REFUTE(arity > CCS_INT_MAX, CCS_INVALID_VALUE);
	size_t size_strs = 0;
	if (value.type == CCS_STRING) {
		CCS_REFUTE(!value.value.s, CCS_INVALID_VALUE);
		size_strs += strlen(value.value.s) + 1;
	}

	uintptr_t mem = (uintptr_t)calloc(1,
		sizeof(struct _ccs_tree_s) +
		sizeof(_ccs_tree_dynamic_data_t) +
		(arity + 1) * sizeof(ccs_float_t) +
		arity * sizeof(ccs_tree_t) +
		size_strs);
	CCS_REFUTE(!mem, CCS_OUT_OF_MEMORY);
	ccs_tree_t tree = (ccs_tree_t)mem;
	_ccs_object_init(&(tree->obj), CCS_TREE, (_ccs_object_ops_t *)&_ccs_tree_dynamic_ops);
	_ccs_tree_dynamic_data_t *data =
		(_ccs_tree_dynamic_data_t *)(mem + sizeof(struct _ccs_tree_s));
	data->common_data.type = CCS_TREE_TYPE_DYNAMIC;
	data->common_data.arity = arity;
	data->common_data.weights = (ccs_float_t *)(mem +
		sizeof(struct _ccs_tree_s) +
		sizeof(_ccs_tree_dynamic_data_t));
	for (size_t j = 0; j < arity + 1; j++)
		data->common_data.weights[j] = 1.0;
	data->common_data.bias = 1.0;
	data->common_data.sum_weights = arity + 1;
	data->common_data.parent = NULL;
	data->common_data.distribution = NULL;
	data->common_data.children = (ccs_tree_t *)(mem +
		sizeof(struct _ccs_tree_s) +
		sizeof(_ccs_tree_dynamic_data_t) +
		(arity + 1) * sizeof(ccs_float_t));
	if (value.type == CCS_STRING) {
		char *str_pool = (char *)(mem +
			sizeof(struct _ccs_tree_s) +
			sizeof(_ccs_tree_dynamic_data_t) +
			(arity + 1) * sizeof(ccs_float_t) +
			arity * sizeof(ccs_tree_t));
		data->common_data.value = ccs_string(str_pool);
		strcpy(str_pool, value.value.s);
	} else {
		data->common_data.value = value;
		data->common_data.value.flags = CCS_FLAG_DEFAULT;
	}
	tree->data = (_ccs_tree_data_t *)data;
	*tree_ret = tree;
	return CCS_SUCCESS;
}
