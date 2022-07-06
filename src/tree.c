#include "cconfigspace_internal.h"
#include "tree_internal.h"

static inline _ccs_tree_ops_t *
_ccs_tree_get_ops(ccs_tree_t tree) {
	return (_ccs_tree_ops_t *)tree->obj.ops;
}

ccs_error_t
ccs_tree_get_type(
		ccs_tree_t       tree,
		ccs_tree_type_t *type_ret) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_PTR(type_ret);
	*type_ret = ((_ccs_tree_common_data_t *)(tree->data))->type;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_get_value(
		ccs_tree_t   tree,
		ccs_datum_t *value_ret) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_PTR(value_ret);
	*value_ret = ((_ccs_tree_common_data_t *)(tree->data))->value;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_get_arity(
		ccs_tree_t  tree,
		size_t     *arity_ret) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_PTR(arity_ret);
	*arity_ret = ((_ccs_tree_common_data_t *)(tree->data))->arity;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_set_child(
		ccs_tree_t tree,
		size_t     index,
		ccs_tree_t child) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	_ccs_tree_common_data_t *tree_data =
		(_ccs_tree_common_data_t *)tree->data;
	CCS_CHECK_TREE(child, tree_data->type);

	CCS_REFUTE(index >= tree_data->arity, CCS_OUT_OF_BOUNDS);
	CCS_REFUTE(tree_data->children[index], CCS_INVALID_VALUE);
	_ccs_tree_common_data_t *child_data =
		(_ccs_tree_common_data_t *)child->data;
	CCS_REFUTE(child_data->parent || child_data->index, CCS_INVALID_TREE);
	ccs_float_t weight = child_data->sum_weights * child_data->bias;
	CCS_VALIDATE(ccs_retain_object(child));

	size_t indx = index;
	child_data->parent = tree;
	child_data->index = indx;
	tree_data->children[indx] = child;
	if (weight == tree_data->weights[indx])
		return CCS_SUCCESS;

	/* Update all parent distributions of child */
	do {
		tree_data->weights[indx] = weight;
		size_t n = tree_data->arity + 1;
		ccs_float_t sum_weights = 0.0;
		for (size_t i = 0; i < n; i++)
			sum_weights += tree_data->weights[i];
		tree_data->sum_weights = sum_weights;
		/* Should not fail, so no error management, would require removing the child
		   and updating all distributions until the one that failed.

		   Possible optimization could be defering distribution weight update */
		if (sum_weights > 0)
			CCS_VALIDATE(ccs_roulette_distribution_set_areas(
				tree_data->distribution, n, tree_data->weights));
		if (tree_data->parent) {
			weight = sum_weights * tree_data->bias;
			indx = tree_data->index;
			tree_data = (_ccs_tree_common_data_t *)tree_data->parent->data;
		} else
			tree_data = NULL;
	} while (tree_data);
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_get_child(
		ccs_tree_t  tree,
		size_t      index,
		ccs_tree_t *child_ret) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_PTR(child_ret);
	_ccs_tree_common_data_t *tree_data =
		(_ccs_tree_common_data_t *)tree->data;
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
	_ccs_tree_common_data_t *data =
		 (_ccs_tree_common_data_t *)tree->data;
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
ccs_tree_get_parent(
		ccs_tree_t  tree,
		ccs_tree_t *parent_ret,
		size_t     *index_ret) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_PTR(parent_ret);
	ccs_tree_t parent = ((_ccs_tree_common_data_t *)(tree->data))->parent;
	*parent_ret = parent;
	if (index_ret && parent)
		*index_ret = ((_ccs_tree_common_data_t *)(tree->data))->index;
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
	ccs_tree_t parent = ((_ccs_tree_common_data_t *)(tree->data))->parent;
	while (parent) {
		depth++;
		parent = ((_ccs_tree_common_data_t *)(parent->data))->parent;
	}
	if (position)
		CCS_REFUTE(position_size < depth, CCS_INVALID_VALUE);
	if (position_size_ret)
		*position_size_ret = depth;
	if (position) {
		size_t index = ((_ccs_tree_common_data_t *)(tree->data))->index;
		parent = ((_ccs_tree_common_data_t *)(tree->data))->parent;
		for(size_t i = depth; i < position_size; i++)
			position[i] = 0;
		while (parent) {
			depth--;
			position[depth] = index;
			index = ((_ccs_tree_common_data_t *)(parent->data))->index;
			parent = ((_ccs_tree_common_data_t *)(parent->data))->parent;
		}
	}
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_get_node_at_position(
		ccs_tree_t  tree,
		size_t      position_size,
		size_t     *position,
		ccs_tree_t *tree_ret) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_ARY(position_size, position);
	CCS_CHECK_PTR(tree_ret);
	*tree_ret = tree;
	while(position_size) {
		CCS_VALIDATE(ccs_tree_get_child(*tree_ret, *position, tree_ret));
		CCS_REFUTE(!*tree_ret, CCS_INVALID_TREE);
		position_size--;
		position++;
	}
	return CCS_SUCCESS;
}

