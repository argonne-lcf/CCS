#ifndef _CCONFIGSPACE_JSON_H
#define _CCONFIGSPACE_JSON_H
#include "cjson/cJSON.h"
#include <math.h>
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
 * Expression type string conversion
 *============================================================================*/

static const char *_ccs_json_expression_type_strings[] = {
	"or",           "and",       "equal",         "not_equal",
	"less",         "greater",   "less_or_equal", "greater_or_equal",
	"add",          "substract", "multiply",      "divide",
	"modulo",       "positive",  "negative",      "not",
	"in",           "list",      "literal",       "variable",
	"user_defined",
};

static inline const char *
_ccs_json_expression_type_to_string(ccs_expression_type_t type)
{
	if (type >= 0 && type < CCS_EXPRESSION_TYPE_MAX)
		return _ccs_json_expression_type_strings[type];
	return NULL;
}

static inline ccs_result_t
_ccs_json_expression_type_from_string(
	const char            *str,
	ccs_expression_type_t *type_ret)
{
	for (int i = 0; i < CCS_EXPRESSION_TYPE_MAX; i++)
		if (!strcmp(str, _ccs_json_expression_type_strings[i])) {
			*type_ret = (ccs_expression_type_t)i;
			return CCS_RESULT_SUCCESS;
		}
	CCS_RAISE(
		CCS_RESULT_ERROR_INVALID_VALUE,
		"Unknown expression type string: %s", str);
}

/*============================================================================
 * Tree space type string conversion
 *============================================================================*/

static inline const char *
_ccs_json_tree_space_type_to_string(ccs_tree_space_type_t type)
{
	switch (type) {
	case CCS_TREE_SPACE_TYPE_STATIC:
		return "static";
	case CCS_TREE_SPACE_TYPE_DYNAMIC:
		return "dynamic";
	default:
		return NULL;
	}
}

static inline ccs_result_t
_ccs_json_tree_space_type_from_string(
	const char            *str,
	ccs_tree_space_type_t *type_ret)
{
	if (!strcmp(str, "static")) {
		*type_ret = CCS_TREE_SPACE_TYPE_STATIC;
		return CCS_RESULT_SUCCESS;
	}
	if (!strcmp(str, "dynamic")) {
		*type_ret = CCS_TREE_SPACE_TYPE_DYNAMIC;
		return CCS_RESULT_SUCCESS;
	}
	CCS_RAISE(
		CCS_RESULT_ERROR_INVALID_VALUE,
		"Unknown tree space type string: %s", str);
}

/*============================================================================
 * Objective type string conversion
 *============================================================================*/

static inline const char *
_ccs_json_objective_type_to_string(ccs_objective_type_t type)
{
	switch (type) {
	case CCS_OBJECTIVE_TYPE_MINIMIZE:
		return "minimize";
	case CCS_OBJECTIVE_TYPE_MAXIMIZE:
		return "maximize";
	default:
		return NULL;
	}
}

static inline ccs_result_t
_ccs_json_objective_type_from_string(
	const char           *str,
	ccs_objective_type_t *type_ret)
{
	if (!strcmp(str, "minimize")) {
		*type_ret = CCS_OBJECTIVE_TYPE_MINIMIZE;
		return CCS_RESULT_SUCCESS;
	}
	if (!strcmp(str, "maximize")) {
		*type_ret = CCS_OBJECTIVE_TYPE_MAXIMIZE;
		return CCS_RESULT_SUCCESS;
	}
	CCS_RAISE(
		CCS_RESULT_ERROR_INVALID_VALUE,
		"Unknown objective type string: %s", str);
}

/*============================================================================
 * Float JSON helpers (handles Infinity and NaN)
 *============================================================================*/

/* Add a float value to a cJSON object, encoding non-finite values as strings
 * so that Infinity and NaN survive round-trip. */
