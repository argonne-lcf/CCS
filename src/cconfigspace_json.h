#ifndef _CCONFIGSPACE_JSON_H
#define _CCONFIGSPACE_JSON_H
#include "cjson/cJSON.h"
#include <string.h>

/*============================================================================
 * Hex encode/decode helpers
 *============================================================================*/

static inline char *
_ccs_json_hex_encode(const void *data, size_t len)
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
	for (size_t i = 0; i < *out_len; i++) {
		unsigned int byte;
		char tmp[3] = {hex_str[i * 2], hex_str[i * 2 + 1], '\0'};
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

#endif /* _CCONFIGSPACE_JSON_H */
