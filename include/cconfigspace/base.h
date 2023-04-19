#ifndef _CCS_BASE_H
#define _CCS_BASE_H
#include <limits.h>
#include <math.h>
#include <string.h>

/**
 * @file base.h
 * Base definition of CCS objects and types.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A CCS floating point type
 */
typedef double   ccs_float_t;
/**
 * A CCS integer type
 */
typedef int64_t  ccs_int_t;
/**
 * A CCS boolean type
 */
typedef int32_t  ccs_bool_t;
/**
 * A CCS hashing value type
 */
typedef uint32_t ccs_hash_t;

/**
 * A structure representing a version of the CCS API.
 */
typedef struct {
	/** Revision version number */
	uint16_t revision;
	/** Patch version number */
	uint16_t patch;
	/** Minor version number */
	uint16_t minor;
	/** Major version number */
	uint16_t major;
} ccs_version_t;

/**
 * A variable containing the current version of the CCS library.
 */
extern const ccs_version_t ccs_version;

/**
 * A macro providing the true value of a ccs_bool_t.
 */
#define CCS_TRUE     ((ccs_bool_t)(1))
/**
 * A macro providing the false value of a ccs_bool_t.
 */
#define CCS_FALSE    ((ccs_bool_t)(0))

/**
 * A macro defining the maximum value of a ccs_int_t.
 */
#define CCS_INT_MAX  INT64_MAX
/**
 * A macro defining the minimum value of a ccs_int_t.
 */
#define CCS_INT_MIN  INT64_MIN
/**
 * A macro defining the (positive) infinity value of a ccs_float_t.
 */
#define CCS_INFINITY INFINITY

/**
 * An opaque type defining a CCS random generator.
 */
typedef struct _ccs_rng_s                 *ccs_rng_t;
/**
 * An opaque type defining a CCS distribution.
 */
typedef struct _ccs_distribution_s        *ccs_distribution_t;
/**
 * An opaque type defining a CCS parameter.
 */
typedef struct _ccs_parameter_s           *ccs_parameter_t;
/**
 * An opaque type defining a CCS expression.
 */
typedef struct _ccs_expression_s          *ccs_expression_t;
/**
 * An opaque type defining a CCS context.
 */
typedef struct _ccs_context_s             *ccs_context_t;
/**
 * An opaque type defining a CCS configuration space.
 */
typedef struct _ccs_configuration_space_s *ccs_configuration_space_t;
/**
 * An opaque type defining a CCS binding.
 */
typedef struct _ccs_binding_s             *ccs_binding_t;
/**
 * An opaque type defining a CCS configuration.
 */
typedef struct _ccs_configuration_s       *ccs_configuration_t;
/**
 * An opaque type defining a CCS features space.
 */
typedef struct _ccs_features_space_s      *ccs_features_space_t;
/**
 * An opaque type defining a CCS features.
 */
typedef struct _ccs_features_s            *ccs_features_t;
/**
 * An opaque type defining a CCS objective space.
 */
typedef struct _ccs_objective_space_s     *ccs_objective_space_t;
/**
 * An opaque type defining a CCS evaluation.
 */
typedef struct _ccs_evaluation_s          *ccs_evaluation_t;
/**
 * An opaque type defining a CCS features evaluation.
 */
typedef struct _ccs_features_evaluation_s *ccs_features_evaluation_t;
/**
 * An opaque type defining a CCS tuner.
 */
typedef struct _ccs_tuner_s               *ccs_tuner_t;
/**
 * An opaque type defining a CCS features tuner.
 */
typedef struct _ccs_features_tuner_s      *ccs_features_tuner_t;
/**
 * An opaque type defining a CCS key-value store.
 */
typedef struct _ccs_map_s                 *ccs_map_t;
/**
 * An opaque type defining a CCS error statck.
 */
typedef struct _ccs_error_stack_s         *ccs_error_stack_t;
/**
 * An opaque type defining a CCS tree.
 */
typedef struct _ccs_tree_s                *ccs_tree_t;
/**
 * An opaque type defining a CCS tree space.
 */
