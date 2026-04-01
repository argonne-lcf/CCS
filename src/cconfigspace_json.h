#ifndef _CCONFIGSPACE_JSON_H
#define _CCONFIGSPACE_JSON_H
#include "cjson/cJSON.h"
#include <string.h>

/*============================================================================
 * Hex encode/decode helpers
 *============================================================================*/

/* Encode into caller-provided buffer. out must be at least len*2+1 bytes. */
static inline void
_ccs_json_hex_encode_buf(const void *data, size_t len, char *out)
{
	static const char    hex[] = "0123456789abcdef";
	const unsigned char *p     = (const unsigned char *)data;
	for (size_t i = 0; i < len; i++) {
		out[i * 2]     = hex[p[i] >> 4];
		out[i * 2 + 1] = hex[p[i] & 0x0f];
	}
	out[len * 2] = '\0';
}

/* Decode into caller-provided buffer. out must be at least slen/2 bytes.
 * Returns 0 on success, -1 on error. */
static inline int
_ccs_json_hex_decode_buf(const char *hex_str, size_t slen, void *out)
{
	unsigned char *p = (unsigned char *)out;
	if (slen % 2 != 0)
		return -1;
	for (size_t i = 0; i < slen / 2; i++) {
		unsigned int byte;
		char tmp[3] = {hex_str[i * 2], hex_str[i * 2 + 1], '\0'};
		if (sscanf(tmp, "%02x", &byte) != 1)
			return -1;
		p[i] = (unsigned char)byte;
	}
	return 0;
}

/* Allocating wrappers */
static inline char *
_ccs_json_hex_encode(const void *data, size_t len)
{
	char *out = (char *)malloc(len * 2 + 1);
	if (!out)
		return NULL;
	_ccs_json_hex_encode_buf(data, len, out);
	return out;
}

static inline unsigned char *
_ccs_json_hex_decode(const char *hex_str, size_t *out_len)
{
	size_t         slen = strlen(hex_str);
	unsigned char *out;
	if (slen % 2 != 0)
		return NULL;
	*out_len = slen / 2;
	out      = (unsigned char *)malloc(*out_len);
	if (!out)
		return NULL;
	if (_ccs_json_hex_decode_buf(hex_str, slen, out) != 0) {
		free(out);
		return NULL;
	}
	return out;
}

/*============================================================================
 * Object type string conversion
 *============================================================================*/

