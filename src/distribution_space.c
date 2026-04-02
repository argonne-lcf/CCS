#include "cconfigspace_internal.h"
#include "cconfigspace_json.h"
#include "distribution_space_internal.h"
#include "distribution_internal.h"

static ccs_result_t
_ccs_distribution_space_del(ccs_object_t object)
{
	ccs_distribution_space_t distribution_space =
		(ccs_distribution_space_t)object;
	_ccs_distribution_space_del_no_release(distribution_space);
	ccs_release_object(distribution_space->data->configuration_space);
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_distribution_space_data(
	_ccs_distribution_space_data_t  *data,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	size_t                       distribution_count;
	_ccs_distribution_wrapper_t *dw;

	*cum_size +=
		_ccs_serialize_bin_size_ccs_object(data->configuration_space);
	*cum_size += _ccs_serialize_bin_size_size(data->num_parameters);

	DL_COUNT(data->distribution_list, dw, distribution_count);
	*cum_size += _ccs_serialize_bin_size_size(distribution_count);

	dw = NULL;
	DL_FOREACH(data->distribution_list, dw)
	{
		CCS_VALIDATE(_ccs_object_serialize_size_with_opts(
			dw->distribution, CCS_SERIALIZE_FORMAT_BINARY, cum_size,
			opts));
		*cum_size += _ccs_serialize_bin_size_size(dw->dimension);
		for (size_t i = 0; i < dw->dimension; i++)
			*cum_size += _ccs_serialize_bin_size_size(
				dw->parameter_indexes[i]);
	}
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_distribution_space_data(
	_ccs_distribution_space_data_t  *data,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	size_t                       distribution_count;
	_ccs_distribution_wrapper_t *dw;

	CCS_VALIDATE(_ccs_serialize_bin_ccs_object(
		data->configuration_space, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_size(
		data->num_parameters, buffer_size, buffer));

	DL_COUNT(data->distribution_list, dw, distribution_count);
	CCS_VALIDATE(_ccs_serialize_bin_size(
		distribution_count, buffer_size, buffer));

	dw = NULL;
	DL_FOREACH(data->distribution_list, dw)
	{
		CCS_VALIDATE(_ccs_object_serialize_with_opts(
			dw->distribution, CCS_SERIALIZE_FORMAT_BINARY,
			buffer_size, buffer, opts));
		CCS_VALIDATE(_ccs_serialize_bin_size(
			dw->dimension, buffer_size, buffer));
		for (size_t i = 0; i < dw->dimension; i++)
			CCS_VALIDATE(_ccs_serialize_bin_size(
				dw->parameter_indexes[i], buffer_size, buffer));
	}

	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_distribution_space(
	ccs_distribution_space_t         distribution_space,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	_ccs_distribution_space_data_t *data =
		(_ccs_distribution_space_data_t *)(distribution_space->data);
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_distribution_space_data(
		data, cum_size, opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_distribution_space(
	ccs_distribution_space_t         distribution_space,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	_ccs_distribution_space_data_t *data =
		(_ccs_distribution_space_data_t *)(distribution_space->data);
	CCS_VALIDATE(_ccs_serialize_bin_ccs_distribution_space_data(
		data, buffer_size, buffer, opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_json_ccs_distribution_space(
	ccs_distribution_space_t         distribution_space,
	cJSON                           *json,
	_ccs_object_serialize_options_t *opts)
{
	_ccs_distribution_space_data_t *data = distribution_space->data;
	_ccs_distribution_wrapper_t    *dw;
	char                            hex[sizeof(ccs_object_t) * 2 + 1];
	size_t                          dummy = 0;

	_ccs_json_hex_encode_buf(
		&data->configuration_space, sizeof(ccs_object_t), hex);
	CCS_REFUTE(
		!cJSON_AddStringToObject(json, "configuration_space", hex),
		CCS_RESULT_ERROR_OUT_OF_MEMORY);

	cJSON *distribs = cJSON_AddArrayToObject(json, "distributions");
	CCS_REFUTE(!distribs, CCS_RESULT_ERROR_OUT_OF_MEMORY);

	dw = NULL;
	DL_FOREACH(data->distribution_list, dw)
	{
		cJSON *entry = cJSON_CreateObject();
		CCS_REFUTE(!entry, CCS_RESULT_ERROR_OUT_OF_MEMORY);
		CCS_REFUTE(
			!cJSON_AddItemToArray(distribs, entry),
			CCS_RESULT_ERROR_OUT_OF_MEMORY);

		cJSON *dist_obj =
			cJSON_AddObjectToObject(entry, "distribution");
		CCS_REFUTE(!dist_obj, CCS_RESULT_ERROR_OUT_OF_MEMORY);
		CCS_VALIDATE(_ccs_object_serialize_with_opts(
			dw->distribution, CCS_SERIALIZE_FORMAT_JSON, &dummy,
			(char **)&dist_obj, opts));

		cJSON *indices =
			cJSON_AddArrayToObject(entry, "parameter_indices");
		CCS_REFUTE(!indices, CCS_RESULT_ERROR_OUT_OF_MEMORY);
		for (size_t i = 0; i < dw->dimension; i++)
			CCS_REFUTE(
				!cJSON_AddItemToArray(
					indices,
					cJSON_CreateNumber(
						(double)dw
							->parameter_indexes[i])),
				CCS_RESULT_ERROR_OUT_OF_MEMORY);
	}

	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_distribution_space_serialize_size(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	ccs_result_t err = CCS_RESULT_SUCCESS;
	CCS_OBJ_RDLOCK(object);
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE_ERR_GOTO(
			err,
			_ccs_serialize_bin_size_ccs_distribution_space(
				(ccs_distribution_space_t)object, cum_size,
				opts),
			end);
		break;
	default:
		CCS_RAISE_ERR_GOTO(
			err, CCS_RESULT_ERROR_INVALID_VALUE, end,
			"Unsupported serialization format: %d", format);
	}
end:
	CCS_OBJ_UNLOCK(object);
	return err;
}

static ccs_result_t
_ccs_distribution_space_serialize(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	ccs_result_t err = CCS_RESULT_SUCCESS;
	CCS_OBJ_RDLOCK(object);
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE_ERR_GOTO(
			err,
			_ccs_serialize_bin_ccs_distribution_space(
				(ccs_distribution_space_t)object, buffer_size,
				buffer, opts),
			end);
		break;
	case CCS_SERIALIZE_FORMAT_JSON:
		CCS_VALIDATE_ERR_GOTO(
			err,
			_ccs_serialize_json_ccs_distribution_space(
				(ccs_distribution_space_t)object,
				*(cJSON **)buffer, opts),
			end);
		break;
	default:
		CCS_RAISE_ERR_GOTO(
			err, CCS_RESULT_ERROR_INVALID_VALUE, end,
			"Unsupported serialization format: %d", format);
	}
end:
	CCS_OBJ_UNLOCK(object);
	return err;
}

static _ccs_distribution_space_ops_t _distribution_space_ops = {
	{&_ccs_distribution_space_del, &_ccs_distribution_space_serialize_size,
	 &_ccs_distribution_space_serialize}};

ccs_result_t
ccs_create_distribution_space(
	ccs_configuration_space_t configuration_space,
	ccs_distribution_space_t *distribution_space_ret)
{
	CCS_CHECK_OBJ(configuration_space, CCS_OBJECT_TYPE_CONFIGURATION_SPACE);
	CCS_CHECK_PTR(distribution_space_ret);
	CCS_VALIDATE(ccs_retain_object(configuration_space));
	ccs_result_t err = CCS_RESULT_SUCCESS;
	CCS_VALIDATE_ERR_GOTO(
		err,
		_ccs_create_distribution_space_no_retain(
			configuration_space, &_distribution_space_ops,
			distribution_space_ret),
		err_conf);
	return CCS_RESULT_SUCCESS;
err_conf:
	ccs_release_object(configuration_space);
	return err;
}

ccs_result_t
ccs_distribution_space_get_configuration_space(
	ccs_distribution_space_t   distribution_space,
	ccs_configuration_space_t *configuration_space_ret)
{
	CCS_CHECK_OBJ(distribution_space, CCS_OBJECT_TYPE_DISTRIBUTION_SPACE);
	CCS_CHECK_PTR(configuration_space_ret);
	*configuration_space_ret =
		distribution_space->data->configuration_space;
	return CCS_RESULT_SUCCESS;
}

/* Create a wrapper for the user-specified distribution covering the
 * given parameter indexes.  Each covered index is removed from
 * parameters_without_distrib so that the caller knows which parameters
 * still need a default distribution. On success the distribution is
 * retained; on failure the allocated wrapper is freed. */
static inline ccs_result_t
_ccs_distribution_space_create_user_wrapper(
	ccs_distribution_t            distribution,
	size_t                        dim,
	size_t                       *indexes,
	size_t                       *parameters_without_distrib,
	size_t                       *without_distrib_count,
	_ccs_distribution_wrapper_t **wrapper_ret)
{
	size_t _sz;
	CCS_REFUTE(
		CCS_ALLOC_SIZE(
			&_sz, CCS_ALLOC_SIZE_TYPE(_ccs_distribution_wrapper_t),
			CCS_ALLOC_SIZE_ARRAY(dim, size_t)),
		CCS_RESULT_ERROR_OUT_OF_MEMORY);
	uintptr_t dmem = (uintptr_t)malloc(_sz);
	CCS_REFUTE(!dmem, CCS_RESULT_ERROR_OUT_OF_MEMORY);

	_ccs_distribution_wrapper_t *dwrapper =
		CCS_ALLOC_CARVE_TYPE(dmem, _ccs_distribution_wrapper_t);
	dwrapper->distribution      = distribution;
	dwrapper->dimension         = dim;
	dwrapper->parameter_indexes = CCS_ALLOC_CARVE_ARRAY(dmem, dim, size_t);
	/* Record each parameter index and remove it from the
	 * "without distribution" list. */
	for (size_t i = 0; i < dim; i++) {
		dwrapper->parameter_indexes[i] = indexes[i];
		size_t indx                    = 0;
		for (size_t j = 0; j < *without_distrib_count; j++, indx++)
			if (parameters_without_distrib[j] == indexes[i])
				break;
		for (size_t j = indx + 1; j < *without_distrib_count; j++)
			parameters_without_distrib[j - 1] =
				parameters_without_distrib[j];
		(*without_distrib_count)--;
	}
	ccs_result_t err = CCS_RESULT_SUCCESS;
	CCS_VALIDATE_ERR_GOTO(
		err, ccs_retain_object(distribution), err_wrapper);
	*wrapper_ret = dwrapper;
	return CCS_RESULT_SUCCESS;
err_wrapper:
	free(dwrapper);
	return err;
}

/* Create one wrapper per displaced parameter that no longer has a
 * distribution, assigning each parameter's default distribution.
 * p_dwrappers is written starting at index 0; the caller should pass
 * an offset pointer if earlier entries are already filled.
 * On failure all wrappers created so far are released and freed. */
static inline ccs_result_t
_ccs_distribution_space_create_default_wrappers(
	ccs_parameter_t              *parameters,
	size_t                       *parameters_without_distrib,
	size_t                        without_distrib_count,
	_ccs_distribution_wrapper_t **p_dwrappers,
	size_t                       *count_ret)
{
	size_t                       count    = 0;
	_ccs_distribution_wrapper_t *dwrapper = NULL;
	ccs_result_t                 err      = CCS_RESULT_SUCCESS;
	for (size_t i = 0; i < without_distrib_count; i++) {
		size_t    _sz;
		uintptr_t dmem;
		CCS_REFUTE_ERR_GOTO(
			err,
			CCS_ALLOC_SIZE(
				&_sz,
				CCS_ALLOC_SIZE_TYPE(_ccs_distribution_wrapper_t),
				CCS_ALLOC_SIZE_ARRAY(1, size_t)),
			CCS_RESULT_ERROR_OUT_OF_MEMORY, err_wrappers);
		dmem = (uintptr_t)malloc(_sz);
		CCS_REFUTE_ERR_GOTO(
			err, !dmem, CCS_RESULT_ERROR_OUT_OF_MEMORY,
			err_wrappers);
		dwrapper =
			CCS_ALLOC_CARVE_TYPE(dmem, _ccs_distribution_wrapper_t);
		dwrapper->parameter_indexes =
			CCS_ALLOC_CARVE_ARRAY(dmem, 1, size_t);
		dwrapper->dimension            = 1;
		dwrapper->parameter_indexes[0] = parameters_without_distrib[i];

		CCS_VALIDATE_ERR_GOTO(
			err,
			ccs_parameter_get_default_distribution(
				parameters[parameters_without_distrib[i]],
				&(dwrapper->distribution)),
			err_wrapper);
		p_dwrappers[count++] = dwrapper;
		/* Clear dwrapper so err_wrapper won't double-free on a later
		 * iteration's failure. */
		dwrapper             = NULL;
	}
	*count_ret = count;
	return CCS_RESULT_SUCCESS;
err_wrapper:
	/* Free the current (unfinished) wrapper whose distribution was
	 * not yet retained. */
	free(dwrapper);
err_wrappers:
	/* Release and free all previously completed wrappers. */
	for (size_t i = 0; i < count; i++) {
		ccs_release_object(p_dwrappers[i]->distribution);
		free(p_dwrappers[i]);
	}
	return err;
}

ccs_result_t
ccs_distribution_space_set_distribution(
	ccs_distribution_space_t distribution_space,
	ccs_distribution_t       distribution,
	size_t                  *indexes)
{
	CCS_CHECK_OBJ(distribution_space, CCS_OBJECT_TYPE_DISTRIBUTION_SPACE);
	CCS_CHECK_OBJ(distribution, CCS_OBJECT_TYPE_DISTRIBUTION);
	CCS_CHECK_PTR(indexes);

	_ccs_distribution_wrapper_t  **p_dwrappers_to_del;
	_ccs_distribution_wrapper_t  **p_dwrappers_to_add;
	_ccs_parameter_distribution_t *pdists;
	_ccs_parameter_distribution_t *pdist;
	ccs_result_t                   err;
	ccs_parameter_t               *parameters;
	size_t                         num_parameters;
	size_t                         dim;
	uintptr_t                      mem;
	uintptr_t                      mem_orig;
	size_t                        *parameters_without_distrib;
	size_t                         to_add_count          = 0;
	size_t                         to_del_count          = 0;
	size_t                         without_distrib_count = 0;
	size_t                         default_count         = 0;

	CCS_VALIDATE(ccs_distribution_get_dimension(distribution, &dim));
	parameters =
		distribution_space->data->configuration_space->data->parameters;
	pdists         = distribution_space->data->parameter_distributions;
	num_parameters = distribution_space->data->num_parameters;

	for (size_t i = 0; i < dim; i++) {
		CCS_REFUTE(
			indexes[i] >= num_parameters,
			CCS_RESULT_ERROR_INVALID_VALUE);
		// Check duplicate entries
		for (size_t j = i + 1; j < dim; j++)
			CCS_REFUTE(
				indexes[i] == indexes[j],
				CCS_RESULT_ERROR_INVALID_VALUE);
	}

	{
		size_t _sz;
		CCS_REFUTE(
			CCS_ALLOC_SIZE(
				&_sz,
				CCS_ALLOC_SIZE_ARRAY(
					num_parameters,
					_ccs_distribution_wrapper_t *),
				CCS_ALLOC_SIZE_ARRAY(
					num_parameters,
					_ccs_distribution_wrapper_t *),
				CCS_ALLOC_SIZE_ARRAY(num_parameters, size_t)),
			CCS_RESULT_ERROR_OUT_OF_MEMORY);
		mem = (uintptr_t)malloc(_sz);
		CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	}

	CCS_OBJ_WRLOCK(distribution_space);
	mem_orig           = mem;
	p_dwrappers_to_del = CCS_ALLOC_CARVE_ARRAY(
		mem, num_parameters, _ccs_distribution_wrapper_t *);
	p_dwrappers_to_add = CCS_ALLOC_CARVE_ARRAY(
		mem, num_parameters, _ccs_distribution_wrapper_t *);
	parameters_without_distrib =
		CCS_ALLOC_CARVE_ARRAY(mem, num_parameters, size_t);

	/* Collect the unique wrappers currently covering the target
	 * parameter indexes — these will be removed. */
	for (size_t i = 0; i < dim; i++) {
		int add = 1;
		pdist   = pdists + indexes[i];
		for (size_t j = 0; j < to_del_count; j++)
			if (p_dwrappers_to_del[j] == pdist->distribution) {
				add = 0;
				break;
			}
		if (add)
			p_dwrappers_to_del[to_del_count++] =
				pdist->distribution;
	}
	/* Build the list of all parameters that will lose their
	 * distribution when the old wrappers are removed. */
	for (size_t i = 0; i < to_del_count; i++) {
		for (size_t j = 0; j < p_dwrappers_to_del[i]->dimension; j++) {
			parameters_without_distrib[without_distrib_count++] =
				p_dwrappers_to_del[i]->parameter_indexes[j];
		}
	}

	/* Create the wrapper for the user-supplied distribution.  This
	 * also removes covered indexes from parameters_without_distrib. */
	CCS_VALIDATE_ERR_GOTO(
		err,
		_ccs_distribution_space_create_user_wrapper(
			distribution, dim, indexes, parameters_without_distrib,
			&without_distrib_count, &p_dwrappers_to_add[0]),
		memory);
	to_add_count = 1;

	/* Create default-distribution wrappers for any parameters that
	 * are still uncovered after the user wrapper claimed its indexes. */
	CCS_VALIDATE_ERR_GOTO(
		err,
		_ccs_distribution_space_create_default_wrappers(
			parameters, parameters_without_distrib,
			without_distrib_count,
			p_dwrappers_to_add + to_add_count, &default_count),
		err_user_wrapper);
	to_add_count += default_count;

	/* Commit: remove old wrappers and insert new ones. */
	for (size_t i = 0; i < to_del_count; i++) {
		DL_DELETE(
			distribution_space->data->distribution_list,
			p_dwrappers_to_del[i]);
		ccs_release_object(p_dwrappers_to_del[i]->distribution);
		free(p_dwrappers_to_del[i]);
	}
	for (size_t i = 0; i < to_add_count; i++) {
		DL_APPEND(
			distribution_space->data->distribution_list,
			p_dwrappers_to_add[i]);
		for (size_t j = 0; j < p_dwrappers_to_add[i]->dimension; j++) {
			pdist = pdists +
				p_dwrappers_to_add[i]->parameter_indexes[j];
			pdist->distribution_index = j;
			pdist->distribution       = p_dwrappers_to_add[i];
		}
	}

	free((void *)mem_orig);
	CCS_OBJ_UNLOCK(distribution_space);
	return CCS_RESULT_SUCCESS;
err_user_wrapper:
	/* Default wrapper creation failed — clean up the user wrapper. */
	ccs_release_object(p_dwrappers_to_add[0]->distribution);
	free(p_dwrappers_to_add[0]);
memory:
	free((void *)mem_orig);
	CCS_OBJ_UNLOCK(distribution_space);
	return err;
}

extern ccs_result_t
ccs_distribution_space_get_parameter_distribution(
	ccs_distribution_space_t distribution_space,
	size_t                   index,
	ccs_distribution_t      *distribution_ret,
	size_t                  *index_ret)
{
	ccs_result_t err = CCS_RESULT_SUCCESS;
	CCS_CHECK_OBJ(distribution_space, CCS_OBJECT_TYPE_DISTRIBUTION_SPACE);
	CCS_CHECK_PTR(distribution_ret);
	CCS_CHECK_PTR(index_ret);
	CCS_REFUTE(
		index >= distribution_space->data->num_parameters,
		CCS_RESULT_ERROR_OUT_OF_BOUNDS);

	CCS_OBJ_RDLOCK(distribution_space);
	_ccs_parameter_distribution_t *pdist =
		distribution_space->data->parameter_distributions + index;
	*distribution_ret = pdist->distribution->distribution;
	*index_ret        = pdist->distribution_index;
	CCS_OBJ_UNLOCK(distribution_space);
	return err;
}