typedef struct _ccs_tree_space_s          *ccs_tree_space_t;
/**
 * An opaque type defining a CCS tree space configuration.
 */
typedef struct _ccs_tree_configuration_s  *ccs_tree_configuration_t;
/**
 * An opaque type defining a CCS tree evaluation.
 */
typedef struct _ccs_tree_evaluation_s     *ccs_tree_evaluation_t;
/**
 * An opaque type defining a CCS tree tuner.
 */
typedef struct _ccs_tree_tuner_s          *ccs_tree_tuner_t;

/**
 * The different possible return codes of a CCS function.
 * Error codes are returned negated.
 */
enum ccs_error_e {
	/** Guard */
	CCS_ERROR_MAX              = 2,
	/** Try again */
	CCS_AGAIN                  = 1,
	/** Success */
	CCS_SUCCESS                = 0,
	/** Not a CCS object or not initialized */
	CCS_INVALID_OBJECT         = -1,
	/** Parameter has an invalid value */
	CCS_INVALID_VALUE          = -2,
	/** The data type is invalid */
	CCS_INVALID_TYPE           = -3,
	/** The provided scale is invalid */
	CCS_INVALID_SCALE          = -4,
	/** The provided distribution is invalid */
	CCS_INVALID_DISTRIBUTION   = -5,
	/** The provided expression is invalid */
	CCS_INVALID_EXPRESSION     = -6,
	/** The provided parameter is invalid */
	CCS_INVALID_PARAMETER      = -7,
	/** The provided configuration is invalid */
	CCS_INVALID_CONFIGURATION  = -8,
	/** The parameter name is invalid */
	CCS_INVALID_NAME           = -9,
	/** The condition is invalid (unused) */
	CCS_INVALID_CONDITION      = -10,
	/** The provided tuner is invalid */
	CCS_INVALID_TUNER          = -11,
	/** The constraint graph would be invalid */
	CCS_INVALID_GRAPH          = -12,
	/** The type is not comparable (unused) */
	CCS_TYPE_NOT_COMPARABLE    = -13,
	/** The bounds are invalid (unused) */
	CCS_INVALID_BOUNDS         = -14,
	/** The index is out of bounds */
	CCS_OUT_OF_BOUNDS          = -15,
	/** Could not gather enough samples */
	CCS_SAMPLING_UNSUCCESSFUL  = -16,
	/** An allocation failed due to lack of available memory */
	CCS_OUT_OF_MEMORY          = -17,
	/** The object does not support this operation */
	CCS_UNSUPPORTED_OPERATION  = -18,
	/** The provided evaluation is invalid */
	CCS_INVALID_EVALUATION     = -19,
	/** The provided features is invalid */
	CCS_INVALID_FEATURES       = -20,
	/** The provided features tuner is invalid */
	CCS_INVALID_FEATURES_TUNER = -21,
	/** The provided file path is invalid */
	CCS_INVALID_FILE_PATH      = -22,
	/** The provided buffer or file is too short */
	CCS_NOT_ENOUGH_DATA        = -23,
	/** The handle was a duplicate */
	CCS_HANDLE_DUPLICATE       = -24,
	/** The handle was not found */
	CCS_INVALID_HANDLE         = -25,
	/** A system error occured */
	CCS_SYSTEM_ERROR           = -26,
	/** External error occured (binding?) */
	CCS_EXTERNAL_ERROR         = -27,
	/** The provided tree is invalid */
	CCS_INVALID_TREE           = -28,
	/** The provided tree space is invalid */
	CCS_INVALID_TREE_SPACE     = -29,
	/** The provided tree tuner is invalid */
	CCS_INVALID_TREE_TUNER     = -30,
	/** Guard */
	CCS_ERROR_MIN              = -31,
	/** Try forcing 32 bits value for bindings */
	CCS_ERROR_FORCE_32BIT      = INT32_MAX
};

/**
 * A commodity type to represent CCS errors and returned by most functione.
 */
typedef enum ccs_error_e ccs_error_t;

/**
 * The result type used for evaluations.
 */
typedef int32_t          ccs_evaluation_result_t;

