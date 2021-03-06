#include "cconfigspace_internal.h"
#include "tuner_internal.h"
#include "evaluation_internal.h"
#include "string.h"

struct _ccs_user_defined_tuner_data_s {
	_ccs_tuner_common_data_t          common_data;
	ccs_tuner_t                       selfref;
	ccs_user_defined_tuner_vector_t   vector;
	void                             *tuner_data;
};
typedef struct _ccs_user_defined_tuner_data_s _ccs_user_defined_tuner_data_t;

static ccs_error_t
_ccs_tuner_user_defined_del(ccs_object_t o) {
	_ccs_user_defined_tuner_data_t *d =
		(_ccs_user_defined_tuner_data_t *)((ccs_tuner_t)o)->data;
	ccs_error_t err;
	err = d->vector.del((ccs_tuner_t)o);
	ccs_release_object(d->common_data.configuration_space);
	ccs_release_object(d->common_data.objective_space);
	return err;
}

static inline ccs_error_t
_ccs_serialize_bin_size_ccs_user_defined_tuner(
		ccs_tuner_t                      tuner,
		size_t                          *cum_size,
		_ccs_object_serialize_options_t *opts) {
	ccs_error_t res = CCS_SUCCESS;
	_ccs_user_defined_tuner_data_t *data =
		(_ccs_user_defined_tuner_data_t *)(tuner->data);
	*cum_size += _ccs_serialize_bin_size_ccs_object_internal(
		(_ccs_object_internal_t *)tuner);
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tuner_common_data(
		&data->common_data, cum_size, opts));
	size_t history_size = 0;
	size_t num_optimums = 0;
	size_t state_size = 0;
	ccs_evaluation_t *history = NULL;
	ccs_evaluation_t *optimums = NULL;
	CCS_VALIDATE(data->vector.get_history(tuner, 0, NULL, &history_size));
	CCS_VALIDATE(data->vector.get_optimums(tuner, 0, NULL, &num_optimums));
	*cum_size += _ccs_serialize_bin_size_uint64(history_size);
	*cum_size += _ccs_serialize_bin_size_uint64(num_optimums);
	if (0 != history_size + num_optimums) {
		history = (ccs_evaluation_t *)calloc(sizeof(ccs_evaluation_t), history_size + num_optimums);
		CCS_REFUTE(!history, CCS_OUT_OF_MEMORY);
		optimums = history + history_size;
		if (history_size) {
			CCS_VALIDATE_ERR_GOTO(res, data->vector.get_history(tuner, history_size, history, NULL), end);
			for (size_t i = 0; i < history_size; i++)
				CCS_VALIDATE_ERR_GOTO(res, history[i]->obj.ops->serialize_size(
					history[i], CCS_SERIALIZE_FORMAT_BINARY, cum_size, opts), end);
		}
		if (num_optimums) {
			CCS_VALIDATE_ERR_GOTO(res, data->vector.get_optimums(tuner, num_optimums, optimums, NULL), end);
			for (size_t i = 0; i < num_optimums; i++)
				*cum_size += _ccs_serialize_bin_size_ccs_object(optimums[i]);
		}
	}
	if (data->vector.serialize_user_state)
		CCS_VALIDATE_ERR_GOTO(res, data->vector.serialize_user_state(
			tuner, 0, NULL, &state_size), end);
	*cum_size += _ccs_serialize_bin_size_uint64(state_size);
	*cum_size += state_size;
