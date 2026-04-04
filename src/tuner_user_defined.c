#include "cconfigspace_internal.h"
#include "cconfigspace_json.h"
#include "tuner_internal.h"
#include "evaluation_internal.h"
#include "search_space_internal.h"
#include "string.h"

struct _ccs_user_defined_tuner_data_s {
	_ccs_tuner_common_data_t        common_data;
	ccs_user_defined_tuner_vector_t vector;
	void                           *tuner_data;
};
typedef struct _ccs_user_defined_tuner_data_s _ccs_user_defined_tuner_data_t;

static ccs_result_t
_ccs_tuner_user_defined_del(ccs_object_t o)
{
	_ccs_user_defined_tuner_data_t *d =
		(_ccs_user_defined_tuner_data_t *)((ccs_tuner_t)o)->data;
	ccs_result_t err;
	err = d->vector.del((ccs_tuner_t)o);
	ccs_release_object(d->common_data.search_space);
	ccs_release_object(d->common_data.objective_space);
	if (d->common_data.feature_space)
		ccs_release_object(d->common_data.feature_space);
	return err;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_user_defined_tuner(
	ccs_tuner_t                      tuner,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	ccs_result_t                    res = CCS_RESULT_SUCCESS;
	_ccs_user_defined_tuner_data_t *data =
		(_ccs_user_defined_tuner_data_t *)(tuner->data);
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tuner_common_data(
		&data->common_data, cum_size, opts));
	size_t            history_size = 0;
	size_t            num_optima   = 0;
	size_t            state_size   = 0;
	ccs_evaluation_t *history      = NULL;
	ccs_evaluation_t *optima       = NULL;
	CCS_VALIDATE(
		data->vector.get_history(tuner, NULL, 0, NULL, &history_size));
	CCS_VALIDATE(
		data->vector.get_optima(tuner, NULL, 0, NULL, &num_optima));
	*cum_size += _ccs_serialize_bin_size_size(history_size);
	*cum_size += _ccs_serialize_bin_size_size(num_optima);
	if (0 != history_size + num_optima) {
		history = (ccs_evaluation_t *)calloc(
			history_size + num_optima, sizeof(ccs_evaluation_t));
		CCS_REFUTE(!history, CCS_RESULT_ERROR_OUT_OF_MEMORY);
		optima = history + history_size;
		if (history_size) {
			CCS_VALIDATE_ERR_GOTO(
				res,
				data->vector.get_history(
					tuner, NULL, history_size, history,
					NULL),
				end);
			for (size_t i = 0; i < history_size; i++)
				CCS_VALIDATE_ERR_GOTO(
					res,
					_ccs_object_serialize_size_with_opts(
						history[i],
						CCS_SERIALIZE_FORMAT_BINARY,
						cum_size, opts),
					end);
		}
		if (num_optima) {
			CCS_VALIDATE_ERR_GOTO(
				res,
				data->vector.get_optima(
					tuner, NULL, num_optima, optima, NULL),
				end);
			for (size_t i = 0; i < num_optima; i++)
				*cum_size += _ccs_serialize_bin_size_ccs_object(
					optima[i]);
		}
	}
	if (data->vector.serialize_user_state)
		CCS_VALIDATE_ERR_GOTO(
			res,
			data->vector.serialize_user_state(
				tuner, 0, NULL, &state_size),
			end);
	*cum_size += _ccs_serialize_bin_size_size(state_size);
	*cum_size += state_size;
end:
	if (history)
		free(history);
	return res;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_user_defined_tuner(
	ccs_tuner_t                      tuner,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	ccs_result_t                    res = CCS_RESULT_SUCCESS;
	_ccs_user_defined_tuner_data_t *data =
		(_ccs_user_defined_tuner_data_t *)(tuner->data);
	CCS_VALIDATE(_ccs_serialize_bin_ccs_tuner_common_data(
		&data->common_data, buffer_size, buffer, opts));
	size_t            history_size = 0;
	size_t            num_optima   = 0;
	size_t            state_size   = 0;
	ccs_evaluation_t *history      = NULL;
	ccs_evaluation_t *optima       = NULL;
	CCS_VALIDATE(
		data->vector.get_history(tuner, NULL, 0, NULL, &history_size));
	CCS_VALIDATE(
		data->vector.get_optima(tuner, NULL, 0, NULL, &num_optima));
	CCS_VALIDATE(
		_ccs_serialize_bin_size(history_size, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_size(num_optima, buffer_size, buffer));
	if (0 != history_size + num_optima) {
		history = (ccs_evaluation_t *)calloc(
			history_size + num_optima, sizeof(ccs_evaluation_t));
		CCS_REFUTE(!history, CCS_RESULT_ERROR_OUT_OF_MEMORY);
		optima = history + history_size;
		if (history_size) {
			CCS_VALIDATE_ERR_GOTO(
				res,
				data->vector.get_history(
					tuner, NULL, history_size, history,
					NULL),
				end);
			for (size_t i = 0; i < history_size; i++)
				CCS_VALIDATE_ERR_GOTO(
					res,
					_ccs_object_serialize_with_opts(
						history[i],
						CCS_SERIALIZE_FORMAT_BINARY,
						buffer_size, buffer, opts),
					end);
		}
		if (num_optima) {
			CCS_VALIDATE_ERR_GOTO(
				res,
				data->vector.get_optima(
					tuner, NULL, num_optima, optima, NULL),
				end);
			for (size_t i = 0; i < num_optima; i++)
				CCS_VALIDATE_ERR_GOTO(
					res,
					_ccs_serialize_bin_ccs_object(
						optima[i], buffer_size, buffer),
					end);
		}
	}
	if (data->vector.serialize_user_state)
		CCS_VALIDATE_ERR_GOTO(
			res,
			data->vector.serialize_user_state(
				tuner, 0, NULL, &state_size),
			end);
	CCS_VALIDATE_ERR_GOTO(
		res, _ccs_serialize_bin_size(state_size, buffer_size, buffer),
		end);
	if (state_size) {
		CCS_REFUTE_ERR_GOTO(
			res, *buffer_size < state_size,
			CCS_RESULT_ERROR_NOT_ENOUGH_DATA, end);
		CCS_VALIDATE_ERR_GOTO(
			res,
			data->vector.serialize_user_state(
				tuner, state_size, *buffer, NULL),
			end);
		*buffer_size -= state_size;
		*buffer += state_size;
	}
end:
	if (history)
		free(history);
	return res;
}

static inline ccs_result_t
_ccs_serialize_json_ccs_user_defined_tuner(
	ccs_tuner_t                      tuner,
	cJSON                           *json,
	_ccs_object_serialize_options_t *opts)
{
	ccs_result_t                    res = CCS_RESULT_SUCCESS;
	_ccs_user_defined_tuner_data_t *data =
		(_ccs_user_defined_tuner_data_t *)(tuner->data);
	size_t            history_size = 0;
	size_t            num_optima   = 0;
	size_t            state_size   = 0;
	ccs_evaluation_t *history      = NULL;
	ccs_evaluation_t *optima       = NULL;

	/* tuner type + name */
	CCS_VALIDATE(_ccs_json_add_string(json, "tuner_type", "user_defined"));
	CCS_VALIDATE(
		_ccs_json_add_string(json, "name", data->common_data.name));

	/* objective_space (inlined) */
	CCS_VALIDATE(_ccs_json_embed_object(
		json, "objective_space", data->common_data.objective_space,
		opts));

	CCS_VALIDATE(
		data->vector.get_history(tuner, NULL, 0, NULL, &history_size));
	CCS_VALIDATE(
		data->vector.get_optima(tuner, NULL, 0, NULL, &num_optima));
	if (0 != history_size + num_optima) {
		history = (ccs_evaluation_t *)calloc(
			history_size + num_optima, sizeof(ccs_evaluation_t));
		CCS_REFUTE(!history, CCS_RESULT_ERROR_OUT_OF_MEMORY);
		optima = history + history_size;
		if (history_size)
			CCS_VALIDATE_ERR_GOTO(
				res,
				data->vector.get_history(
					tuner, NULL, history_size, history,
					NULL),
				end);
		if (num_optima)
			CCS_VALIDATE_ERR_GOTO(
				res,
				data->vector.get_optima(
					tuner, NULL, num_optima, optima, NULL),
				end);
	}

	/* history (inlined evaluations) */
	{
		cJSON *j_history;
		CCS_VALIDATE_ERR_GOTO(
			res, _ccs_json_add_array(json, "history", &j_history),
			end);
		for (size_t i = 0; i < history_size; i++)
			CCS_VALIDATE_ERR_GOTO(
				res,
				_ccs_json_embed_array_object(
					j_history, history[i], opts),
				end);
	}

	/* optima (handles) */
	{
		cJSON *j_optima;
		CCS_VALIDATE_ERR_GOTO(
			res, _ccs_json_add_array(json, "optima", &j_optima),
			end);
		for (size_t i = 0; i < num_optima; i++) {
			char hex[sizeof(ccs_object_t) * 2 + 1];
			_ccs_json_hex_encode_buf(
				&optima[i], sizeof(ccs_object_t), hex);
			cJSON *h = NULL;
			CCS_VALIDATE_ERR_GOTO(
				res, _ccs_json_create_string(hex, &h), end);
			CCS_REFUTE_ERR_GOTO(
				res, !cJSON_AddItemToArray(j_optima, h),
				CCS_RESULT_ERROR_OUT_OF_MEMORY, end);
		}
	}

	/* user state */
	if (data->vector.serialize_user_state) {
		CCS_VALIDATE_ERR_GOTO(
			res,
			data->vector.serialize_user_state(
				tuner, 0, NULL, &state_size),
			end);
		if (state_size) {
			char *state_buf = (char *)malloc(state_size);
			CCS_REFUTE_ERR_GOTO(
				res, !state_buf, CCS_RESULT_ERROR_OUT_OF_MEMORY,
				end);
			ccs_result_t err = data->vector.serialize_user_state(
				tuner, state_size, state_buf, NULL);
			if (err != CCS_RESULT_SUCCESS) {
				free(state_buf);
				res = err;
				goto end;
			}
			char *hex = _ccs_json_hex_encode(state_buf, state_size);
			free(state_buf);
			CCS_REFUTE_ERR_GOTO(
				res, !hex, CCS_RESULT_ERROR_OUT_OF_MEMORY, end);
			ccs_result_t add_err =
				_ccs_json_add_string(json, "user_state", hex);
			free(hex);
			CCS_VALIDATE_ERR_GOTO(res, add_err, end);
		}
	}

end:
	if (history)
		free(history);
	return res;
}

static ccs_result_t
_ccs_tuner_user_defined_serialize_size(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_user_defined_tuner(
			(ccs_tuner_t)object, cum_size, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_tuner_user_defined_serialize(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_user_defined_tuner(
			(ccs_tuner_t)object, buffer_size, buffer, opts));
		break;
	case CCS_SERIALIZE_FORMAT_JSON:
		CCS_VALIDATE(_ccs_serialize_json_ccs_user_defined_tuner(
			(ccs_tuner_t)object, *(cJSON **)buffer, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_tuner_user_defined_ask(
	ccs_tuner_t                 tuner,
	ccs_features_t              features,
	size_t                      num_configurations,
	ccs_search_configuration_t *configurations,
	size_t                     *num_configurations_ret)
{
	_ccs_user_defined_tuner_data_t *d =
		(_ccs_user_defined_tuner_data_t *)tuner->data;
	CCS_VALIDATE(d->vector.ask(
		tuner, features, num_configurations, configurations,
		num_configurations_ret));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_tuner_user_defined_tell(
	ccs_tuner_t       tuner,
	size_t            num_evaluations,
	ccs_evaluation_t *evaluations)
{
	_ccs_user_defined_tuner_data_t *d =
		(_ccs_user_defined_tuner_data_t *)tuner->data;
	CCS_VALIDATE(d->vector.tell(tuner, num_evaluations, evaluations));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_tuner_user_defined_get_optima(
	ccs_tuner_t       tuner,
	ccs_features_t    features,
	size_t            num_evaluations,
	ccs_evaluation_t *evaluations,
	size_t           *num_evaluations_ret)
{
	_ccs_user_defined_tuner_data_t *d =
		(_ccs_user_defined_tuner_data_t *)tuner->data;
	CCS_VALIDATE(d->vector.get_optima(
		tuner, features, num_evaluations, evaluations,
		num_evaluations_ret));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_tuner_user_defined_get_history(
	ccs_tuner_t       tuner,
	ccs_features_t    features,
	size_t            num_evaluations,
	ccs_evaluation_t *evaluations,
	size_t           *num_evaluations_ret)
{
	_ccs_user_defined_tuner_data_t *d =
		(_ccs_user_defined_tuner_data_t *)tuner->data;
	CCS_VALIDATE(d->vector.get_history(
		tuner, features, num_evaluations, evaluations,
		num_evaluations_ret));
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
_ccs_tuner_user_defined_suggest(
	ccs_tuner_t                 tuner,
	ccs_features_t              features,
	ccs_search_configuration_t *configuration_ret)
{
	_ccs_user_defined_tuner_data_t *d =
		(_ccs_user_defined_tuner_data_t *)tuner->data;
	CCS_REFUTE(!d->vector.suggest, CCS_RESULT_ERROR_UNSUPPORTED_OPERATION);
	CCS_VALIDATE(d->vector.suggest(tuner, features, configuration_ret));
	return CCS_RESULT_SUCCESS;
}

static _ccs_tuner_ops_t _ccs_tuner_user_defined_ops = {
	{&_ccs_tuner_user_defined_del, &_ccs_tuner_user_defined_serialize_size,
	 &_ccs_tuner_user_defined_serialize},
	&_ccs_tuner_user_defined_ask,
	&_ccs_tuner_user_defined_tell,
	&_ccs_tuner_user_defined_get_optima,
	&_ccs_tuner_user_defined_get_history,
	&_ccs_tuner_user_defined_suggest};

ccs_result_t
ccs_create_user_defined_tuner(
	const char                      *name,
	ccs_objective_space_t            objective_space,
	ccs_user_defined_tuner_vector_t *vector,
	void                            *tuner_data,
	ccs_tuner_t                     *tuner_ret)
{
	ccs_search_space_t  search_space;
	ccs_feature_space_t feature_space;
	CCS_CHECK_PTR(name);
	CCS_CHECK_OBJ(objective_space, CCS_OBJECT_TYPE_OBJECTIVE_SPACE);
	CCS_VALIDATE(ccs_objective_space_get_search_space(
		objective_space, &search_space));
	CCS_VALIDATE(_ccs_search_space_get_feature_space(
		search_space, &feature_space));
	CCS_CHECK_PTR(tuner_ret);
	CCS_CHECK_PTR(vector);
	CCS_CHECK_PTR(vector->del);
	CCS_CHECK_PTR(vector->ask);
	CCS_CHECK_PTR(vector->tell);
	CCS_CHECK_PTR(vector->get_optima);
	CCS_CHECK_PTR(vector->get_history);

	size_t name_len = strlen(name) + 1;
	size_t _sz;
	CCS_REFUTE(
		CCS_ALLOC_SIZE(
			&_sz, CCS_ALLOC_SIZE_TYPE(struct _ccs_tuner_s),
			CCS_ALLOC_SIZE_TYPE(
				struct _ccs_user_defined_tuner_data_s),
			CCS_ALLOC_SIZE_ARRAY(name_len, char)),
		CCS_RESULT_ERROR_OUT_OF_MEMORY);
	uintptr_t mem = (uintptr_t)calloc(1, _sz);
	CCS_REFUTE(!mem, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	uintptr_t                       mem_orig = mem;
	ccs_tuner_t                     tun;
	_ccs_user_defined_tuner_data_t *data;
	ccs_result_t                    err;

	CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(search_space), errmem);
	CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(objective_space), errcs);
	if (feature_space)
		CCS_VALIDATE_ERR_GOTO(
			err, ccs_retain_object(feature_space), erros);

	tun = CCS_ALLOC_CARVE_TYPE(mem, struct _ccs_tuner_s);
	_ccs_object_init(
		&(tun->obj), CCS_OBJECT_TYPE_TUNER,
		(_ccs_object_ops_t *)&_ccs_tuner_user_defined_ops);
	data = CCS_ALLOC_CARVE_TYPE(mem, struct _ccs_user_defined_tuner_data_s);
	tun->data              = (struct _ccs_tuner_data_s *)data;
	data->common_data.type = CCS_TUNER_TYPE_USER_DEFINED;
	data->common_data.name = CCS_ALLOC_CARVE_ARRAY(mem, name_len, char);
	data->common_data.search_space    = search_space;
	data->common_data.objective_space = objective_space;
	data->common_data.feature_space   = feature_space;
	data->vector                      = *vector;
	data->tuner_data                  = tuner_data;
	strcpy((char *)data->common_data.name, name);
	*tuner_ret = tun;
	return CCS_RESULT_SUCCESS;
erros:
	ccs_release_object(objective_space);
errcs:
	ccs_release_object(search_space);
errmem:
	free((void *)mem_orig);
	return err;
}

ccs_result_t
ccs_user_defined_tuner_get_tuner_data(ccs_tuner_t tuner, void **tuner_data_ret)
{
	CCS_CHECK_OBJ(tuner, CCS_OBJECT_TYPE_TUNER);
	CCS_CHECK_PTR(tuner_data_ret);
	_ccs_user_defined_tuner_data_t *d =
		(_ccs_user_defined_tuner_data_t *)tuner->data;
	CCS_REFUTE(
		d->common_data.type != CCS_TUNER_TYPE_USER_DEFINED,
		CCS_RESULT_ERROR_INVALID_TUNER);
	*tuner_data_ret = d->tuner_data;
	return CCS_RESULT_SUCCESS;
}