/**
 * CCS object types.
 */
enum ccs_object_type_e {
	CCS_OBJECT_TYPE_RNG,                 /*!< A random number generator */
	CCS_OBJECT_TYPE_DISTRIBUTION,        /*!< A numerical distribution */
	CCS_OBJECT_TYPE_PARAMETER,           /*!< A parameter */
	CCS_OBJECT_TYPE_EXPRESSION,          /*!< An arithmetic expression */
	CCS_OBJECT_TYPE_CONFIGURATION_SPACE, /*!< A configuration space */
	CCS_OBJECT_TYPE_CONFIGURATION,       /*!< A configuration */
	CCS_OBJECT_TYPE_OBJECTIVE_SPACE,     /*!< An objective space */
	CCS_OBJECT_TYPE_EVALUATION,     /*!< An evaluation of a configuration */
	CCS_OBJECT_TYPE_TUNER,          /*!< A tuner */
	CCS_OBJECT_TYPE_FEATURES_SPACE, /*!< A features space */
	CCS_OBJECT_TYPE_FEATURES,       /*!< A features */
	CCS_OBJECT_TYPE_FEATURES_EVALUATION, /*!< An evaluation of a configuration given specific features */
	CCS_OBJECT_TYPE_FEATURES_TUNER,     /*!< A features aware tuner */
	CCS_OBJECT_TYPE_MAP,                /*!< A key value store */
	CCS_OBJECT_TYPE_ERROR_STACK,        /*!< An error stack */
	CCS_OBJECT_TYPE_TREE,               /*!< A tree structure */
	CCS_OBJECT_TYPE_TREE_SPACE,         /*!< A tree space */
	CCS_OBJECT_TYPE_TREE_CONFIGURATION, /*!< A configuration on a tree space */
	CCS_OBJECT_TYPE_TREE_EVALUATION, /*!< An evaluation of a tree configuration */
	CCS_OBJECT_TYPE_TREE_TUNER, /*!< A tree tuner */
	CCS_OBJECT_TYPE_MAX,        /*!< Guard */
	/** Try forcing 32 bits value for bindings */
	CCS_OBJECT_TYPE_FORCE_32BIT = INT32_MAX
};

/**
 * A commodity type to represent CCS object types.
 */
typedef enum ccs_object_type_e ccs_object_type_t;

/**
 * CCS supported data types.
 */
enum ccs_data_type_e {
	CCS_NONE,          /*!< An empty value */
	CCS_INTEGER,       /*!< A ccs_int_t */
	CCS_FLOAT,         /*!< A ccs_float_t */
	CCS_BOOLEAN,       /*!< A ccs_bool_t */
	CCS_STRING,        /*!< A pointer to a NULL terminated string */
	CCS_INACTIVE,      /*!< An inactive value */
	CCS_OBJECT,        /*!< A CCS object */
	CCS_DATA_TYPE_MAX, /*!< Guard */
	/** Try forcing 32 bits value for bindings */
	CCS_DATA_TYPE_FORCE_32BIT = INT32_MAX
};

/**
 * A commodity type to represent CCS data types.
 */
typedef enum ccs_data_type_e ccs_data_type_t;

/**
 * Flags that can be attached to a CCS datum.
 */
enum ccs_datum_flag_e {
	/** Empty default flags */
	CCS_FLAG_DEFAULT           = 0,
	/**
	 * The value given to CCS is a pointer and is not guaranteed to stay
	 * allocated
	 */
	CCS_FLAG_TRANSIENT         = (1 << 0),
	/**
	 * The value returned by CCS is a pointer and is not associated to a
	 * CCS object and needs to be freed by the user (unused).
	 */
	CCS_FLAG_UNPOOLED          = (1 << 1),
	/**
	 * The object handle is just an identifier.
	 */
	CCS_FLAG_ID                = (1 << 2),
	/** Try forcing 32 bits value for bindings */
	CCS_DATUM_FLAG_FORCE_32BIT = INT32_MAX
};

/**
 * A commodity type to represent CCS datum flags.
 */
typedef enum ccs_datum_flag_e ccs_datum_flag_t;