end:
	if (history)
		free(history);
	return res;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_user_defined_tuner(
		ccs_tuner_t                       tuner,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	ccs_error_t res = CCS_SUCCESS;
	_ccs_user_defined_tuner_data_t *data =
		(_ccs_user_defined_tuner_data_t *)(tuner->data);
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)tuner, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_tuner_common_data(
		&data->common_data, buffer_size, buffer, opts));
	size_t history_size = 0;
	size_t num_optimums = 0;
	size_t state_size = 0;
	ccs_evaluation_t *history = NULL;
	ccs_evaluation_t *optimums = NULL;
	CCS_VALIDATE(data->vector.get_history(tuner, 0, NULL, &history_size));
	CCS_VALIDATE(data->vector.get_optimums(tuner, 0, NULL, &num_optimums));
	CCS_VALIDATE(_ccs_serialize_bin_uint64(
		history_size, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_uint64(
		num_optimums, buffer_size, buffer));
	if (0 != history_size + num_optimums) {
		history = (ccs_evaluation_t *)calloc(sizeof(ccs_evaluation_t), history_size + num_optimums);
		CCS_REFUTE(!history, CCS_OUT_OF_MEMORY);
		optimums = history + history_size;
		if (history_size) {
			CCS_VALIDATE_ERR_GOTO(res, data->vector.get_history(tuner, history_size, history, NULL), end);
			for (size_t i = 0; i < history_size; i++)
				CCS_VALIDATE_ERR_GOTO(res, history[i]->obj.ops->serialize(
					history[i], CCS_SERIALIZE_FORMAT_BINARY, buffer_size, buffer, opts), end);
		}
		if (num_optimums) {
			CCS_VALIDATE_ERR_GOTO(res, data->vector.get_optimums(tuner, num_optimums, optimums, NULL), end);
			for (size_t i = 0; i < num_optimums; i++)
				CCS_VALIDATE_ERR_GOTO(res, _ccs_serialize_bin_ccs_object(
					optimums[i], buffer_size, buffer), end);
		}
	}
	if (data->vector.serialize_user_state)
		CCS_VALIDATE_ERR_GOTO(res, data->vector.serialize_user_state(
			tuner, 0, NULL, &state_size), end);
	CCS_VALIDATE_ERR_GOTO(res, _ccs_serialize_bin_uint64(
		state_size, buffer_size, buffer), end);
	if (state_size) {
		CCS_REFUTE_ERR_GOTO(res, *buffer_size < state_size, CCS_NOT_ENOUGH_DATA, end);
		CCS_VALIDATE_ERR_GOTO(res, data->vector.serialize_user_state(
			tuner, state_size, *buffer, NULL), end);
		*buffer_size -= state_size;
		*buffer += state_size;
	}
end:
	if (history)
		free(history);
	return res;
}