static inline ccs_result_t
_ccs_json_add_float(cJSON *json, const char *key, double value)
{
	if (isfinite(value)) {
		CCS_REFUTE(
			!cJSON_AddNumberToObject(json, key, value),
			CCS_RESULT_ERROR_OUT_OF_MEMORY);
	} else if (isinf(value)) {
		CCS_REFUTE(
			!cJSON_AddStringToObject(
				json, key,
				value > 0 ? "Infinity" : "-Infinity"),
			CCS_RESULT_ERROR_OUT_OF_MEMORY);
	} else {
		CCS_REFUTE(
			!cJSON_AddStringToObject(json, key, "NaN"),
			CCS_RESULT_ERROR_OUT_OF_MEMORY);
	}
	return CCS_RESULT_SUCCESS;
}

/* Read a float value that may be a number or a string ("Infinity", etc.). */
static inline ccs_result_t
_ccs_json_get_float(cJSON *item, double *value_ret)
{
	if (cJSON_IsNumber(item)) {
		*value_ret = item->valuedouble;
	} else if (cJSON_IsString(item)) {
		if (!strcmp(item->valuestring, "Infinity"))
			*value_ret = INFINITY;
		else if (!strcmp(item->valuestring, "-Infinity"))
			*value_ret = -INFINITY;
		else if (!strcmp(item->valuestring, "NaN"))
			*value_ret = NAN;
		else
			CCS_RAISE(
				CCS_RESULT_ERROR_INVALID_VALUE,
				"Unknown float string: %s", item->valuestring);
	} else {
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Expected number or string for float value");
	}
	return CCS_RESULT_SUCCESS;
}

/*============================================================================
 * ccs_datum_t JSON helpers
 *============================================================================*/

