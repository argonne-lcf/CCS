#ifndef _CCONFIGSPACE_JSON_H
#define _CCONFIGSPACE_JSON_H
#include "cconfigspace_internal.h"
#include "rng_internal.h"
#include "cjson/cJSON.h"
#include <string.h>

/*============================================================================
 * Hex encode/decode helpers
 *============================================================================*/

static inline char *
_ccs_hex_encode(const void *data, size_t len)
{
	static const char hex[] = "0123456789abcdef";
	char             *out   = (char *)malloc(len * 2 + 1);
	if (!out)
		return NULL;
	const unsigned char *p = (const unsigned char *)data;
	for (size_t i = 0; i < len; i++) {
		out[i * 2]     = hex[p[i] >> 4];
		out[i * 2 + 1] = hex[p[i] & 0x0f];
	}
	out[len * 2] = '\0';
	return out;
}

static inline unsigned char *
_ccs_hex_decode(const char *hex_str, size_t *out_len)
{
	size_t         slen = strlen(hex_str);
	unsigned char *out;
	if (slen % 2 != 0)
		return NULL;
	*out_len = slen / 2;
	out      = (unsigned char *)malloc(*out_len);
	if (!out)
		return NULL;
	for (size_t i = 0; i < *out_len; i++) {
		unsigned int byte;
		char         tmp[3] = {hex_str[i * 2], hex_str[i * 2 + 1], '\0'};
		if (sscanf(tmp, "%02x", &byte) != 1) {
			free(out);
			return NULL;
		}
		out[i] = (unsigned char)byte;
	}
	return out;
}

/*============================================================================
 * Object type string conversion
 *============================================================================*/

static const char *_ccs_object_type_strings[] = {
	"rng",
	"distribution",
	"parameter",
	"expression",
	"configuration_space",
	"configuration",
	"objective_space",
	"evaluation",
	"tuner",
	"feature_space",
	"features",
	"map",
	"error_stack",
	"tree",
	"tree_space",
	"tree_configuration",
	"distribution_space",
};

static inline const char *
_ccs_object_type_to_string(ccs_object_type_t type)
{
	if (type >= 0 && type < CCS_OBJECT_TYPE_MAX)
		return _ccs_object_type_strings[type];
	return NULL;
}

static inline ccs_result_t
_ccs_object_type_from_string(const char *str, ccs_object_type_t *type_ret)
{
	for (int i = 0; i < CCS_OBJECT_TYPE_MAX; i++)
		if (!strcmp(str, _ccs_object_type_strings[i])) {
			*type_ret = (ccs_object_type_t)i;
			return CCS_RESULT_SUCCESS;
		}
	CCS_RAISE(
		CCS_RESULT_ERROR_INVALID_VALUE,
		"Unknown object type string: %s", str);
}

/*============================================================================
 * cJSON helpers
 *============================================================================*/

/* Validate that a cJSON_Add* call succeeded (returns NULL on OOM) */
#define CCS_JSON_CHECK_ADD(ptr)                                                \
	CCS_REFUTE(!(ptr), CCS_RESULT_ERROR_OUT_OF_MEMORY)

/*============================================================================
 * Per-type JSON serialize functions: object → cJSON
 *
 * Each function populates a cJSON object with the type-specific fields.
 * The caller has already created the root object and added "version" and
 * "object_type".
 *============================================================================*/

