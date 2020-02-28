#include <cconfigspace.h>

struct _ccs_object_ops_s {
	ccs_error_t (*del)(ccs_object_t object);
};

typedef struct _ccs_object_ops_s _ccs_object_ops_t;

struct _ccs_object_internal_s {
	ccs_object_type_t  type;
	int32_t            refcount;
	_ccs_object_ops_t *ops;
	void              *data; 
};

typedef struct _ccs_object_internal_s _ccs_object_internal_t;
