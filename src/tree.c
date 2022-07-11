#include "cconfigspace_internal.h"
#include "tree_internal.h"

static ccs_error_t
_ccs_tree_del(ccs_object_t o) {
	struct _ccs_tree_data_s *data = ((ccs_tree_t)o)->data;
	for (size_t i = 0; i < data->arity; i++)
		if (data->children[i]) {
			struct _ccs_tree_data_s *cd = data->children[i]->data;
			cd->parent = NULL;
			cd->index = 0;
			ccs_release_object(data->children[i]);
		}
	ccs_release_object(data->distribution);
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_size_ccs_tree_data(
		_ccs_tree_data_t                *data,
		size_t                          *cum_size,
                _ccs_object_serialize_options_t *opts) {
	*cum_size += _ccs_serialize_bin_size_size(data->arity) +
		_ccs_serialize_bin_size_ccs_float(data->weights[data->arity]);
	for (size_t i = 0; i < data->arity; i++) {
		_ccs_serialize_bin_size_ccs_bool(data->children[i] != NULL);
		CCS_VALIDATE(data->children[i]->obj.ops->serialize_size(
			data->children[i], CCS_SERIALIZE_FORMAT_BINARY, cum_size, opts));
	}
	*cum_size += _ccs_serialize_bin_size_ccs_float(data->bias) +
		_ccs_serialize_bin_size_ccs_datum(data->value);
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_tree_data(
		_ccs_tree_data_t                 *data,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	CCS_VALIDATE(_ccs_serialize_bin_size(
		data->arity, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_float(
		data->weights[data->arity], buffer_size, buffer));
	for (size_t i = 0; i < data->arity; i++) {
		CCS_VALIDATE(_ccs_serialize_bin_ccs_bool(
			data->children[i] != NULL, buffer_size, buffer));
		CCS_VALIDATE(data->children[i]->obj.ops->serialize(
			data->children[i], CCS_SERIALIZE_FORMAT_BINARY, buffer_size, buffer, opts));
	}
	CCS_VALIDATE(_ccs_serialize_bin_ccs_float(
		data->bias, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_datum(
		data->value, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_size_ccs_tree(
		ccs_tree_t                       tree,
		size_t                          *cum_size,
		_ccs_object_serialize_options_t *opts) {
	*cum_size += _ccs_serialize_bin_size_ccs_object_internal(
		(_ccs_object_internal_t *)tree);
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tree_data(
		tree->data, cum_size, opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_tree(
		ccs_tree_t                        tree,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		 (_ccs_object_internal_t *)tree, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_tree_data(
		tree->data, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tree_serialize_size(
		ccs_object_t                     object,
		ccs_serialize_format_t           format,
		size_t                          *cum_size,
		_ccs_object_serialize_options_t *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tree(
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
_ccs_tree_serialize(
		ccs_object_t                      object,
		ccs_serialize_format_t            format,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_tree(
		    (ccs_tree_t)object, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(CCS_INVALID_VALUE, "Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data(
		object, format, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static _ccs_tree_ops_t _ccs_tree_ops = {
	{ &_ccs_tree_del,
	  &_ccs_tree_serialize_size,
	  &_ccs_tree_serialize }
};

ccs_error_t
ccs_create_tree(
		size_t       arity,
		ccs_datum_t  value,
		ccs_tree_t  *tree_ret) {
	CCS_CHECK_PTR(tree_ret);
	CCS_REFUTE(value.type > CCS_STRING, CCS_INVALID_VALUE);
	CCS_REFUTE(arity > CCS_INT_MAX - 1, CCS_INVALID_VALUE);
	size_t size_strs = 0;
	if (value.type == CCS_STRING) {
		CCS_REFUTE(!value.value.s, CCS_INVALID_VALUE);
		size_strs += strlen(value.value.s) + 1;
	}

	uintptr_t mem = (uintptr_t)calloc(1,
		sizeof(struct _ccs_tree_s) +
		sizeof(_ccs_tree_data_t) +
		(arity + 1) * sizeof(ccs_float_t) +
		arity * sizeof(ccs_tree_t) +
		size_strs);
	CCS_REFUTE(!mem, CCS_OUT_OF_MEMORY);

	ccs_tree_t tree = (ccs_tree_t)mem;
	_ccs_object_init(&(tree->obj), CCS_TREE, (_ccs_object_ops_t *)&_ccs_tree_ops);
	_ccs_tree_data_t *data =
		(_ccs_tree_data_t *)(mem + sizeof(struct _ccs_tree_s));
	data->arity = arity;
	data->weights = (ccs_float_t *)(mem +
		sizeof(struct _ccs_tree_s) +
		sizeof(_ccs_tree_data_t));
	for (size_t j = 0; j < arity + 1; j++)
		data->weights[j] = 1.0;
	data->bias = 1.0;
	data->sum_weights = arity + 1;
	data->parent = NULL;
	data->distribution = NULL;
	data->children = (ccs_tree_t *)(mem +
		sizeof(struct _ccs_tree_s) +
		sizeof(_ccs_tree_data_t) +
		(arity + 1) * sizeof(ccs_float_t));
	if (value.type == CCS_STRING) {
		char *str_pool = (char *)(mem +
			sizeof(struct _ccs_tree_s) +
			sizeof(_ccs_tree_data_t) +
			(arity + 1) * sizeof(ccs_float_t) +
			arity * sizeof(ccs_tree_t));
		data->value = ccs_string(str_pool);
		strcpy(str_pool, value.value.s);
	} else {
		data->value = value;
		data->value.flags = CCS_FLAG_DEFAULT;
	}
	ccs_error_t err = CCS_SUCCESS;
	CCS_VALIDATE_ERR_GOTO(err, ccs_create_roulette_distribution(
		arity + 1, data->weights, &data->distribution), err_mem);
	tree->data = (_ccs_tree_data_t *)data;
	*tree_ret = tree;
	return CCS_SUCCESS;
err_mem:
	free((void *)mem);
	return err;
}

ccs_error_t
ccs_tree_get_value(
		ccs_tree_t   tree,
		ccs_datum_t *value_ret) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_PTR(value_ret);
	*value_ret = tree->data->value;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_get_arity(
		ccs_tree_t  tree,
		size_t     *arity_ret) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_PTR(arity_ret);
	*arity_ret = tree->data->arity;
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_tree_update_weight(
		ccs_tree_t  tree,
		size_t      index,
		ccs_float_t weight) {
	_ccs_tree_data_t *tree_data = tree->data;
	CCS_REFUTE(index > tree_data->arity, CCS_OUT_OF_BOUNDS);
	if (weight == tree_data->weights[index])
		return CCS_SUCCESS;
	do {
		size_t n = tree_data->arity + 1;
		ccs_float_t sum_weights = 0.0;
		tree_data->weights[index] = weight;
		for (size_t i = 0; i < n; i++)
			sum_weights += tree_data->weights[i];
		tree_data->sum_weights = sum_weights;
		if (sum_weights > 0)
			CCS_VALIDATE(ccs_roulette_distribution_set_areas(
				tree_data->distribution, n, tree_data->weights));
		if (tree_data->parent) {
			weight = sum_weights * tree_data->bias;
			index = tree_data->index;
			tree_data = tree_data->parent->data;
		} else
			tree_data = NULL;
	} while (tree_data);
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_tree_update_bias(
		ccs_tree_t  tree,
		ccs_float_t bias) {
	_ccs_tree_data_t *tree_data = tree->data;
	if (bias == tree_data->bias)
		return CCS_SUCCESS;
	tree_data->bias = bias;
	if (!tree_data->parent)
		return CCS_SUCCESS;
	ccs_float_t weight = tree_data->sum_weights * bias;
	CCS_VALIDATE(_ccs_tree_update_weight(
		tree_data->parent, tree_data->index, weight));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_set_child(
		ccs_tree_t tree,
		size_t     index,
		ccs_tree_t child) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_OBJ(child, CCS_TREE);
	_ccs_tree_data_t *tree_data = tree->data;

	CCS_REFUTE(index >= tree_data->arity, CCS_OUT_OF_BOUNDS);
	CCS_REFUTE(tree_data->children[index], CCS_INVALID_VALUE);
	_ccs_tree_data_t *child_data = child->data;
	CCS_REFUTE(child_data->parent || child_data->index, CCS_INVALID_TREE);
	CCS_VALIDATE(ccs_retain_object(child));

	child_data->parent = tree;
	child_data->index = index;
	tree_data->children[index] = child;

	ccs_error_t err;
	ccs_float_t weight = child_data->sum_weights * child_data->bias;
	CCS_VALIDATE_ERR_GOTO(err, _ccs_tree_update_weight(tree, index, weight), err_distrib);
	return CCS_SUCCESS;
err_distrib:
	ccs_release_object(child);
	child_data->parent = NULL;
	child_data->index = 0;
	tree_data->children[index] = NULL;
	_ccs_tree_update_weight(tree, index, 1.0);
	return err;
}

ccs_error_t
ccs_tree_get_child(
		ccs_tree_t  tree,
		size_t      index,
		ccs_tree_t *child_ret) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_PTR(child_ret);
	_ccs_tree_data_t *tree_data = tree->data;
	CCS_REFUTE(index >= tree_data->arity, CCS_OUT_OF_BOUNDS);
	*child_ret = tree_data->children[index];
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_get_children(
		ccs_tree_t  tree,
		size_t      num_children,
		ccs_tree_t *children,
		size_t     *num_children_ret) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_ARY(num_children, children);
	CCS_REFUTE(!children && !num_children_ret, CCS_INVALID_VALUE);
	_ccs_tree_data_t *data = tree->data;
	size_t arity = data->arity;
	if (children) {
		CCS_REFUTE(num_children < arity, CCS_INVALID_VALUE);
		for (size_t i = 0; i < arity; i++)
			children[i] = data->children[i];
		for (size_t i = arity; i < num_children; i++)
			children[i] = NULL;
	}
	if (num_children_ret)
		*num_children_ret = arity;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_get_weight(
		ccs_tree_t   tree,
		ccs_float_t *weight_ret) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_PTR(weight_ret);
	_ccs_tree_data_t *tree_data = tree->data;
	*weight_ret = tree_data->weights[tree_data->arity];
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_set_weight(
		ccs_tree_t   tree,
		ccs_float_t weight) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_REFUTE(weight < 0.0, CCS_INVALID_VALUE);
	_ccs_tree_data_t *tree_data = tree->data;
	size_t      index      = tree_data->arity;
	ccs_float_t old_weight = tree_data->weights[index];
	ccs_error_t err        = CCS_SUCCESS;
	CCS_VALIDATE_ERR_GOTO(err, _ccs_tree_update_weight(tree, index, weight), err_distrib);
	return CCS_SUCCESS;
err_distrib:
	_ccs_tree_update_weight(tree, index, old_weight);
	return err;
}

ccs_error_t
ccs_tree_get_bias(
		ccs_tree_t   tree,
		ccs_float_t *bias_ret) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_PTR(bias_ret);
	*bias_ret = tree->data->bias;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_set_bias(
		ccs_tree_t  tree,
		ccs_float_t bias) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_REFUTE(bias < 0.0, CCS_INVALID_VALUE);
	_ccs_tree_data_t *tree_data = tree->data;
	ccs_float_t old_bias = tree_data->bias;
	ccs_error_t err      = CCS_SUCCESS;
	CCS_VALIDATE_ERR_GOTO(err, _ccs_tree_update_bias(tree, bias), err_distrib);
	return CCS_SUCCESS;
err_distrib:
	_ccs_tree_update_bias(tree, old_bias);
	return err;
}

ccs_error_t
ccs_tree_get_parent(
		ccs_tree_t  tree,
		ccs_tree_t *parent_ret,
		size_t     *index_ret) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_PTR(parent_ret);
	ccs_tree_t parent = tree->data->parent;
	*parent_ret = parent;
	if (index_ret && parent)
		*index_ret = tree->data->index;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_get_position(
		ccs_tree_t  tree,
		size_t      position_size,
		size_t     *position,
		size_t     *position_size_ret) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_ARY(position_size, position);
	CCS_REFUTE(!position && !position_size_ret, CCS_INVALID_VALUE);
	size_t depth = 0;
	ccs_tree_t parent = tree->data->parent;
	while (parent) {
		depth++;
		parent = parent->data->parent;
	}
	if (position)
		CCS_REFUTE(position_size < depth, CCS_INVALID_VALUE);
	if (position_size_ret)
		*position_size_ret = depth;
	if (position) {
		for(size_t i = 0; i < depth; i++) {
			position[depth - i - 1] = tree->data->index;
			tree = tree->data->parent;
		}
		for(size_t i = depth; i < position_size; i++)
			position[i] = 0;
	}
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_get_values(
		ccs_tree_t   tree,
		size_t       num_values,
		ccs_datum_t *values,
		size_t      *num_values_ret) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_ARY(num_values, values);
	CCS_REFUTE(!values && !num_values_ret, CCS_INVALID_VALUE);
	size_t depth = 0;
	ccs_tree_t parent = tree->data->parent;
	while (parent) {
		depth++;
		parent = parent->data->parent;
	}
	if (values)
		CCS_REFUTE(num_values < depth + 1, CCS_INVALID_VALUE);
	if (num_values_ret)
		*num_values_ret = depth + 1;
	if (values) {
		for(size_t i = 0; i < depth + 1; i++) {
			values[depth - i] = tree->data->value;
			tree = tree->data->parent;
		}
		for(size_t i = depth + 1; i < num_values; i++)
			values[i] = ccs_none;
	}
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_position_is_valid(
		ccs_tree_t    tree,
		size_t        position_size,
		const size_t *position,
		ccs_bool_t   *is_valid_ret) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_ARY(position_size, position);
	CCS_CHECK_PTR(is_valid_ret);
	for (size_t i = 0; i < position_size; i++) {
		if (!tree || position[i] >= tree->data->arity) {
			*is_valid_ret = CCS_FALSE;
			return CCS_SUCCESS;
		}
		tree = tree->data->children[position[i]];
	}
	*is_valid_ret = CCS_TRUE;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_get_values_at_position(
		ccs_tree_t    tree,
		size_t        position_size,
		const size_t *position,
		size_t        num_values,
		ccs_datum_t  *values) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_ARY(position_size, position);
	CCS_CHECK_ARY(num_values, values);
	CCS_REFUTE(num_values < position_size + 1, CCS_INVALID_VALUE);
	*values++ = tree->data->value;
	for (size_t i = 0; i < position_size; i++) {
		CCS_VALIDATE(ccs_tree_get_child(tree, position[i], &tree));
		CCS_REFUTE(!tree, CCS_INVALID_TREE);
		*values++ = tree->data->value;
	}
	for (size_t i = position_size; i < num_values; i++)
		*values++ = ccs_none;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_get_node_at_position(
		ccs_tree_t    tree,
		size_t        position_size,
		const size_t *position,
		ccs_tree_t   *tree_ret) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_ARY(position_size, position);
	CCS_CHECK_PTR(tree_ret);
	while(position_size) {
		CCS_VALIDATE(ccs_tree_get_child(tree, *position, &tree));
		CCS_REFUTE(!tree, CCS_INVALID_TREE);
		position_size--;
		position++;
	}
	*tree_ret = tree;
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_tree_samples(
		_ccs_tree_data_t *data,
		ccs_rng_t         rng,
		size_t            num_indices,
		size_t           *indices) {
	CCS_REFUTE(data->sum_weights == 0.0, CCS_INVALID_DISTRIBUTION);
	CCS_VALIDATE(ccs_distribution_samples(
		data->distribution, rng, num_indices, (ccs_numeric_t *)indices));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_sample(
		ccs_tree_t          tree,
		ccs_rng_t           rng,
		size_t             *index_ret) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_OBJ(rng, CCS_RNG);
	CCS_CHECK_PTR(index_ret);
	_ccs_tree_data_t *data = (_ccs_tree_data_t *)(tree->data);
	CCS_VALIDATE(_ccs_tree_samples(
		data, rng, 1, index_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_samples(
		ccs_tree_t          tree,
		ccs_rng_t           rng,
		size_t              num_indices,
		size_t             *indices) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_OBJ(rng, CCS_RNG);
	CCS_CHECK_ARY(num_indices, indices);
	_ccs_tree_data_t *data = (_ccs_tree_data_t *)(tree->data);
	CCS_VALIDATE(_ccs_tree_samples(
		data, rng, num_indices, indices));
	return CCS_SUCCESS;
}