static inline ccs_result_t
_ccs_json_datum_to_cjson(ccs_datum_t datum, cJSON **item_ret)
{
	ccs_result_t err  = CCS_RESULT_SUCCESS;
	cJSON       *item = cJSON_CreateObject();
	CCS_REFUTE(!item, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	switch (datum.type) {
	case CCS_DATA_TYPE_NONE:
		CCS_REFUTE_ERR_GOTO(
			err, !cJSON_AddStringToObject(item, "type", "none"),
			CCS_RESULT_ERROR_OUT_OF_MEMORY, err_item);
		break;
	case CCS_DATA_TYPE_INT:
		CCS_REFUTE_ERR_GOTO(
			err, !cJSON_AddStringToObject(item, "type", "int"),
			CCS_RESULT_ERROR_OUT_OF_MEMORY, err_item);
		CCS_REFUTE_ERR_GOTO(
			err,
			!cJSON_AddNumberToObject(item, "value", datum.value.i),
			CCS_RESULT_ERROR_OUT_OF_MEMORY, err_item);
		break;
	case CCS_DATA_TYPE_FLOAT:
		CCS_REFUTE_ERR_GOTO(
			err, !cJSON_AddStringToObject(item, "type", "float"),
			CCS_RESULT_ERROR_OUT_OF_MEMORY, err_item);
		CCS_VALIDATE_ERR_GOTO(
			err, _ccs_json_add_float(item, "value", datum.value.f),
			err_item);
		break;
	case CCS_DATA_TYPE_BOOL:
		CCS_REFUTE_ERR_GOTO(
			err, !cJSON_AddStringToObject(item, "type", "bool"),
			CCS_RESULT_ERROR_OUT_OF_MEMORY, err_item);
		CCS_REFUTE_ERR_GOTO(
			err,
			!cJSON_AddBoolToObject(item, "value", datum.value.i),
			CCS_RESULT_ERROR_OUT_OF_MEMORY, err_item);
		break;
	case CCS_DATA_TYPE_STRING:
		CCS_REFUTE_ERR_GOTO(
			err, !cJSON_AddStringToObject(item, "type", "string"),
			CCS_RESULT_ERROR_OUT_OF_MEMORY, err_item);
		CCS_REFUTE_ERR_GOTO(
			err,
			!cJSON_AddStringToObject(item, "value", datum.value.s),
			CCS_RESULT_ERROR_OUT_OF_MEMORY, err_item);
		break;
	case CCS_DATA_TYPE_INACTIVE:
		CCS_REFUTE_ERR_GOTO(
			err, !cJSON_AddStringToObject(item, "type", "inactive"),
			CCS_RESULT_ERROR_OUT_OF_MEMORY, err_item);
		break;
	case CCS_DATA_TYPE_OBJECT: {
		char hex[sizeof(ccs_object_t) * 2 + 1];
		_ccs_json_hex_encode_buf(
			&datum.value.o, sizeof(ccs_object_t), hex);
		CCS_REFUTE_ERR_GOTO(
			err, !cJSON_AddStringToObject(item, "type", "object"),
			CCS_RESULT_ERROR_OUT_OF_MEMORY, err_item);
		CCS_REFUTE_ERR_GOTO(
			err, !cJSON_AddStringToObject(item, "value", hex),
			CCS_RESULT_ERROR_OUT_OF_MEMORY, err_item);
	} break;
	default:
		CCS_RAISE_ERR_GOTO(
			err, CCS_RESULT_ERROR_INVALID_VALUE, err_item,
			"Unsupported datum type for JSON: %d", datum.type);
	}
	*item_ret = item;
	return CCS_RESULT_SUCCESS;
err_item:
	cJSON_Delete(item);
	return err;
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
	cJSON *j_type  = NULL;
	cJSON *j_value = NULL;
	CCS_REFUTE(!cJSON_IsObject(item), CCS_RESULT_ERROR_INVALID_VALUE);
	j_type = cJSON_GetObjectItemCaseSensitive(item, "type");
	CCS_REFUTE(
		!j_type || !cJSON_IsString(j_type),
		CCS_RESULT_ERROR_INVALID_VALUE);
	if (!strcmp(j_type->valuestring, "none")) {
		*datum_ret = ccs_none;
	} else if (!strcmp(j_type->valuestring, "int")) {
		j_value = cJSON_GetObjectItemCaseSensitive(item, "value");
		CCS_REFUTE(
			!j_value || !cJSON_IsNumber(j_value),
			CCS_RESULT_ERROR_INVALID_VALUE);
		*datum_ret = ccs_int((ccs_int_t)j_value->valuedouble);
	} else if (!strcmp(j_type->valuestring, "float")) {
		double fval;
		j_value = cJSON_GetObjectItemCaseSensitive(item, "value");
		CCS_REFUTE(!j_value, CCS_RESULT_ERROR_INVALID_VALUE);
		CCS_VALIDATE(_ccs_json_get_float(j_value, &fval));
		*datum_ret = ccs_float(fval);
	} else if (!strcmp(j_type->valuestring, "bool")) {
		j_value = cJSON_GetObjectItemCaseSensitive(item, "value");
		CCS_REFUTE(
			!j_value || !cJSON_IsBool(j_value),
			CCS_RESULT_ERROR_INVALID_VALUE);
		*datum_ret =
			ccs_bool(cJSON_IsTrue(j_value) ? CCS_TRUE : CCS_FALSE);
	} else if (!strcmp(j_type->valuestring, "string")) {
		j_value = cJSON_GetObjectItemCaseSensitive(item, "value");
		CCS_REFUTE(
			!j_value || !cJSON_IsString(j_value),
			CCS_RESULT_ERROR_INVALID_VALUE);
		*datum_ret = ccs_string(j_value->valuestring);
	} else if (!strcmp(j_type->valuestring, "inactive")) {
		*datum_ret = ccs_inactive;
	} else if (!strcmp(j_type->valuestring, "object")) {
		ccs_object_t obj;
		j_value = cJSON_GetObjectItemCaseSensitive(item, "value");
		CCS_REFUTE(
			!j_value || !cJSON_IsString(j_value),
			CCS_RESULT_ERROR_INVALID_VALUE);
		CCS_REFUTE(
			strlen(j_value->valuestring) !=
				sizeof(ccs_object_t) * 2,
			CCS_RESULT_ERROR_INVALID_VALUE);
		CCS_REFUTE(
			_ccs_json_hex_decode_buf(
				j_value->valuestring, sizeof(ccs_object_t) * 2,
				&obj),
			CCS_RESULT_ERROR_INVALID_VALUE);
		*datum_ret = ccs_object(obj);
	} else {
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unknown JSON datum type: %s", j_type->valuestring);
	}
	return CCS_RESULT_SUCCESS;
}

#endif /* _CCONFIGSPACE_JSON_H */