/**
 * A type representing the combination of flags that can be attached to a CCS
 * datum.
 */
typedef uint32_t              ccs_datum_flags_t;

/**
 * The subset of CCS data types that represent numerical data.
 */
enum ccs_numeric_type_e {
	CCS_NUM_INTEGER = CCS_INTEGER, /*!< A ccs_int_t */
	CCS_NUM_FLOAT   = CCS_FLOAT,   /*!< A ccs_float_t */
	CCS_NUM_TYPE_MAX,              /*!< Guard */
	/** Try forcing 32 bits value for bindings */
	CCS_NUM_TYPE_FORCE_32BIT = INT32_MAX
};

/**
 * A commodity type to represent CCS numeric types.
 */
typedef enum ccs_numeric_type_e ccs_numeric_type_t;

/**
 * A type representing a generic CCS object.
 */
typedef void                   *ccs_object_t;

/**
 * A union that can contain either a ccs_int_t or a ccs_float_t.
 */
union ccs_numeric_u {
	ccs_float_t f; /*!< The floating point value of the union */
	ccs_int_t   i; /*!< The integer value of the union */
#ifdef __cplusplus
	ccs_numeric_u()
		: i(0L)
	{
	}
	ccs_numeric_u(float v)
		: f((ccs_float_t)v)
	{
	}
	ccs_numeric_u(int v)
		: i((ccs_int_t)v)
	{
	}
	ccs_numeric_u(ccs_int_t v)
		: i(v)
	{
	}
	ccs_numeric_u(ccs_float_t v)
		: f(v)
	{
	}
#endif
};

/**
 * A commodity type to represent CCS numeric values.
 */
typedef union ccs_numeric_u ccs_numeric_t;

#ifdef __cplusplus
#define CCSF(v) v
#define CCSI(v) v
#else
/**
 * A macro casting a value to a CCS floating point numeric.
 */
#define CCSF(v) ((ccs_numeric_t){.f = v})
/**
 * A macro casting a value to a CCS integer numeric.
 */
#define CCSI(v) ((ccs_numeric_t){.i = v})
#endif

/**
 * A union that represent a CCS datum value.
 */
union ccs_value_u {
	ccs_float_t  f; /*!< The floating point value of the union */
	ccs_int_t    i; /*!< The integer value of the union */
	const char  *s; /*!< The string value of the union */
	ccs_object_t o; /*!< The CCS object value of the union */
#ifdef __cplusplus
	ccs_value_u()
		: i(0L)
	{
	}
	ccs_value_u(float v)
		: f((ccs_float_t)v)
	{
	}
	ccs_value_u(int v)
		: i((ccs_int_t)v)
	{
	}
	ccs_value_u(ccs_float_t v)
		: f(v)
	{
	}
	ccs_value_u(ccs_int_t v)
		: i(v)
	{
	}
	ccs_value_u(char *v)
		: s(v)
	{
	}
	ccs_value_u(ccs_object_t v)
		: o(v)
	{
	}
#endif
};

/**
 * A commodity type to represent a CCS datum value.
 */
typedef union ccs_value_u ccs_value_t;

/**
 * A Structure containing a CCS datum.
 */
struct ccs_datum_s {
	ccs_value_t       value; /*!< The value of the datum */
	ccs_data_type_t   type;  /*!< The type of the datum */
	ccs_datum_flags_t flags; /*!< The flags attached to the datum */
};

/**
 * A commodity type to represent a CCS datum.
 */
typedef struct ccs_datum_s ccs_datum_t;

/**
 * A helper function to construct a datum containing a boolean value.
 * @param[in] v #CCS_TRUE or #CCS_FALSE
 * @return a CCS datum value
 */
static inline ccs_datum_t
ccs_bool(ccs_bool_t v)
{
	ccs_datum_t d;
	d.type    = CCS_BOOLEAN;
	d.value.i = v;
	d.flags   = CCS_FLAG_DEFAULT;
	return d;
}

/**
 * A helper function to construct a datum containing a floating point value.
 * @param[in] v a floating point value.
 * @return a CCS datum value
 */
