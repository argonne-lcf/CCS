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
 * String JSON helpers
 *============================================================================*/

static inline ccs_result_t
_ccs_json_add_string(cJSON *json, const char *key, const char *value)
{
	CCS_REFUTE(
		!cJSON_AddStringToObject(json, key, value),
		CCS_RESULT_ERROR_OUT_OF_MEMORY);
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_json_get_string(cJSON *item, const char **value_ret)
{
	CCS_REFUTE(
		!item || !cJSON_IsString(item), CCS_RESULT_ERROR_INVALID_VALUE);
	*value_ret = item->valuestring;
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_json_create_string(const char *value, cJSON **item_ret)
{
	*item_ret = cJSON_CreateString(value);
	CCS_REFUTE(!*item_ret, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	return CCS_RESULT_SUCCESS;
}

/*============================================================================
 * Bool JSON helpers
 *============================================================================*/

static inline ccs_result_t
_ccs_json_add_bool(cJSON *json, const char *key, ccs_bool_t value)
{
	CCS_REFUTE(
		!cJSON_AddBoolToObject(json, key, value),
		CCS_RESULT_ERROR_OUT_OF_MEMORY);
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_json_get_bool(cJSON *item, ccs_bool_t *value_ret)
{
	CCS_REFUTE(
		!item || !cJSON_IsBool(item), CCS_RESULT_ERROR_INVALID_VALUE);
	*value_ret = cJSON_IsTrue(item) ? CCS_TRUE : CCS_FALSE;
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_json_create_bool(ccs_bool_t value, cJSON **item_ret)
{
	*item_ret = cJSON_CreateBool(value);
	CCS_REFUTE(!*item_ret, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	return CCS_RESULT_SUCCESS;
}

/*============================================================================
 * Integer JSON helpers (guards against precision loss)
 *============================================================================*/

/* Safe integer range matching JavaScript's Number.MAX_SAFE_INTEGER
 * and Number.MIN_SAFE_INTEGER: ±(2^53 - 1). */
#define CCS_JSON_INT_MAX (((ccs_int_t)1 << 53) - 1)
#define CCS_JSON_INT_MIN (-(CCS_JSON_INT_MAX))

/* Add an integer value to a cJSON object, raising an error if the value
 * exceeds the range representable by a double without precision loss. */
static inline ccs_result_t
_ccs_json_add_int(cJSON *json, const char *key, ccs_int_t value)
{
	CCS_REFUTE(
		value > CCS_JSON_INT_MAX || value < CCS_JSON_INT_MIN,
		CCS_RESULT_ERROR_INVALID_VALUE);
	CCS_REFUTE(
		!cJSON_AddNumberToObject(json, key, (double)value),
		CCS_RESULT_ERROR_OUT_OF_MEMORY);
	return CCS_RESULT_SUCCESS;
}

/* Create a cJSON number item from an integer, with range check. */
static inline ccs_result_t
_ccs_json_create_int(ccs_int_t value, cJSON **item_ret)
{
	CCS_REFUTE(
		value > CCS_JSON_INT_MAX || value < CCS_JSON_INT_MIN,
		CCS_RESULT_ERROR_INVALID_VALUE);
	*item_ret = cJSON_CreateNumber((double)value);
	CCS_REFUTE(!*item_ret, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	return CCS_RESULT_SUCCESS;
}

/* Read an integer value from a cJSON number. */
static inline ccs_result_t
_ccs_json_get_int(cJSON *item, ccs_int_t *value_ret)
{
	CCS_REFUTE(
		!item || !cJSON_IsNumber(item), CCS_RESULT_ERROR_INVALID_VALUE);
	*value_ret = (ccs_int_t)item->valuedouble;
	return CCS_RESULT_SUCCESS;
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

/* Create a cJSON item from a float, encoding non-finite values as strings. */
static inline ccs_result_t
_ccs_json_create_float(double value, cJSON **item_ret)
{
	cJSON *item = NULL;
	if (isfinite(value)) {
		item = cJSON_CreateNumber(value);
	} else if (isinf(value)) {
		item = cJSON_CreateString(value > 0 ? "Infinity" : "-Infinity");
	} else {
		item = cJSON_CreateString("NaN");
	}
	CCS_REFUTE(!item, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	*item_ret = item;
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
 * Extract helpers (lookup by key + validate + read in one call)
 *============================================================================*/

static inline ccs_result_t
_ccs_json_extract_string(cJSON *json, const char *key, const char **value_ret)
{
	cJSON *item = cJSON_GetObjectItemCaseSensitive(json, key);
	CCS_VALIDATE(_ccs_json_get_string(item, value_ret));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_json_extract_bool(cJSON *json, const char *key, ccs_bool_t *value_ret)
{
	cJSON *item = cJSON_GetObjectItemCaseSensitive(json, key);
	CCS_VALIDATE(_ccs_json_get_bool(item, value_ret));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_json_extract_int(cJSON *json, const char *key, ccs_int_t *value_ret)
{
	cJSON *item = cJSON_GetObjectItemCaseSensitive(json, key);
	CCS_VALIDATE(_ccs_json_get_int(item, value_ret));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_json_extract_float(cJSON *json, const char *key, double *value_ret)
{
	cJSON *item = cJSON_GetObjectItemCaseSensitive(json, key);
	CCS_VALIDATE(_ccs_json_get_float(item, value_ret));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_json_extract_datum(cJSON *json, const char *key, ccs_datum_t *value_ret)
{
	cJSON *item = cJSON_GetObjectItemCaseSensitive(json, key);
	CCS_REFUTE(!item, CCS_RESULT_ERROR_INVALID_VALUE);
	CCS_VALIDATE(_ccs_json_get_datum(item, value_ret));
	return CCS_RESULT_SUCCESS;
}

/*============================================================================
 * Array JSON helpers
 *============================================================================*/

/* Add an array to a JSON object and return a pointer to it. */
static inline ccs_result_t
_ccs_json_add_array(cJSON *json, const char *key, cJSON **array_ret)
{
	*array_ret = cJSON_AddArrayToObject(json, key);
	CCS_REFUTE(!*array_ret, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	return CCS_RESULT_SUCCESS;
}

/* Look up an array by key in a JSON object and return it with its size. */
static inline ccs_result_t
_ccs_json_extract_array(
	cJSON      *json,
	const char *key,
	cJSON     **array_ret,
	size_t     *count_ret)
{
	cJSON *item = cJSON_GetObjectItemCaseSensitive(json, key);
	CCS_REFUTE(
		!item || !cJSON_IsArray(item), CCS_RESULT_ERROR_INVALID_VALUE);
	*array_ret = item;
	*count_ret = (size_t)cJSON_GetArraySize(item);
	return CCS_RESULT_SUCCESS;
}

/* Create an empty JSON object and append it to a JSON array. */
static inline ccs_result_t
_ccs_json_add_object_to_array(cJSON *array, cJSON **object_ret)
{
	ccs_result_t err = CCS_RESULT_SUCCESS;
	*object_ret      = cJSON_CreateObject();
	CCS_REFUTE(!*object_ret, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	CCS_REFUTE_ERR_GOTO(
		err, !cJSON_AddItemToArray(array, *object_ret),
		CCS_RESULT_ERROR_OUT_OF_MEMORY, err_item);
	return CCS_RESULT_SUCCESS;
err_item:
	cJSON_Delete(*object_ret);
	*object_ret = NULL;
	return err;
}

/* Append an integer value to a JSON array. */
static inline ccs_result_t
_ccs_json_add_int_to_array(cJSON *array, ccs_int_t value)
{
	ccs_result_t err = CCS_RESULT_SUCCESS;
	cJSON       *item;
	CCS_VALIDATE(_ccs_json_create_int(value, &item));
	CCS_REFUTE_ERR_GOTO(
		err, !cJSON_AddItemToArray(array, item),
		CCS_RESULT_ERROR_OUT_OF_MEMORY, err_item);
	return CCS_RESULT_SUCCESS;
err_item:
	cJSON_Delete(item);
	return err;
}

/* Append a float value to a JSON array. */
static inline ccs_result_t
_ccs_json_add_float_to_array(cJSON *array, double value)
{
	ccs_result_t err = CCS_RESULT_SUCCESS;
	cJSON       *item;
	CCS_VALIDATE(_ccs_json_create_float(value, &item));
	CCS_REFUTE_ERR_GOTO(
		err, !cJSON_AddItemToArray(array, item),
		CCS_RESULT_ERROR_OUT_OF_MEMORY, err_item);
	return CCS_RESULT_SUCCESS;
err_item:
	cJSON_Delete(item);
	return err;
}

/* Append a string value to a JSON array. */
static inline ccs_result_t
_ccs_json_add_string_to_array(cJSON *array, const char *value)
{
	ccs_result_t err = CCS_RESULT_SUCCESS;
	cJSON       *item;
	CCS_VALIDATE(_ccs_json_create_string(value, &item));
	CCS_REFUTE_ERR_GOTO(
		err, !cJSON_AddItemToArray(array, item),
		CCS_RESULT_ERROR_OUT_OF_MEMORY, err_item);
	return CCS_RESULT_SUCCESS;
err_item:
	cJSON_Delete(item);
	return err;
}

/* Append a null value to a JSON array. */
static inline ccs_result_t
_ccs_json_add_null_to_array(cJSON *array)
{
	ccs_result_t err  = CCS_RESULT_SUCCESS;
	cJSON       *item = cJSON_CreateNull();
	CCS_REFUTE(!item, CCS_RESULT_ERROR_OUT_OF_MEMORY);
	CCS_REFUTE_ERR_GOTO(
		err, !cJSON_AddItemToArray(array, item),
		CCS_RESULT_ERROR_OUT_OF_MEMORY, err_item);
	return CCS_RESULT_SUCCESS;
err_item:
	cJSON_Delete(item);
	return err;
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
		CCS_VALIDATE_ERR_GOTO(
			err, _ccs_json_add_string(item, "type", "none"),
			err_item);
		break;
	case CCS_DATA_TYPE_INT:
		CCS_VALIDATE_ERR_GOTO(
			err, _ccs_json_add_string(item, "type", "int"),
			err_item);
		CCS_VALIDATE_ERR_GOTO(
			err, _ccs_json_add_int(item, "value", datum.value.i),
			err_item);
		break;
	case CCS_DATA_TYPE_FLOAT:
		CCS_VALIDATE_ERR_GOTO(
			err, _ccs_json_add_string(item, "type", "float"),
			err_item);
		CCS_VALIDATE_ERR_GOTO(
			err, _ccs_json_add_float(item, "value", datum.value.f),
			err_item);
		break;
	case CCS_DATA_TYPE_BOOL:
		CCS_VALIDATE_ERR_GOTO(
			err, _ccs_json_add_string(item, "type", "bool"),
			err_item);
		CCS_VALIDATE_ERR_GOTO(
			err, _ccs_json_add_bool(item, "value", datum.value.i),
			err_item);
		break;
	case CCS_DATA_TYPE_STRING:
		CCS_VALIDATE_ERR_GOTO(
			err, _ccs_json_add_string(item, "type", "string"),
			err_item);
		CCS_VALIDATE_ERR_GOTO(
			err, _ccs_json_add_string(item, "value", datum.value.s),
			err_item);
		break;
	case CCS_DATA_TYPE_INACTIVE:
		CCS_VALIDATE_ERR_GOTO(
			err, _ccs_json_add_string(item, "type", "inactive"),
			err_item);
		break;
	case CCS_DATA_TYPE_OBJECT: {
		char hex[sizeof(ccs_object_t) * 2 + 1];
		_ccs_json_hex_encode_buf(
			&datum.value.o, sizeof(ccs_object_t), hex);
		CCS_VALIDATE_ERR_GOTO(
			err, _ccs_json_add_string(item, "type", "object"),
			err_item);
		CCS_VALIDATE_ERR_GOTO(
			err, _ccs_json_add_string(item, "value", hex),
			err_item);
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
	ccs_result_t err  = CCS_RESULT_SUCCESS;
	cJSON       *item = NULL;
	CCS_VALIDATE(_ccs_json_datum_to_cjson(datum, &item));
	CCS_REFUTE_ERR_GOTO(
		err, !cJSON_AddItemToObject(json, key, item),
		CCS_RESULT_ERROR_OUT_OF_MEMORY, err_item);
	return CCS_RESULT_SUCCESS;
err_item:
	cJSON_Delete(item);
	return err;
}

static inline ccs_result_t
_ccs_json_add_datum_to_array(cJSON *array, ccs_datum_t datum)
{
	ccs_result_t err  = CCS_RESULT_SUCCESS;
	cJSON       *item = NULL;
	CCS_VALIDATE(_ccs_json_datum_to_cjson(datum, &item));
	CCS_REFUTE_ERR_GOTO(
		err, !cJSON_AddItemToArray(array, item),
		CCS_RESULT_ERROR_OUT_OF_MEMORY, err_item);
	return CCS_RESULT_SUCCESS;
err_item:
	cJSON_Delete(item);
	return err;
}

static inline ccs_result_t
_ccs_json_get_datum(cJSON *item, ccs_datum_t *datum_ret)
{
	cJSON      *j_type  = NULL;
	cJSON      *j_value = NULL;
	const char *type_str;
	CCS_REFUTE(!cJSON_IsObject(item), CCS_RESULT_ERROR_INVALID_VALUE);
	j_type = cJSON_GetObjectItemCaseSensitive(item, "type");
	CCS_VALIDATE(_ccs_json_get_string(j_type, &type_str));
	if (!strcmp(type_str, "none")) {
		*datum_ret = ccs_none;
	} else if (!strcmp(type_str, "int")) {
		j_value = cJSON_GetObjectItemCaseSensitive(item, "value");
		CCS_REFUTE(
			!j_value || !cJSON_IsNumber(j_value),
			CCS_RESULT_ERROR_INVALID_VALUE);
		*datum_ret = ccs_int((ccs_int_t)j_value->valuedouble);
	} else if (!strcmp(type_str, "float")) {
		double fval;
		j_value = cJSON_GetObjectItemCaseSensitive(item, "value");
		CCS_REFUTE(!j_value, CCS_RESULT_ERROR_INVALID_VALUE);
		CCS_VALIDATE(_ccs_json_get_float(j_value, &fval));
		*datum_ret = ccs_float(fval);
	} else if (!strcmp(type_str, "bool")) {
		ccs_bool_t bval;
		j_value = cJSON_GetObjectItemCaseSensitive(item, "value");
		CCS_VALIDATE(_ccs_json_get_bool(j_value, &bval));
		*datum_ret = ccs_bool(bval);
	} else if (!strcmp(type_str, "string")) {
		const char *sval;
		j_value = cJSON_GetObjectItemCaseSensitive(item, "value");
		CCS_VALIDATE(_ccs_json_get_string(j_value, &sval));
		*datum_ret = ccs_string(sval);
	} else if (!strcmp(type_str, "inactive")) {
		*datum_ret = ccs_inactive;
	} else if (!strcmp(type_str, "object")) {
		const char  *oval;
		ccs_object_t obj;
		j_value = cJSON_GetObjectItemCaseSensitive(item, "value");
		CCS_VALIDATE(_ccs_json_get_string(j_value, &oval));
		CCS_REFUTE(
			strlen(oval) != sizeof(ccs_object_t) * 2,
			CCS_RESULT_ERROR_INVALID_VALUE);
		CCS_REFUTE(
			_ccs_json_hex_decode_buf(
				oval, sizeof(ccs_object_t) * 2, &obj),
			CCS_RESULT_ERROR_INVALID_VALUE);
		*datum_ret = ccs_object(obj);
	} else {
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unknown JSON datum type: %s", type_str);
	}
	return CCS_RESULT_SUCCESS;
}

#endif /* _CCONFIGSPACE_JSON_H */
