#include "cconfigspace_internal.h"
#include "tuner_internal.h"
#include "evaluation_internal.h"

#include "utarray.h"

struct _ccs_random_tuner_data_s {
	_ccs_tuner_common_data_t common_data;
	UT_array                *history;
	UT_array                *optimums;
	UT_array                *old_optimums;
};
typedef struct _ccs_random_tuner_data_s _ccs_random_tuner_data_t;

static ccs_error_t
_ccs_tuner_random_del(ccs_object_t o)
{
	_ccs_random_tuner_data_t *d =
		(_ccs_random_tuner_data_t *)((ccs_tuner_t)o)->data;
	ccs_release_object(d->common_data.configuration_space);
	ccs_release_object(d->common_data.objective_space);
	ccs_evaluation_t *e = NULL;
	while ((e = (ccs_evaluation_t *)utarray_next(d->history, e)))
		ccs_release_object(*e);
	utarray_free(d->history);
	utarray_free(d->optimums);
	utarray_free(d->old_optimums);
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_size_ccs_random_tuner_data(
	_ccs_random_tuner_data_t        *data,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	ccs_evaluation_t *e = NULL;
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_tuner_common_data(
		&data->common_data, cum_size, opts));
	*cum_size += _ccs_serialize_bin_size_size(utarray_len(data->history));
	*cum_size += _ccs_serialize_bin_size_size(utarray_len(data->optimums));
	while ((e = (ccs_evaluation_t *)utarray_next(data->history, e)))
		CCS_VALIDATE((*e)->obj.ops->serialize_size(
			*e, CCS_SERIALIZE_FORMAT_BINARY, cum_size, opts));
	e = NULL;
	while ((e = (ccs_evaluation_t *)utarray_next(data->optimums, e)))
		*cum_size += _ccs_serialize_bin_size_ccs_object(*e);
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_random_tuner_data(
	_ccs_random_tuner_data_t        *data,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	ccs_evaluation_t *e = NULL;
	CCS_VALIDATE(_ccs_serialize_bin_ccs_tuner_common_data(
		&data->common_data, buffer_size, buffer, opts));
	CCS_VALIDATE(_ccs_serialize_bin_size(
		utarray_len(data->history), buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_size(
		utarray_len(data->optimums), buffer_size, buffer));
	while ((e = (ccs_evaluation_t *)utarray_next(data->history, e)))
		CCS_VALIDATE((*e)->obj.ops->serialize(
			*e, CCS_SERIALIZE_FORMAT_BINARY, buffer_size, buffer,
			opts));
	e = NULL;
	while ((e = (ccs_evaluation_t *)utarray_next(data->optimums, e)))
		CCS_VALIDATE(
			_ccs_serialize_bin_ccs_object(*e, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_size_ccs_random_tuner(
	ccs_tuner_t                      tuner,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	_ccs_random_tuner_data_t *data =
		(_ccs_random_tuner_data_t *)(tuner->data);
	*cum_size += _ccs_serialize_bin_size_ccs_object_internal(
		(_ccs_object_internal_t *)tuner);
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_random_tuner_data(
		data, cum_size, opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_random_tuner(
	ccs_tuner_t                      tuner,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	_ccs_random_tuner_data_t *data =
		(_ccs_random_tuner_data_t *)(tuner->data);
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)tuner, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_random_tuner_data(
		data, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tuner_random_serialize_size(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_random_tuner(
			(ccs_tuner_t)object, cum_size, opts));
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
_ccs_tuner_random_serialize(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_random_tuner(
			(ccs_tuner_t)object, buffer_size, buffer, opts));
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

static ccs_error_t
_ccs_tuner_random_ask(
	ccs_tuner_t          tuner,
	size_t               num_configurations,
	ccs_configuration_t *configurations,
	size_t              *num_configurations_ret)
{
	_ccs_random_tuner_data_t *d = (_ccs_random_tuner_data_t *)tuner->data;
	if (!configurations) {
		*num_configurations_ret = 1;
		return CCS_SUCCESS;
	}
	CCS_VALIDATE(ccs_configuration_space_samples(
		d->common_data.configuration_space, num_configurations,
		configurations));
	if (num_configurations_ret)
		*num_configurations_ret = num_configurations;
	return CCS_SUCCESS;
}
#undef utarray_oom
#define utarray_oom()                                                          \
	{                                                                      \
		ccs_release_object(evaluations[i]);                            \
		CCS_RAISE(                                                     \
			CCS_OUT_OF_MEMORY, "Out of memory to allocate array"); \
	}
static ccs_error_t
_ccs_tuner_random_tell(
	ccs_tuner_t       tuner,
	size_t            num_evaluations,
	ccs_evaluation_t *evaluations)
{
	_ccs_random_tuner_data_t *d = (_ccs_random_tuner_data_t *)tuner->data;
	UT_array                 *history = d->history;
	ccs_error_t               err;
	for (size_t i = 0; i < num_evaluations; i++) {
		ccs_evaluation_result_t result;
		CCS_VALIDATE(
			ccs_evaluation_get_result(evaluations[i], &result));
		if (!result) {
			int       discard = 0;
			UT_array *tmp;
			ccs_retain_object(evaluations[i]);
			utarray_push_back(history, evaluations + i);
			tmp             = d->old_optimums;
			d->old_optimums = d->optimums;
			d->optimums     = tmp;
			utarray_clear(d->optimums);
			ccs_evaluation_t *eval = NULL;
#undef utarray_oom
#define utarray_oom()                                                          \
	{                                                                      \
		d->optimums = d->old_optimums;                                 \
		CCS_RAISE(                                                     \
			CCS_OUT_OF_MEMORY, "Out of memory to allocate array"); \
	}
			while ((eval = (ccs_evaluation_t *)utarray_next(
					d->old_optimums, eval))) {
				if (!discard) {
					ccs_comparison_t cmp;
					err = ccs_evaluation_compare(
						evaluations[i], *eval, &cmp);
					if (err)
						discard = 1;
					else
						switch (cmp) {
						case CCS_EQUIVALENT:
						case CCS_WORSE:
							discard = 1;
							utarray_push_back(
								d->optimums,
								eval);
							break;
						case CCS_BETTER:
							break;
						case CCS_NOT_COMPARABLE:
						default:
							utarray_push_back(
								d->optimums,
								eval);
							break;
						}
				} else {
					utarray_push_back(d->optimums, eval);
				}
			}
			if (!discard)
				utarray_push_back(d->optimums, evaluations + i);
		}
	}
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tuner_random_get_optimums(
	ccs_tuner_t       tuner,
	size_t            num_evaluations,
	ccs_evaluation_t *evaluations,
	size_t           *num_evaluations_ret)
{
	_ccs_random_tuner_data_t *d = (_ccs_random_tuner_data_t *)tuner->data;
	size_t                    count = utarray_len(d->optimums);
	if (evaluations) {
		CCS_REFUTE(num_evaluations < count, CCS_INVALID_VALUE);
		ccs_evaluation_t *eval  = NULL;
		size_t            index = 0;
		while ((eval = (ccs_evaluation_t *)utarray_next(
				d->optimums, eval)))
			evaluations[index++] = *eval;
		for (size_t i = count; i < num_evaluations; i++)
			evaluations[i] = NULL;
	}
	if (num_evaluations_ret)
		*num_evaluations_ret = count;
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tuner_random_get_history(
	ccs_tuner_t       tuner,
	size_t            num_evaluations,
	ccs_evaluation_t *evaluations,
	size_t           *num_evaluations_ret)
{
	_ccs_random_tuner_data_t *d = (_ccs_random_tuner_data_t *)tuner->data;
	size_t                    count = utarray_len(d->history);
	if (evaluations) {
		CCS_REFUTE(num_evaluations < count, CCS_INVALID_VALUE);
		ccs_evaluation_t *eval  = NULL;
		size_t            index = 0;
		while ((eval = (ccs_evaluation_t *)utarray_next(
				d->history, eval)))
			evaluations[index++] = *eval;
		for (size_t i = count; i < num_evaluations; i++)
			evaluations[i] = NULL;
	}
	if (num_evaluations_ret)
		*num_evaluations_ret = count;
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_tuner_random_suggest(ccs_tuner_t tuner, ccs_configuration_t *configuration)
{
	_ccs_random_tuner_data_t *d = (_ccs_random_tuner_data_t *)tuner->data;
	size_t                    count = utarray_len(d->optimums);
	if (count > 0) {
		ccs_rng_t         rng;
		unsigned long int indx;
		CCS_VALIDATE(ccs_configuration_space_get_rng(
			d->common_data.configuration_space, &rng));
		CCS_VALIDATE(ccs_rng_get(rng, &indx));
		indx = indx % count;
		ccs_evaluation_t *eval =
			(ccs_evaluation_t *)utarray_eltptr(d->optimums, indx);
		CCS_VALIDATE(
			ccs_evaluation_get_configuration(*eval, configuration));
		CCS_VALIDATE(ccs_retain_object(*configuration));
	} else
		CCS_VALIDATE(
			_ccs_tuner_random_ask(tuner, 1, configuration, NULL));
	return CCS_SUCCESS;
}

static _ccs_tuner_ops_t _ccs_tuner_random_ops = {
	{&_ccs_tuner_random_del, &_ccs_tuner_random_serialize_size,
	 &_ccs_tuner_random_serialize},
	&_ccs_tuner_random_ask,
	&_ccs_tuner_random_tell,
	&_ccs_tuner_random_get_optimums,
	&_ccs_tuner_random_get_history,
	&_ccs_tuner_random_suggest};

static const UT_icd _evaluation_icd = {
	sizeof(ccs_evaluation_t),
	NULL,
	NULL,
	NULL,
};

#undef utarray_oom
#define utarray_oom()                                                          \
	{                                                                      \
		CCS_RAISE_ERR_GOTO(                                            \
			err, CCS_OUT_OF_MEMORY, arrays,                        \
			"Out of memory to allocate array");                    \
	}
ccs_error_t
ccs_create_random_tuner(
	const char               *name,
	ccs_configuration_space_t configuration_space,
	ccs_objective_space_t     objective_space,
	ccs_tuner_t              *tuner_ret)
{
	CCS_CHECK_PTR(name);
	CCS_CHECK_OBJ(configuration_space, CCS_CONFIGURATION_SPACE);
	CCS_CHECK_OBJ(objective_space, CCS_OBJECTIVE_SPACE);
	CCS_CHECK_PTR(tuner_ret);

	uintptr_t mem = (uintptr_t)calloc(
		1, sizeof(struct _ccs_tuner_s) +
			   sizeof(struct _ccs_random_tuner_data_s) +
			   strlen(name) + 1);
	CCS_REFUTE(!mem, CCS_OUT_OF_MEMORY);
	ccs_tuner_t               tun;
	_ccs_random_tuner_data_t *data;
	ccs_error_t               err;
	CCS_VALIDATE_ERR_GOTO(
		err, ccs_retain_object(configuration_space), errmemory);
	CCS_VALIDATE_ERR_GOTO(
		err, ccs_retain_object(objective_space), errconfigs);
	tun = (ccs_tuner_t)mem;
	_ccs_object_init(
		&(tun->obj), CCS_TUNER,
		(_ccs_object_ops_t *)&_ccs_tuner_random_ops);
	tun->data =
		(struct _ccs_tuner_data_s *)(mem + sizeof(struct _ccs_tuner_s));
	data                   = (_ccs_random_tuner_data_t *)tun->data;
	data->common_data.type = CCS_TUNER_RANDOM;
	data->common_data.name =
		(const char
			 *)(mem + sizeof(struct _ccs_tuner_s) + sizeof(struct _ccs_random_tuner_data_s));
	data->common_data.configuration_space = configuration_space;
	data->common_data.objective_space     = objective_space;
	utarray_new(data->history, &_evaluation_icd);
	utarray_new(data->optimums, &_evaluation_icd);
	utarray_new(data->old_optimums, &_evaluation_icd);
	strcpy((char *)data->common_data.name, name);
	*tuner_ret = tun;
	return CCS_SUCCESS;

arrays:
	if (data->history)
		utarray_free(data->history);
	if (data->optimums)
		utarray_free(data->optimums);
	if (data->old_optimums)
		utarray_free(data->old_optimums);
	ccs_release_object(objective_space);
errconfigs:
	ccs_release_object(configuration_space);
errmemory:
	free((void *)mem);
	return err;
}