static inline ccs_datum_t
ccs_float(ccs_float_t v)
{
	ccs_datum_t d;
	d.type    = CCS_FLOAT;
	d.value.f = v;
	d.flags   = CCS_FLAG_DEFAULT;
	return d;
}

/**
 * A helper function to construct a datum containing an integer value.
 * @param[in] v a signed integer.
 * @return a CCS datum value
 */
static inline ccs_datum_t
ccs_int(ccs_int_t v)
{
	ccs_datum_t d;
	d.type    = CCS_INTEGER;
	d.value.i = v;
	d.flags   = CCS_FLAG_DEFAULT;
	return d;
}

/**
 * A helper function to construct a datum containing a CCS object value.
 * @param[in] v a CCS object.
 * @return a CCS datum value
 */
static inline ccs_datum_t
ccs_object(ccs_object_t v)
{
	ccs_datum_t d;
	d.type    = CCS_OBJECT;
	d.value.o = v;
	d.flags   = CCS_FLAG_DEFAULT;
	return d;
}

/**
 * A helper function to construct a datum containing a string value.
 * @param[in] v a pointer to a NULL terminated string.
 * @return a CCS datum value
 */
static inline ccs_datum_t
ccs_string(const char *v)
{
	ccs_datum_t d;
	d.type    = CCS_STRING;
	d.value.s = v;
	d.flags   = CCS_FLAG_DEFAULT;
	return d;
}

/**
 * A helper function providing a strict ordering of datum.
 * Flags are not taken into account.
 * @param[in] a the first datum
 * @param[in] b the second datum
 * @return -1, 0, or 1 if the first datum is found to be respectively lesser
 *         than, equal, or greater than the second datum
 */
static inline int
ccs_datum_cmp(ccs_datum_t a, ccs_datum_t b)
{
	if (a.type < b.type) {
		return -1;
	} else if (a.type > b.type) {
		return 1;
	} else {
		switch (a.type) {
		case CCS_STRING:
			if (a.value.s == b.value.s)
				return 0;
			else if (!a.value.s)
				return -1;
			else if (!b.value.s)
				return 1;
			else
				return strcmp(a.value.s, b.value.s);
			break;
		case CCS_INTEGER:
			return a.value.i < b.value.i ? -1 :
			       a.value.i > b.value.i ? 1 :
						       0;
			break;
		case CCS_FLOAT:
			return a.value.f < b.value.f ? -1 :
			       a.value.f > b.value.f ? 1 :
						       0;
			break;
		case CCS_NONE:
		case CCS_INACTIVE:
			return 0;
			break;
		default:
			return memcmp(
				&(a.value), &(b.value), sizeof(ccs_value_t));
		}
	}
}

/**
 * A variable containing an empty datum.
 */
extern const ccs_datum_t ccs_none;
/**
 * A variable containing an inactive datum.
 */
extern const ccs_datum_t ccs_inactive;
/**
 * A variable containing a boolean true datum.
 */
extern const ccs_datum_t ccs_true;
/**
 * A variable containing a boolean false datum.
 */
extern const ccs_datum_t ccs_false;

/**
 * A macro defining the none datum value.
 */
#define CCS_NONE_VAL                                                           \
	{                                                                      \
		{0}, CCS_NONE, CCS_FLAG_DEFAULT                                \
	}
/**
 * A macro defining the inactive datum value.
 */
#define CCS_INACTIVE_VAL                                                       \
	{                                                                      \
		{0}, CCS_INACTIVE, CCS_FLAG_DEFAULT                            \
	}
#ifdef __cplusplus
#define CCS_TRUE_VAL                                                           \
	{                                                                      \
		{(ccs_int_t)CCS_TRUE}, CCS_BOOLEAN, CCS_FLAG_DEFAULT           \
	}
#define CCS_FALSE_VAL                                                          \
	{                                                                      \
		{(ccs_int_t)CCS_FALSE}, CCS_BOOLEAN, CCS_FLAG_DEFAULT          \
	}
#else
/**
 * A macro defining the true boolean datum value.
 */