static const char *_ccs_json_object_type_strings[] = {
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
_ccs_json_object_type_to_string(ccs_object_type_t type)
{
	if (type >= 0 && type < CCS_OBJECT_TYPE_MAX)
		return _ccs_json_object_type_strings[type];
	return NULL;
}

static inline ccs_result_t
_ccs_json_object_type_from_string(const char *str, ccs_object_type_t *type_ret)
{
	for (int i = 0; i < CCS_OBJECT_TYPE_MAX; i++)
		if (!strcmp(str, _ccs_json_object_type_strings[i])) {
			*type_ret = (ccs_object_type_t)i;
			return CCS_RESULT_SUCCESS;
		}
	CCS_RAISE(
		CCS_RESULT_ERROR_INVALID_VALUE,
		"Unknown object type string: %s", str);
}

/*============================================================================
 * Distribution type string conversion
 *============================================================================*/

static const char *_ccs_json_distribution_type_strings[] = {
	"uniform", "normal", "roulette", "mixture", "multivariate",
};

static inline const char *
_ccs_json_distribution_type_to_string(ccs_distribution_type_t type)
{
	if (type >= 0 && type < CCS_DISTRIBUTION_TYPE_MAX)
		return _ccs_json_distribution_type_strings[type];
	return NULL;
}

static inline ccs_result_t
_ccs_json_distribution_type_from_string(
	const char              *str,
	ccs_distribution_type_t *type_ret)
{
	for (int i = 0; i < CCS_DISTRIBUTION_TYPE_MAX; i++)
		if (!strcmp(str, _ccs_json_distribution_type_strings[i])) {
			*type_ret = (ccs_distribution_type_t)i;
			return CCS_RESULT_SUCCESS;
		}
	CCS_RAISE(
		CCS_RESULT_ERROR_INVALID_VALUE,
		"Unknown distribution type string: %s", str);
}

/*============================================================================
 * Numeric type string conversion
 *============================================================================*/

static inline const char *
_ccs_json_numeric_type_to_string(ccs_numeric_type_t type)
{
	switch (type) {
	case CCS_NUMERIC_TYPE_INT:
		return "int";
	case CCS_NUMERIC_TYPE_FLOAT:
		return "float";
	default:
		return NULL;
	}
}

static inline ccs_result_t
_ccs_json_numeric_type_from_string(const char *str, ccs_numeric_type_t *type_ret)
{
	if (!strcmp(str, "int")) {
		*type_ret = CCS_NUMERIC_TYPE_INT;
		return CCS_RESULT_SUCCESS;
	}
	if (!strcmp(str, "float")) {
		*type_ret = CCS_NUMERIC_TYPE_FLOAT;
		return CCS_RESULT_SUCCESS;
	}
	CCS_RAISE(
		CCS_RESULT_ERROR_INVALID_VALUE,
		"Unknown numeric type string: %s", str);
}

/*============================================================================
 * Scale type string conversion
 *============================================================================*/

static inline const char *
_ccs_json_scale_type_to_string(ccs_scale_type_t type)
{
	switch (type) {
	case CCS_SCALE_TYPE_LINEAR:
		return "linear";
	case CCS_SCALE_TYPE_LOGARITHMIC:
		return "logarithmic";
	default:
		return NULL;
	}
}

static inline ccs_result_t
_ccs_json_scale_type_from_string(const char *str, ccs_scale_type_t *type_ret)
{
	if (!strcmp(str, "linear")) {
		*type_ret = CCS_SCALE_TYPE_LINEAR;
		return CCS_RESULT_SUCCESS;
	}
	if (!strcmp(str, "logarithmic")) {
		*type_ret = CCS_SCALE_TYPE_LOGARITHMIC;
		return CCS_RESULT_SUCCESS;
	}
	CCS_RAISE(
		CCS_RESULT_ERROR_INVALID_VALUE, "Unknown scale type string: %s",
		str);
}

/*============================================================================
 * Parameter type string conversion
 *============================================================================*/

static const char *_ccs_json_parameter_type_strings[] = {
	"numerical", "categorical", "ordinal", "discrete", "string",
};

static inline const char *
_ccs_json_parameter_type_to_string(ccs_parameter_type_t type)
{
	if (type >= 0 && type < CCS_PARAMETER_TYPE_MAX)
		return _ccs_json_parameter_type_strings[type];
	return NULL;
}

static inline ccs_result_t
_ccs_json_parameter_type_from_string(
	const char           *str,
	ccs_parameter_type_t *type_ret)
{
	for (int i = 0; i < CCS_PARAMETER_TYPE_MAX; i++)
		if (!strcmp(str, _ccs_json_parameter_type_strings[i])) {
			*type_ret = (ccs_parameter_type_t)i;
			return CCS_RESULT_SUCCESS;
		}
	CCS_RAISE(
		CCS_RESULT_ERROR_INVALID_VALUE,
		"Unknown parameter type string: %s", str);
}

/*============================================================================
 * ccs_datum_t JSON helpers
 *============================================================================*/

static inline ccs_result_t
_ccs_json_datum_to_cjson(ccs_datum_t datum, cJSON **item_ret)
{
	cJSON *item = NULL;
	switch (datum.type) {
	case CCS_DATA_TYPE_NONE:
		item = cJSON_CreateNull();
		break;
	case CCS_DATA_TYPE_INT:
		item = cJSON_CreateNumber(datum.value.i);
		break;
	case CCS_DATA_TYPE_FLOAT:
		item = cJSON_CreateNumber(datum.value.f);
		break;
	case CCS_DATA_TYPE_BOOL:
		item = cJSON_CreateBool(datum.value.i);
		break;
	case CCS_DATA_TYPE_STRING:
		item = cJSON_CreateString(datum.value.s);
		break;
	case CCS_DATA_TYPE_INACTIVE:
		item = cJSON_CreateObject();
		if (item)
			cJSON_AddStringToObject(item, "_type", "inactive");
		break;
	case CCS_DATA_TYPE_OBJECT: {
		char hex[sizeof(ccs_object_t) * 2 + 1];
		_ccs_json_hex_encode_buf(
			&datum.value.o, sizeof(ccs_object_t), hex);
		item = cJSON_CreateObject();
		if (item) {
			cJSON_AddStringToObject(item, "_type", "object");
			cJSON *h = cJSON_AddStringToObject(item, "handle", hex);
			if (!h) {
				cJSON_Delete(item);
				item = NULL;
			}
		}
	} break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported datum type for JSON: %d", datum.type);
	}
	CCS_REFUTE(!item, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	*item_ret = item;
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_json_add_datum(cJSON *json, const char *key, ccs_datum_t datum)
{
	cJSON *item = NULL;
	CCS_VALIDATE(_ccs_json_datum_to_cjson(datum, &item));
	CCS_REFUTE(
		!cJSON_AddItemToObject(json, key, item),
		CCS_RESULT_ERROR_OUT_OF_MEMORY);
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_json_add_datum_to_array(cJSON *array, ccs_datum_t datum)
{
	cJSON *item = NULL;
	CCS_VALIDATE(_ccs_json_datum_to_cjson(datum, &item));
	CCS_REFUTE(
		!cJSON_AddItemToArray(array, item),
		CCS_RESULT_ERROR_OUT_OF_MEMORY);
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_json_get_datum(cJSON *item, ccs_datum_t *datum_ret)
{
	if (cJSON_IsNull(item)) {
		*datum_ret = ccs_none;
	} else if (cJSON_IsBool(item)) {
		*datum_ret =
			ccs_bool(cJSON_IsTrue(item) ? CCS_TRUE : CCS_FALSE);
	} else if (cJSON_IsNumber(item)) {
		double    v  = item->valuedouble;
		ccs_int_t iv = (ccs_int_t)v;
		if ((double)iv == v)
			*datum_ret = ccs_int(iv);
		else
			*datum_ret = ccs_float(v);
	} else if (cJSON_IsString(item)) {
		*datum_ret = ccs_string(item->valuestring);
	} else if (cJSON_IsObject(item)) {
		cJSON *t = cJSON_GetObjectItemCaseSensitive(item, "_type");
		CCS_REFUTE(
			!t || !cJSON_IsString(t),
			CCS_RESULT_ERROR_INVALID_VALUE);
		if (!strcmp(t->valuestring, "inactive")) {
			*datum_ret = ccs_inactive;
		} else if (!strcmp(t->valuestring, "object")) {
			cJSON *h = cJSON_GetObjectItemCaseSensitive(
				item, "handle");
			CCS_REFUTE(
				!h || !cJSON_IsString(h),
				CCS_RESULT_ERROR_INVALID_VALUE);
			CCS_REFUTE(
				strlen(h->valuestring) !=
					sizeof(ccs_object_t) * 2,
				CCS_RESULT_ERROR_INVALID_VALUE);
			ccs_object_t obj;
			CCS_REFUTE(
				_ccs_json_hex_decode_buf(
					h->valuestring,
					sizeof(ccs_object_t) * 2, &obj),
				CCS_RESULT_ERROR_INVALID_VALUE);
			*datum_ret = ccs_object(obj);
		} else {
			CCS_RAISE(
				CCS_RESULT_ERROR_INVALID_VALUE,
				"Unknown JSON datum _type: %s", t->valuestring);
		}
	} else {
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported JSON datum type");
	}
	return CCS_RESULT_SUCCESS;
}

#endif /* _CCONFIGSPACE_JSON_H */
