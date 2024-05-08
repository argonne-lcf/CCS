#ifndef _SEARCH_SPACE_INTERNAL_H
#define _SEARCH_SPACE_INTERNAL_H

struct _ccs_search_space_data_s;
typedef struct _ccs_search_space_data_s _ccs_search_space_data_t;

struct _ccs_search_space_s {
	_ccs_object_internal_t    obj;
	_ccs_search_space_data_t *data;
};

#endif //_SEARCH_SPACE_INTERNAL_H