#define CCS_TRUE_VAL                                                           \
	{                                                                      \
		{.i = CCS_TRUE}, CCS_BOOLEAN, CCS_FLAG_DEFAULT                 \
	}
/**
 * A macro defining the false boolean datum value.
 */
#define CCS_FALSE_VAL                                                          \
	{                                                                      \
		{.i = CCS_FALSE}, CCS_BOOLEAN, CCS_FLAG_DEFAULT                \
	}
#endif

/**
 * The library initialization function. Should be called before any operation
 * using the library are performed.
 * @return #CCS_SUCCESS
 */
extern ccs_error_t
ccs_init();

/**
 * The library deinitialization function. Should be called after all opertaions
 * using the library are performed.
 * @return #CCS_SUCCESS
 */
extern ccs_error_t
ccs_fini();

/**
 * Return the string corresponding to the provided CCS error.
 * @param[in] error the CCS error
 * @param[out] name a pointer to a variable that will contain the string
 *             representation of the error name.
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p name is NULL or if \p error is not a valid
 *                             CCS error code
 */
extern ccs_error_t
ccs_get_error_name(ccs_error_t error, const char **name);

/**
 * Query the library API version.
 * @return the library API version
 */
extern ccs_version_t
ccs_get_version();

/**
 * Retain a CCS object, incrementing the internal reference counting.
 * @param[in,out] object a CCS object
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if the object is found to be invalid
 */
extern ccs_error_t
ccs_retain_object(ccs_object_t object);

/**
 * Release a CCS object, decrementing the internal reference counting.
 * When the internsal reference count reaches zero, the destruction callbacks
 * are called and the object is freed
 * @param[in,out] object a CCS object
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if the object is found to be invalid
 * @return an error code given by the object destructor
 */
extern ccs_error_t
ccs_release_object(ccs_object_t object);

/**
 * Get a CCS object type.
 * @param[in] object a CCS object
 * @param[out] type_ret a pointer to a ccs_object_type_t variable that will
 *                      contain the type
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if the object is found to be invalid
 * @return #CCS_INVALID_VALUE if type_ret is NULL
 */
extern ccs_error_t
ccs_object_get_type(ccs_object_t object, ccs_object_type_t *type_ret);

/**
 * Get an object internal reference counting.
 * @param[in] object a CCS object
 * @param[out] refcount_ret a pointer to a int32_t variable that will contain
 *                          the refcount
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p object is found to be invalid
 * @return #CCS_INVALID_VALUE if \p refcount_ret is NULL
 */
extern ccs_error_t
ccs_object_get_refcount(ccs_object_t object, int32_t *refcount_ret);

/**
 * The type of CCS object destruction callbacks.
 */
typedef void (
	*ccs_object_release_callback_t)(ccs_object_t object, void *user_data);

/**
 * Attach a destruction callback to a CCS object.
 * Destruction callbacks are called in reverse order they were attached.
 * @param[in,out] object a CCS object
 * @param[in] callback the destruction callback to attach
 * @param[in] user_data an optional pointer that will be passed to the callback
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p object is found to be invalid
 * @return #CCS_INVALID_VALUE if \p callback is NULL
 */
extern ccs_error_t
ccs_object_set_destroy_callback(
	ccs_object_t                  object,
	ccs_object_release_callback_t callback,
	void                         *user_data);

/**
 * Set the associated `user_data` pointer of a CCS object.
 * @param[in] object a CCS object
 * @param[in] user_data a pointer to the user data to attach to this object
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p object is found to be invalid
 */
extern ccs_error_t
ccs_object_set_user_data(ccs_object_t object, void *user_data);

/**
 * Get the associated `user_data` pointer of a CCS object.
 * @param[in] object a CCS object
 * @param[out] user_data_ret a pointer to a `void *` variable that will contain
 *                           the value of the `user_data`
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p object is found to be invalid
 * @return #CCS_INVALID_VALUE if \p user_data_ret is NULL
 */
extern ccs_error_t
ccs_object_get_user_data(ccs_object_t object, void **user_data_ret);

