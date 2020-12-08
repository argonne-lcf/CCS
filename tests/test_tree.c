#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <string.h>

void test_create() {
	ccs_tree_t         tree;
	ccs_datum_t        value;
	ccs_object_type_t  type;
	size_t             arity;
	ccs_result_t       err;
	void              *user_data;

	err = ccs_create_tree(10, ccs_string("root"), (void *)0xdeadbeef, &tree);
	assert( err == CCS_SUCCESS );

	err = ccs_object_get_type(tree, &type);
	assert( err == CCS_SUCCESS );
	assert( CCS_TREE == type);

	err = ccs_tree_get_value(tree, &value);
	assert( err == CCS_SUCCESS );
	assert( CCS_STRING == value.type );
	assert( strcmp(value.value.s, "root") == 0);

	err = ccs_tree_get_user_data(tree, &user_data);
	assert( err == CCS_SUCCESS );
	assert( (void *)0xdeadbeef == user_data);

	err = ccs_tree_get_arity(tree, &arity);
	assert( err == CCS_SUCCESS );
	assert(10 == arity);

	err = ccs_release_object(tree);
	assert( err == CCS_SUCCESS );
}

void test_children() {
	ccs_tree_t   tree, child1, child2, child;
	ccs_result_t err;
	ccs_tree_t   children[3];
	ssize_t      indices[3];
	size_t       count;

	err = ccs_create_tree(10, ccs_string("root"), NULL, &tree);
	assert( err == CCS_SUCCESS );
	err = ccs_create_tree(5, ccs_string("child1"), NULL, &child1);
	assert( err == CCS_SUCCESS );
	err = ccs_create_tree(0, ccs_string("child2"), NULL, &child2);
	assert( err == CCS_SUCCESS );

	err = ccs_tree_get_children(tree, 0, NULL, NULL, &count);
	assert( err == CCS_SUCCESS );
	assert( count == 0);

	err = ccs_tree_set_child(tree, 5, child1);
	assert( err == CCS_SUCCESS );
	err = ccs_tree_set_child(tree, 3, child2);
	assert( err == CCS_SUCCESS );
	err = ccs_tree_set_child(tree, 12, child2);
	assert( err == -CCS_OUT_OF_BOUNDS );

	err = ccs_tree_get_children(tree, 0, NULL, NULL, &count);
	assert( err == CCS_SUCCESS );
	assert( count == 2);

	err = ccs_tree_get_children(tree, 3, indices, NULL, NULL);
	assert( err == CCS_SUCCESS );
	assert(  3 == indices[0] );
	assert(  5 == indices[1] );
	assert( -1 == indices[2] );

	err = ccs_tree_get_children(tree, 3, NULL, children, NULL);
	assert( err == CCS_SUCCESS );
	assert( child2 == children[0] );
	assert( child1 == children[1] );
	assert( NULL   == children[2] );

	err = ccs_tree_get_child(tree, 3, &child);
	assert( err == CCS_SUCCESS );
	assert( child2 == child);

	err = ccs_tree_get_child(tree, 4, &child);
	assert( err == CCS_SUCCESS );
	assert( NULL == child);

	err = ccs_tree_get_child(tree, 5, &child);
	assert( err == CCS_SUCCESS );
	assert( child1 == child);

	err = ccs_tree_get_child(tree, 12, &child);
	assert( err == -CCS_OUT_OF_BOUNDS );


	err = ccs_release_object(child1);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(child2);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(tree);
	assert( err == CCS_SUCCESS );

}
int main(int argc, char *argv[]) {
	ccs_init();
	test_create();
	test_children();
	return 0;
}