/* --- RNG --- */
static inline ccs_result_t
_ccs_serialize_json_rng(ccs_rng_t rng, cJSON *json)
{
	_ccs_rng_data_t *data = (_ccs_rng_data_t *)(rng->data);
	char            *hex;

	CCS_JSON_CHECK_ADD(
		cJSON_AddStringToObject(json, "rng_type", gsl_rng_name(data->rng)));
	CCS_JSON_CHECK_ADD(cJSON_AddBoolToObject(
		json, "little_endian", ccs_is_little_endian()));

	hex = _ccs_hex_encode(
		gsl_rng_state(data->rng), gsl_rng_size(data->rng));
	CCS_REFUTE(!hex, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	cJSON *s = cJSON_AddStringToObject(json, "state", hex);
	free(hex);
	CCS_JSON_CHECK_ADD(s);

	return CCS_RESULT_SUCCESS;
}

/*============================================================================
 * Per-type JSON deserialize functions: cJSON → object
 *============================================================================*/

static const gsl_rng_type **_ccs_json_gsl_rng_types = NULL;

static inline ccs_result_t
_ccs_deserialize_json_rng(
	ccs_rng_t                         *rng_ret,
	cJSON                             *json,
	_ccs_object_deserialize_options_t *opts)
{
	(void)opts;
	cJSON *j_rng_type =
		cJSON_GetObjectItemCaseSensitive(json, "rng_type");
	cJSON *j_little_endian =
		cJSON_GetObjectItemCaseSensitive(json, "little_endian");
	cJSON *j_state =
		cJSON_GetObjectItemCaseSensitive(json, "state");

	CCS_REFUTE(
		!j_rng_type || !cJSON_IsString(j_rng_type),
		CCS_RESULT_ERROR_INVALID_VALUE);
	CCS_REFUTE(
		!j_little_endian || !cJSON_IsBool(j_little_endian),
		CCS_RESULT_ERROR_INVALID_VALUE);
	CCS_REFUTE(
		!j_state || !cJSON_IsString(j_state),
		CCS_RESULT_ERROR_INVALID_VALUE);

	if (!_ccs_json_gsl_rng_types)
		_ccs_json_gsl_rng_types = gsl_rng_types_setup();

	const gsl_rng_type **t;
	for (t = _ccs_json_gsl_rng_types; *t != NULL; t++)
		if (!strcmp(j_rng_type->valuestring, (*t)->name))
			break;
	CCS_REFUTE(!*t, CCS_RESULT_ERROR_INVALID_VALUE);

	CCS_VALIDATE(ccs_create_rng_with_type(*t, rng_ret));

	/* Restore state if compatible */
	ccs_bool_t little_endian =
		cJSON_IsTrue(j_little_endian) ? CCS_TRUE : CCS_FALSE;
	size_t          state_len;
	unsigned char  *state_bytes =
		_ccs_hex_decode(j_state->valuestring, &state_len);
	if (state_bytes) {
		if (state_len == gsl_rng_size((*rng_ret)->data->rng) &&
		    little_endian == ccs_is_little_endian())
			memcpy(gsl_rng_state((*rng_ret)->data->rng),
			       state_bytes, state_len);
		free(state_bytes);
	}

	return CCS_RESULT_SUCCESS;
}

/*============================================================================
 * Top-level JSON dispatch
 *============================================================================*/

static inline ccs_result_t
_ccs_object_serialize_to_json(
	ccs_object_t                     object,
	cJSON                          **json_ret,
	_ccs_object_serialize_options_t *opts)
{
	(void)opts;
	_ccs_object_internal_t *obj = (_ccs_object_internal_t *)object;
	cJSON                  *json;
	ccs_result_t            err = CCS_RESULT_SUCCESS;

	json = cJSON_CreateObject();
	CCS_REFUTE(!json, CCS_RESULT_ERROR_OUT_OF_MEMORY);

	/* Envelope: version + object_type */
	if (!cJSON_AddNumberToObject(json, "version",
	                             CCS_SERIALIZATION_API_VERSION) ||
	    !cJSON_AddStringToObject(json, "object_type",
	                             _ccs_object_type_to_string(obj->type))) {
		cJSON_Delete(json);
		CCS_RAISE(CCS_RESULT_ERROR_OUT_OF_MEMORY,
		          "Failed to create JSON envelope");
	}

	CCS_OBJ_RDLOCK(object);
	switch (obj->type) {
	case CCS_OBJECT_TYPE_RNG:
		err = _ccs_serialize_json_rng((ccs_rng_t)object, json);
		break;
	default:
		err = CCS_RESULT_ERROR_INVALID_VALUE;
		break;
	}
	CCS_OBJ_UNLOCK(object);

	if (err != CCS_RESULT_SUCCESS) {
		cJSON_Delete(json);
		return err;
	}

	/* Serialize user_data if present */
	if (obj->user_data && (obj->serialize_callback ||
	                       (opts && opts->serialize_callback))) {
		size_t serialize_data_size = 0;
		ccs_object_serialize_callback_t cb =
			obj->serialize_callback ? obj->serialize_callback
						: opts->serialize_callback;
		void *cb_data = obj->serialize_callback
					? obj->serialize_user_data
					: opts->serialize_user_data;
		CCS_VALIDATE_ERR_GOTO(
			err, cb(object, 0, NULL, &serialize_data_size, cb_data),
			err_json);
		if (serialize_data_size) {
			char *tmp = (char *)malloc(serialize_data_size);
			if (!tmp) {
				err = CCS_RESULT_ERROR_OUT_OF_MEMORY;
				goto err_json;
			}
			err = cb(object, serialize_data_size, tmp, NULL,
			         cb_data);
			if (err != CCS_RESULT_SUCCESS) {
				free(tmp);
				goto err_json;
			}
			char *hex = _ccs_hex_encode(tmp, serialize_data_size);
			free(tmp);
			if (!hex) {
				err = CCS_RESULT_ERROR_OUT_OF_MEMORY;
				goto err_json;
			}
			cJSON *ud =
				cJSON_AddStringToObject(json, "user_data", hex);
			free(hex);
			if (!ud) {
				err = CCS_RESULT_ERROR_OUT_OF_MEMORY;
				goto err_json;
			}
		}
	}

	*json_ret = json;
	return CCS_RESULT_SUCCESS;
err_json:
	cJSON_Delete(json);
	return err;
}

static inline ccs_result_t
_ccs_object_deserialize_from_json(
	ccs_object_t                      *object_ret,
	cJSON                             *json,
	_ccs_object_deserialize_options_t *opts)
{
	ccs_result_t      err;
	ccs_object_type_t otype;

	cJSON *j_version =
		cJSON_GetObjectItemCaseSensitive(json, "version");
	cJSON *j_object_type =
		cJSON_GetObjectItemCaseSensitive(json, "object_type");

	CCS_REFUTE(
		!j_version || !cJSON_IsNumber(j_version),
		CCS_RESULT_ERROR_INVALID_VALUE);
	CCS_REFUTE(
		(uint32_t)j_version->valuedouble > CCS_SERIALIZATION_API_VERSION,
		CCS_RESULT_ERROR_INVALID_VALUE);
	CCS_REFUTE(
		!j_object_type || !cJSON_IsString(j_object_type),
		CCS_RESULT_ERROR_INVALID_VALUE);

	CCS_VALIDATE(
		_ccs_object_type_from_string(j_object_type->valuestring, &otype));

	switch (otype) {
	case CCS_OBJECT_TYPE_RNG:
		CCS_VALIDATE(_ccs_deserialize_json_rng(
			(ccs_rng_t *)object_ret, json, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"JSON deserialization not yet supported for type: %s",
			j_object_type->valuestring);
	}

	/* Deserialize user_data if present */
	cJSON *j_user_data =
		cJSON_GetObjectItemCaseSensitive(json, "user_data");
	if (j_user_data && cJSON_IsString(j_user_data) &&
	    opts->deserialize_data_callback) {
		size_t          data_len;
		unsigned char  *data_bytes =
			_ccs_hex_decode(j_user_data->valuestring, &data_len);
		CCS_REFUTE(!data_bytes, CCS_RESULT_ERROR_OUT_OF_MEMORY);
		err = opts->deserialize_data_callback(
			*object_ret, data_len, (const char *)data_bytes,
			opts->deserialize_data_user_data);
		free(data_bytes);
		CCS_VALIDATE(err);
	}

	/* Handle map if requested */
	if (opts->map_values) {
		/* For JSON, we use the object pointer as its own handle */
		CCS_VALIDATE(_ccs_object_handle_check_add(
			opts->handle_map, *object_ret, *object_ret));
	}

	return CCS_RESULT_SUCCESS;
}

/*============================================================================
 * JSON string serialize/deserialize convenience functions
 *============================================================================*/

static inline ccs_result_t
_ccs_object_serialize_json_to_string(
	ccs_object_t                     object,
	char                           **string_ret,
	size_t                          *size_ret,
	_ccs_object_serialize_options_t *opts)
{
	cJSON *json = NULL;
	char  *str;
	CCS_VALIDATE(_ccs_object_serialize_to_json(object, &json, opts));
	str = cJSON_PrintUnformatted(json);
	cJSON_Delete(json);
	CCS_REFUTE(!str, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	*string_ret = str;
	*size_ret   = strlen(str) + 1;
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_object_deserialize_json_from_string(
	ccs_object_t                      *object_ret,
	size_t                             buffer_size,
	const char                        *buffer,
	_ccs_object_deserialize_options_t *opts)
{
	ccs_result_t err;
	cJSON       *json = cJSON_ParseWithLength(buffer, buffer_size);
	CCS_REFUTE(!json, CCS_RESULT_ERROR_INVALID_VALUE);
	err = _ccs_object_deserialize_from_json(object_ret, json, opts);
	cJSON_Delete(json);
	return err;
}

#endif /* _CCONFIGSPACE_JSON_H */