/**
 * The type of CCS object serialization callbacks.
 * This callback is used to serialize object information that CCS is not
 * aware of, the most glaring example being the object associated user_data.
 * @param[in] object a CCS object
 * @param[in] serialize_data_size the size of the memory pointed to by
 *                                \p serialize_data, can be zero when
 *                                querying the size
 * @param[out] serialize_data a pointer to a memory area of size \p
 *                            serialize_data_size. Can be NULL when \p
 *                            serialize_data_size is zero
 * @param[out] serialize_data_size_ret a pointer to the variable that will
 *                                     contain the required user size to
 *                                     serialize the object.
 * @param[in] callback_user_data the pointer provided when the callback
 *                               was attached.
 * @return #CCS_SUCCESS on success
 * @return an error code on error
 */
typedef ccs_error_t (*ccs_object_serialize_callback_t)(
	ccs_object_t object,
	size_t       serialize_data_size,
	void        *serialize_data,
	size_t      *serialize_data_size_ret,
	void        *callback_user_data);

/**
 * Set the object serialization callback.
 * @param[in,out] object a CCS object
 * @param[in] callback the serialization callback to attach, eventually
 *                     replacing the previous callback. Can be NULL in
 *                     which case the previous callback is removed, if it
 *                     exists
 * @param[in] user_data an optional pointer that will be passed to the
 *                      callback
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p object is found to be invalid
 * @return #CCS_INVALID_VALUE if \p callback is NULL
 */
extern ccs_error_t
ccs_object_set_serialize_callback(
	ccs_object_t                    object,
	ccs_object_serialize_callback_t callback,
	void                           *user_data);

/**
 * The different serialization formats supported by CCS.
 */
enum ccs_serialize_format_e {
	/** A binary format that should be compact and performant. */
	CCS_SERIALIZE_FORMAT_BINARY,
	/** Guard */
	CCS_SERIALIZE_FORMAT_MAX,
	/** Try forcing 32 bits value for bindings */
	CCS_SERIALIZE_FORMAT_FORCE_32BIT = INT32_MAX
};
/**
 * A commodity type to represent CCS serialization formats.
 */
typedef enum ccs_serialize_format_e ccs_serialize_format_t;

/**
 * The different serialization operations supported by CCS.
 */
enum ccs_serialize_operation_e {
	/** Query the memory footprint of the serialized object */
	CCS_SERIALIZE_OPERATION_SIZE,
	/** Serialize the object in a user provided memory buffer */
	CCS_SERIALIZE_OPERATION_MEMORY,
	/** Serialize the object in a file at the given path */
	CCS_SERIALIZE_OPERATION_FILE,
	/** Serialize the ojbect in the given file descriptor */
	CCS_SERIALIZE_OPERATION_FILE_DESCRIPTOR,
	/** Guard */
	CCS_SERIALIZE_OPERATION_MAX,
	/** Try forcing 32 bits value for bindings */
	CCS_SERIALIZE_OPERATION_FORCE_32BIT = INT32_MAX
};
/**
 * A commodity type to represent CCS serializaiton operations.
 */
typedef enum ccs_serialize_operation_e ccs_serialize_operation_t;

/**
 * The different serialization options.
 */
enum ccs_serialize_option_e {
	/** Option list terminator */
	CCS_SERIALIZE_OPTION_END = 0,
	/** The file descriptor operation is non-blocking. The next parameter is
	 *  a pointer to a void * variable (initialized to NULL) that will hold
	 *  the state of the serialization in order to restart. The function
	 *  performing the operation will return #CCS_AGAIN if the operation has
	 *  not completed. The state is managed internally. */
	CCS_SERIALIZE_OPTION_NON_BLOCKING,
	/** The next parameters are a serialization callback and it's user_data.
	 *  This callback will be called for all objects that have user_data set
	 *  and have not a serialization callback set via
	 *  ccs_object_set_serialize_callback */
	CCS_SERIALIZE_OPTION_CALLBACK,
	/** Guard */
	CCS_SERIALIZE_OPTION_MAX,
	/** Try forcing 32 bits value for bindings */
	CCS_SERIALIZE_OPTION_FORCE_32BIT = INT32_MAX
};
/**
 * A commodity type to represent CCS serialization options.
 */
