#include "cconfigspace_internal.h"
#include "tree_space_internal.h"
#include "tree_configuration_internal.h"
#include "tree_internal.h"
#include "utarray.h"

static inline _ccs_tree_space_ops_t *
_ccs_tree_space_get_ops(ccs_tree_space_t tree_space)
{
	return (_ccs_tree_space_ops_t *)tree_space->obj.ops;
}

ccs_result_t
ccs_tree_space_get_type(
	ccs_tree_space_t       tree_space,
	ccs_tree_space_type_t *type_ret)
{
	CCS_CHECK_OBJ(tree_space, CCS_OBJECT_TYPE_TREE_SPACE);
	CCS_CHECK_PTR(type_ret);
	*type_ret = ((_ccs_tree_space_common_data_t *)(tree_space->data))->type;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_tree_space_get_name(ccs_tree_space_t tree_space, const char **name_ret)
{
	CCS_CHECK_OBJ(tree_space, CCS_OBJECT_TYPE_TREE_SPACE);
	CCS_CHECK_PTR(name_ret);
	*name_ret = ((_ccs_tree_space_common_data_t *)(tree_space->data))->name;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_tree_space_set_rng(ccs_tree_space_t tree_space, ccs_rng_t rng)
{
	CCS_CHECK_OBJ(tree_space, CCS_OBJECT_TYPE_TREE_SPACE);
	CCS_CHECK_OBJ(rng, CCS_OBJECT_TYPE_RNG);
	CCS_VALIDATE(ccs_retain_object(rng));
	ccs_rng_t tmp =
		((_ccs_tree_space_common_data_t *)(tree_space->data))->rng;
	((_ccs_tree_space_common_data_t *)(tree_space->data))->rng = rng;
	CCS_VALIDATE(ccs_release_object(tmp));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_tree_space_get_rng(ccs_tree_space_t tree_space, ccs_rng_t *rng_ret)
{
	CCS_CHECK_OBJ(tree_space, CCS_OBJECT_TYPE_TREE_SPACE);
	CCS_CHECK_PTR(rng_ret);
	*rng_ret = ((_ccs_tree_space_common_data_t *)(tree_space->data))->rng;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_tree_space_get_tree(ccs_tree_space_t tree_space, ccs_tree_t *tree_ret)
{
	CCS_CHECK_OBJ(tree_space, CCS_OBJECT_TYPE_TREE_SPACE);
	CCS_CHECK_PTR(tree_ret);
	*tree_ret = ((_ccs_tree_space_common_data_t *)(tree_space->data))->tree;
	return CCS_RESULT_SUCCESS;
}

static UT_icd _size_t_icd = {sizeof(size_t), NULL, NULL, NULL};

ccs_result_t
ccs_tree_space_get_node_at_position(
	ccs_tree_space_t tree_space,
	size_t           position_size,
	const size_t    *position,
	ccs_tree_t      *tree_ret)
{
	CCS_CHECK_OBJ(tree_space, CCS_OBJECT_TYPE_TREE_SPACE);
	CCS_CHECK_ARY(position_size, position);
	CCS_CHECK_PTR(tree_ret);
	_ccs_tree_space_ops_t *ops = _ccs_tree_space_get_ops(tree_space);
	CCS_VALIDATE(ops->get_node_at_position(
		tree_space, position_size, position, tree_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_tree_space_get_values_at_position(
	ccs_tree_space_t tree_space,
	size_t           position_size,
	const size_t    *position,
	size_t           num_values,
	ccs_datum_t     *values)
{
	CCS_CHECK_OBJ(tree_space, CCS_OBJECT_TYPE_TREE_SPACE);
	CCS_CHECK_ARY(position_size, position);
	CCS_CHECK_ARY(num_values, values);
	_ccs_tree_space_ops_t *ops = _ccs_tree_space_get_ops(tree_space);
	CCS_VALIDATE(ops->get_values_at_position(
		tree_space, position_size, position, num_values, values));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_tree_space_check_position(
	ccs_tree_space_t tree_space,
	size_t           position_size,
	const size_t    *position,
	ccs_bool_t      *is_valid_ret)
{
	CCS_CHECK_OBJ(tree_space, CCS_OBJECT_TYPE_TREE_SPACE);
	CCS_CHECK_ARY(position_size, position);
	CCS_CHECK_PTR(is_valid_ret);
	_ccs_tree_space_ops_t *ops = _ccs_tree_space_get_ops(tree_space);
	CCS_VALIDATE(ops->check_position(
		tree_space, position_size, position, is_valid_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_tree_space_check_configuration(
	ccs_tree_space_t         tree_space,
	ccs_tree_configuration_t configuration,
	ccs_bool_t              *is_valid_ret)
{
	CCS_CHECK_OBJ(tree_space, CCS_OBJECT_TYPE_TREE_SPACE);
	CCS_CHECK_OBJ(configuration, CCS_OBJECT_TYPE_TREE_CONFIGURATION);
	CCS_REFUTE(
		configuration->data->tree_space != tree_space,
		CCS_RESULT_ERROR_INVALID_CONFIGURATION);
	CCS_CHECK_PTR(is_valid_ret);
	_ccs_tree_space_ops_t *ops = _ccs_tree_space_get_ops(tree_space);
	CCS_VALIDATE(ops->check_position(
		tree_space, configuration->data->position_size,
		configuration->data->position, is_valid_ret));
	return CCS_RESULT_SUCCESS;
}

#undef utarray_oom
#define utarray_oom()                                                          \
	{                                                                      \
		CCS_RAISE_ERR_GOTO(                                            \
			err, CCS_RESULT_ERROR_OUT_OF_MEMORY, err_arr,          \
			"Out of memory to allocate array");                    \
	}

static inline ccs_result_t
_ccs_tree_space_samples(
	ccs_tree_space_t          tree_space,
	size_t                    num_configurations,
	ccs_tree_configuration_t *configurations)
{
	_ccs_tree_space_common_data_t *data =
		(_ccs_tree_space_common_data_t *)(tree_space->data);
	ccs_rng_t    rng = data->rng;
	ccs_result_t err = CCS_RESULT_SUCCESS;
	UT_array    *arr = NULL;
	utarray_new(arr, &_size_t_icd);
	for (size_t i = 0; i < num_configurations; i++) {
		size_t     index;
		ccs_tree_t tree = data->tree;
		if (tree) {
			CCS_REFUTE_ERR_GOTO(
				err, tree->data->sum_weights == 0.0,
				CCS_RESULT_ERROR_INVALID_DISTRIBUTION, err_arr);
			CCS_VALIDATE_ERR_GOTO(
				err,
				ccs_distribution_samples(
					tree->data->distribution, rng, 1,
					(ccs_numeric_t *)&index),
				err_arr);
			while (index != tree->data->arity) {
				utarray_push_back(arr, &index);
				tree = tree->data->children[index];
				if (!tree)
					break;
				CCS_REFUTE(
					tree->data->sum_weights == 0.0,
					CCS_RESULT_ERROR_INVALID_DISTRIBUTION);
				CCS_VALIDATE_ERR_GOTO(
					err,
					ccs_distribution_samples(
						tree->data->distribution, rng,
						1, (ccs_numeric_t *)&index),
					err_arr);
			}
		}
		CCS_VALIDATE_ERR_GOTO(
			err,
			ccs_create_tree_configuration(
				tree_space, utarray_len(arr),
				(size_t *)utarray_eltptr(arr, 0),
				configurations + i),
			err_arr);
		utarray_clear(arr);
	}
err_arr:
	if (arr)
		utarray_free(arr);
	return err;
}

ccs_result_t
ccs_tree_space_sample(
	ccs_tree_space_t          tree_space,
	ccs_tree_configuration_t *configuration_ret)
{
	CCS_CHECK_OBJ(tree_space, CCS_OBJECT_TYPE_TREE_SPACE);
	CCS_CHECK_PTR(configuration_ret);
	CCS_VALIDATE(_ccs_tree_space_samples(tree_space, 1, configuration_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_tree_space_samples(
	ccs_tree_space_t          tree_space,
	size_t                    num_configurations,
	ccs_tree_configuration_t *configurations)
{
	CCS_CHECK_OBJ(tree_space, CCS_OBJECT_TYPE_TREE_SPACE);
	CCS_CHECK_ARY(num_configurations, configurations);
	if (num_configurations == 0)
		return CCS_RESULT_SUCCESS;
	CCS_VALIDATE(_ccs_tree_space_samples(
		tree_space, num_configurations, configurations));
	return CCS_RESULT_SUCCESS;
}