static ccs_error_t
_ccs_tuner_user_defined_serialize_size(
		ccs_object_t                     object,
		ccs_serialize_format_t           format,
		size_t                          *cum_size,
		_ccs_object_serialize_options_t *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_user_defined_tuner(
			(ccs_tuner_t)object, cum_size, opts));
		break;
	default:
		CCS_RAISE(CCS_INVALID_VALUE, "Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data_size(
		object, format, cum_size, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tuner_user_defined_serialize(
		ccs_object_t                      object,
		ccs_serialize_format_t            format,
		size_t                           *buffer_size,
		char                            **buffer,
		_ccs_object_serialize_options_t  *opts) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_user_defined_tuner(
			(ccs_tuner_t)object, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(CCS_INVALID_VALUE, "Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data(
		object, format, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tuner_user_defined_ask(_ccs_tuner_data_t   *data,
                            size_t               num_configurations,
                            ccs_configuration_t *configurations,
                            size_t              *num_configurations_ret) {
	_ccs_user_defined_tuner_data_t *d =
		(_ccs_user_defined_tuner_data_t *)data;
	CCS_VALIDATE(d->vector.ask(d->selfref, num_configurations, configurations, num_configurations_ret));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tuner_user_defined_tell(_ccs_tuner_data_t *data,
                             size_t             num_evaluations,
                             ccs_evaluation_t  *evaluations) {
	_ccs_user_defined_tuner_data_t *d =
		(_ccs_user_defined_tuner_data_t *)data;
	CCS_VALIDATE(d->vector.tell(d->selfref, num_evaluations, evaluations));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tuner_user_defined_get_optimums(_ccs_tuner_data_t *data,
                                     size_t             num_evaluations,
                                     ccs_evaluation_t  *evaluations,
                                     size_t            *num_evaluations_ret) {
	_ccs_user_defined_tuner_data_t *d =
		(_ccs_user_defined_tuner_data_t *)data;
	CCS_VALIDATE(d->vector.get_optimums(d->selfref, num_evaluations, evaluations, num_evaluations_ret));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tuner_user_defined_get_history(_ccs_tuner_data_t *data,
                                    size_t             num_evaluations,
                                    ccs_evaluation_t  *evaluations,
                                    size_t            *num_evaluations_ret) {
	_ccs_user_defined_tuner_data_t *d =
		(_ccs_user_defined_tuner_data_t *)data;
	CCS_VALIDATE(d->vector.get_history(d->selfref, num_evaluations, evaluations, num_evaluations_ret));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tuner_user_defined_suggest(_ccs_tuner_data_t   *data,
                                ccs_configuration_t *configuration_ret) {
	_ccs_user_defined_tuner_data_t *d =
		(_ccs_user_defined_tuner_data_t *)data;
	CCS_REFUTE(!d->vector.suggest, CCS_UNSUPPORTED_OPERATION);
	CCS_VALIDATE(d->vector.suggest(d->selfref, configuration_ret));
	return CCS_SUCCESS;
}

static _ccs_tuner_ops_t _ccs_tuner_user_defined_ops = {
	{ &_ccs_tuner_user_defined_del,
	  &_ccs_tuner_user_defined_serialize_size,
	  &_ccs_tuner_user_defined_serialize },
	&_ccs_tuner_user_defined_ask,
	&_ccs_tuner_user_defined_tell,
	&_ccs_tuner_user_defined_get_optimums,
	&_ccs_tuner_user_defined_get_history,
	&_ccs_tuner_user_defined_suggest
};

ccs_error_t
ccs_create_user_defined_tuner(const char                      *name,
                              ccs_configuration_space_t        configuration_space,
                              ccs_objective_space_t            objective_space,
                              ccs_user_defined_tuner_vector_t *vector,
                              void                            *tuner_data,
                              ccs_tuner_t                     *tuner_ret) {
	CCS_CHECK_PTR(name);
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_OBJ(objective_space, CCS_OBJECTIVE_SPACE);
	CCS_CHECK_PTR(tuner_ret);
	CCS_CHECK_PTR(vector);
	CCS_CHECK_PTR(vector->del);
	CCS_CHECK_PTR(vector->ask);
	CCS_CHECK_PTR(vector->tell);
	CCS_CHECK_PTR(vector->get_optimums);
	CCS_CHECK_PTR(vector->get_history);

	uintptr_t mem = (uintptr_t)calloc(1,
		sizeof(struct _ccs_tuner_s) +
		sizeof(struct _ccs_user_defined_tuner_data_s) +
		strlen(name) + 1);
	CCS_REFUTE(!mem, CCS_OUT_OF_MEMORY);
	ccs_tuner_t tun;
	_ccs_user_defined_tuner_data_t * data;
	ccs_error_t err;
	CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(configuration_space), errmem);
	CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(objective_space), errcs);

	tun = (ccs_tuner_t)mem;
	_ccs_object_init(&(tun->obj), CCS_TUNER, (_ccs_object_ops_t *)&_ccs_tuner_user_defined_ops);
	tun->data = (struct _ccs_tuner_data_s *)(mem + sizeof(struct _ccs_tuner_s));
	data = (_ccs_user_defined_tuner_data_t *)tun->data;
	data->common_data.type = CCS_TUNER_USER_DEFINED;
	data->common_data.name = (const char *)(mem +
		sizeof(struct _ccs_tuner_s) +
		sizeof(struct _ccs_user_defined_tuner_data_s));
	data->common_data.configuration_space = configuration_space;
	data->common_data.objective_space = objective_space;
        data->selfref = tun;
	data->vector = *vector;
	data->tuner_data = tuner_data;
	strcpy((char*)data->common_data.name, name);
	*tuner_ret = tun;
	return CCS_SUCCESS;
errcs:
	ccs_release_object(configuration_space);
errmem:
	free((void *)mem);
	return err;
}

ccs_error_t
ccs_user_defined_tuner_get_tuner_data(ccs_tuner_t   tuner,
                                      void        **tuner_data_ret) {
	CCS_CHECK_OBJ(tuner, CCS_TUNER);
	CCS_CHECK_PTR(tuner_data_ret);
	_ccs_user_defined_tuner_data_t *d =
		(_ccs_user_defined_tuner_data_t *)tuner->data;
        CCS_REFUTE(d->common_data.type != CCS_TUNER_USER_DEFINED, CCS_INVALID_TUNER);
	*tuner_data_ret = d->tuner_data;
	return CCS_SUCCESS;
}