typedef enum ccs_serialize_option_e ccs_serialize_option_t;

/**
 * A commodity type to represent CCS deserializaiton callbacks.
 */
typedef ccs_error_t (*ccs_object_deserialize_callback_t)(
	ccs_object_t object,
	size_t       serialize_data_size,
	const char  *serialize_data,
	void        *callback_user_data);

/**
 * The different deserialization options.
 */
enum ccs_deserialize_option_e {
	/** Option list terminator */
	CCS_DESERIALIZE_OPTION_END = 0,
	/** The next parameter is a ccs_handle_map_t object that must contain
	 *  the mappings required to deserialize an object (usually bindings or
	 *  expressions). I given, will also add a mapping between the object
	 *  original handle and its current handle. */
	CCS_DESERIALIZE_OPTION_HANDLE_MAP,
	/** The next parameter is a pointer to a ccs object vector struct, for
	 *  user defined tuners */
	CCS_DESERIALIZE_OPTION_VECTOR,
	/** The next parameter is a pointer to a ccs object internal data, for
	 *  user defined tuners */
	CCS_DESERIALIZE_OPTION_DATA,
	/** The file descriptor operation is non-blocking. The next parameter is
	 *  a pointer to a void * variable (initialized to NULL) that will hold
	 *  the state of the serialization in order to restart. The function
	 *  performing the operation will return #CCS_AGAIN if the operation has
	 *  not completed. The state is managed internally. */
	CCS_DESERIALIZE_OPTION_NON_BLOCKING,
	/** The next parameters are a deserialization callback and it's user_data.
	 *  This callback will be called for all objects that had their
	 *  user_data serialized. If no such callback is provided the object's
	 *  user_data value will not be set. */
	CCS_DESERIALIZE_CALLBACK,
	/** Guard */
	CCS_DESERIALIZE_OPTION_MAX,
	/** Try forcing 32 bits value for bindings */
	CCS_DESERIALIZE_OPTION_FORCE_32BIT = INT32_MAX
};
/**
 * A commodity type to represent CCS deserialization options.
 */
typedef enum ccs_deserialize_option_e ccs_deserialize_option_t;

/**
 * Perform a serialization operation on a CCS object.
 * @param[in] object a CCS object
 * @param[in] format the requested serialization format
 * @param[in] operation the requested serialization operation
 * @param[in,out] ... list of parameters that depend on the selected \p
 *                    operation, followed by a CCS_SERIALIZE_OPTION_END
 *                    terminated list of options
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p object is found to be invalid
 * @return #CCS_INVALID_VALUE if parameters and option combination are
 *                             unsupported
 * @return #CCS_OUT_OF_MEMORY if required memory could not be allocated
 * @return #CCS_NOT_ENOUGH_DATA in case where the provided buffer is too small
 *                               for the requested operation
 */
extern ccs_error_t
ccs_object_serialize(
	ccs_object_t              object,
	ccs_serialize_format_t    format,
	ccs_serialize_operation_t operation,
	...);

/**
 * Perform a deserialization operation and returns a new CCS object.
 * @param[out] object_ret a pointer to the variable that will hold the
 *                        newly created CCS object
 * @param[in] format the requested serialization format
 * @param[in] operation the requested serialization operation
 * @param[in,out] ... list of parameters that depend on the selected \p
 *                    operation, followed by a CCS_SERIALIZE_OPTION_END
 *                    terminated list of options
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p object is found to be invalid
 * @return #CCS_INVALID_VALUE if parameters and option combination are
 *                             unsupported
 * @return #CCS_OUT_OF_MEMORY if required memory could not be allocated
 * @return #CCS_NOT_ENOUGH_DATA in case where the provided buffer is too small
 *                               for the requested operation
 */
extern ccs_error_t
ccs_object_deserialize(
	ccs_object_t             *object_ret,
	ccs_serialize_format_t    format,
	ccs_serialize_operation_t operation,
	...);

#ifdef __cplusplus
}
#endif

#endif //_CCS_BASE_H
