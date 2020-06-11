#include "cconfigspace_internal.h"
#include "configuration_internal.h"
#include "datum_hash.h"
#include <string.h>

static inline _ccs_configuration_ops_t *
ccs_configuration_get_ops(ccs_configuration_t configuration) {
	return (_ccs_configuration_ops_t *)configuration->obj.ops;
}

static ccs_error_t
_ccs_configuration_del(ccs_object_t object) {
	ccs_configuration_t configuration = (ccs_configuration_t)object;
	ccs_release_object(configuration->data->configuration_space);
	return CCS_SUCCESS;
}

static _ccs_configuration_ops_t _configuration_ops =
    { {&_ccs_configuration_del} };

ccs_error_t
ccs_create_configuration(ccs_configuration_space_t configuration_space,
                         size_t                    num_values,
                         ccs_datum_t              *values,
                         void                     *user_data,
                         ccs_configuration_t      *configuration_ret) {
	if (!configuration_ret)
		return -CCS_INVALID_VALUE;
	if (num_values && !values)
		return -CCS_INVALID_VALUE;
	if (!num_values && values)
		return -CCS_INVALID_VALUE;
	ccs_error_t err;
	size_t num;
	err = ccs_configuration_space_get_num_hyperparameters(configuration_space, &num);
	if (err)
		return err;
	if (values && num != num_values)
		return -CCS_INVALID_VALUE;
	uintptr_t mem = (uintptr_t)calloc(1, sizeof(struct _ccs_configuration_s) +
	                                     sizeof(struct _ccs_configuration_data_s) +
	                                     num * sizeof(ccs_datum_t));
	if (!mem)
		return -CCS_ENOMEM;
	err = ccs_retain_object(configuration_space);
	if (err) {
		free((void*)mem);
		return err;
	}
	ccs_configuration_t config = (ccs_configuration_t)mem;
	_ccs_object_init(&(config->obj), CCS_CONFIGURATION, (_ccs_object_ops_t*)&_configuration_ops);
	config->data = (struct _ccs_configuration_data_s*)(mem + sizeof(struct _ccs_configuration_s));
	config->data->user_data = user_data;
	config->data->num_values = num;
	config->data->configuration_space = configuration_space;
	config->data->values = (ccs_datum_t *)(mem + sizeof(struct _ccs_configuration_s) + sizeof(struct _ccs_configuration_data_s));
	if (values)
		memcpy(config->data->values, values, num*sizeof(ccs_datum_t));
	*configuration_ret = config;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_get_configuration_space(ccs_configuration_t        configuration,
                                          ccs_configuration_space_t *configuration_space_ret) {
	if (!configuration || !configuration->data)
		return -CCS_INVALID_OBJECT;
	if (!configuration_space_ret)
		return -CCS_INVALID_VALUE;
	*configuration_space_ret = configuration->data->configuration_space;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_get_user_data(ccs_configuration_t   configuration,
                                void                **user_data_ret) {
	if (!configuration || !configuration->data)
		return -CCS_INVALID_OBJECT;
	if (!user_data_ret)
		return -CCS_INVALID_VALUE;
	*user_data_ret = configuration->data->user_data;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_get_value(ccs_configuration_t  configuration,
                            size_t               index,
                            ccs_datum_t         *value_ret) {
	if (!configuration || !configuration->data)
		return -CCS_INVALID_OBJECT;
	if (!value_ret)
		return -CCS_INVALID_VALUE;
	if (index >= configuration->data->num_values)
		return -CCS_OUT_OF_BOUNDS;
	*value_ret = configuration->data->values[index];
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_set_value(ccs_configuration_t configuration,
                            size_t              index,
                            ccs_datum_t         value) {
	if (!configuration || !configuration->data)
		return -CCS_INVALID_OBJECT;
	if (index >= configuration->data->num_values)
		return -CCS_OUT_OF_BOUNDS;
	configuration->data->values[index] = value;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_get_values(ccs_configuration_t  configuration,
                             size_t               num_values,
                             ccs_datum_t         *values,
                             size_t              *num_values_ret) {
	if (!configuration || !configuration->data)
		return -CCS_INVALID_OBJECT;
	if (!num_values_ret && !values)
		return -CCS_INVALID_VALUE;
	if (num_values && !values)
		return -CCS_INVALID_VALUE;
	if (!num_values && values)
		return -CCS_INVALID_VALUE;
	size_t num = configuration->data->num_values;
	if (values) {
		if (num_values < num)
			return -CCS_INVALID_VALUE;
		memcpy(values, configuration->data->values, num*sizeof(ccs_datum_t));
		for (size_t i = num; i < num_values; i++) {
			values[i].type = CCS_NONE;
			values[i].value.i = 0;
		}
	}
	if (num_values_ret)
		*num_values_ret = num;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_get_value_by_name(ccs_configuration_t  configuration,
                                    const char          *name,
                                    ccs_datum_t         *value_ret) {
	if (!configuration || !configuration->data)
		return -CCS_INVALID_OBJECT;
	if (!name)
		return -CCS_INVALID_VALUE;
	size_t index;
	ccs_error_t err;
	err = ccs_configuration_space_get_hyperparameter_index_by_name(
		configuration->data->configuration_space, name, &index);
	if (err)
		return err;
	*value_ret = configuration->data->values[index];
	return CCS_SUCCESS;
}

ccs_error_t
ccs_configuration_check(ccs_configuration_t configuration) {
	if (!configuration || !configuration->data)
		return -CCS_INVALID_OBJECT;
	return ccs_configuration_space_check_configuration(
		configuration->data->configuration_space, configuration);
}

ccs_error_t
ccs_configuration_hash(ccs_configuration_t  configuration,
                       ccs_hash_t          *hash_ret) {
	if (!configuration || !configuration->data)
		return -CCS_INVALID_OBJECT;
	if (!hash_ret)
		return -CCS_INVALID_VALUE;
	_ccs_configuration_data_t *data = configuration->data;
	ccs_hash_t h, ht;
	HASH_JEN(&(data->configuration_space), sizeof(data->configuration_space), h);
	HASH_JEN(&(data->num_values), sizeof(data->num_values), ht);
	h = _hash_combine(h, ht);
	for (size_t i = 0; i < data->num_values; i++) {
		ht = _hash_datum(data->values + i);
		h = _hash_combine(h, ht);
	}
	*hash_ret = h;
	return CCS_SUCCESS;
}