static inline void
_ccs_set_tree_interval(
		_ccs_tree_common_data_t *data,
		ccs_interval_t          *interval) {
	interval->type = CCS_NUM_INTEGER;
	interval->lower.i = 0;
	interval->upper.i = data->arity;
	interval->lower_included = CCS_TRUE;
	interval->upper_included = CCS_TRUE;
}

static inline ccs_error_t
_ccs_tree_validate_distribution(
		_ccs_tree_common_data_t *data,
		ccs_distribution_t       distribution) {
	CCS_CHECK_OBJ(distribution, CCS_DISTRIBUTION);
	size_t dimension;
	ccs_interval_t interval, interval_dis;
	ccs_bool_t equal;
	_ccs_set_tree_interval(data, &interval);
	CCS_VALIDATE(ccs_distribution_get_dimension(distribution, &dimension));
	CCS_REFUTE(dimension != 1, CCS_INVALID_DISTRIBUTION);
	CCS_VALIDATE(ccs_distribution_get_bounds(distribution, &interval_dis));
	CCS_VALIDATE(ccs_interval_equal(&interval, &interval_dis, &equal));
	CCS_REFUTE(!equal, CCS_INVALID_DISTRIBUTION);
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_set_distribution(
		ccs_tree_t         tree,
		ccs_distribution_t distribution) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	_ccs_tree_common_data_t *data = (_ccs_tree_common_data_t *)(tree->data);
	ccs_error_t err = CCS_SUCCESS;
	if (distribution) {
		CCS_VALIDATE(_ccs_tree_validate_distribution(data, distribution));
		CCS_VALIDATE(ccs_retain_object(distribution));
	}
	if (data->distribution)
		CCS_VALIDATE_ERR_GOTO(err, ccs_release_object(data->distribution), err_distrib);
	data->distribution = distribution;
	return CCS_SUCCESS;
err_distrib:
	ccs_release_object(distribution);
	return err;
}

ccs_error_t
ccs_tree_get_distribution(
		ccs_tree_t          tree,
		ccs_distribution_t *distribution_ret) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_PTR(distribution_ret);
	*distribution_ret = ((_ccs_tree_common_data_t *)(tree->data))->distribution;
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_tree_samples(
		_ccs_tree_common_data_t *data,
		ccs_distribution_t       distribution,
		ccs_rng_t                rng,
		size_t                   num_indices,
		size_t                  *indices) {
	if (distribution)
		CCS_VALIDATE(_ccs_tree_validate_distribution(data, distribution));
	else {
		CCS_REFUTE(data->sum_weights == 0.0, CCS_INVALID_DISTRIBUTION);
		distribution = data->distribution;
	}
	gsl_rng *grng;
	CCS_VALIDATE(ccs_rng_get_gsl_rng(rng, &grng));
	size_t arity = data->arity;
	if (distribution) {
		CCS_VALIDATE(ccs_distribution_samples(
			distribution, rng, num_indices, (ccs_numeric_t *)indices));
	} else {
		for (size_t i = 0; i < num_indices; i++)
			indices[i] =  gsl_rng_uniform_int(grng, (unsigned long int)(arity + 1));
	}
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_sample(
		ccs_tree_t          tree,
		ccs_distribution_t  distribution,
		ccs_rng_t           rng,
		size_t             *index_ret) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_OBJ(rng, CCS_RNG);
	CCS_CHECK_PTR(index_ret);
	_ccs_tree_common_data_t *data = (_ccs_tree_common_data_t *)(tree->data);
	CCS_VALIDATE(_ccs_tree_samples(
		data, distribution, rng, 1, index_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_tree_samples(
		ccs_tree_t          tree,
		ccs_distribution_t  distribution,
		ccs_rng_t           rng,
		size_t              num_indices,
		size_t             *indices) {
	CCS_CHECK_OBJ(tree, CCS_TREE);
	CCS_CHECK_OBJ(rng, CCS_RNG);
	CCS_CHECK_ARY(num_indices, indices);
	_ccs_tree_common_data_t *data = (_ccs_tree_common_data_t *)(tree->data);
	CCS_VALIDATE(_ccs_tree_samples(
		data, distribution, rng, num_indices, indices));
	return CCS_SUCCESS;
}
