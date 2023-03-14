#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <string.h>
#include <Python.h>

ccs_hyperparameter_t create_numerical(const char * name, double lower, double upper) {
	ccs_hyperparameter_t hyperparameter;
	ccs_error_t         err;
	err = ccs_create_numerical_hyperparameter(name, CCS_NUM_FLOAT,
	                                          CCSF(lower), CCSF(upper),
	                                          CCSF(0.0), CCSF(0),
	                                          &hyperparameter);
	assert( err == CCS_SUCCESS );
	return hyperparameter;
}

void create_problem(ccs_configuration_space_t *cs, ccs_objective_space_t *os) {
	ccs_hyperparameter_t      hyperparameter1, hyperparameter2;
	ccs_hyperparameter_t      hyperparameter3;
	ccs_configuration_space_t cspace;
	ccs_objective_space_t     ospace;
	ccs_expression_t          expression;
	ccs_error_t              err;

	hyperparameter1 = create_numerical("x", -5.0, 5.0);
	hyperparameter2 = create_numerical("y", -5.0, 5.0);

	err = ccs_create_configuration_space("2dplane", &cspace);
	assert( err == CCS_SUCCESS );
	err = ccs_configuration_space_add_hyperparameter(cspace, hyperparameter1, NULL);
	assert( err == CCS_SUCCESS );
	err = ccs_configuration_space_add_hyperparameter(cspace, hyperparameter2, NULL);
	assert( err == CCS_SUCCESS );

	hyperparameter3 = create_numerical("z", -CCS_INFINITY, CCS_INFINITY);
	err = ccs_create_variable(hyperparameter3, &expression);
	assert( err == CCS_SUCCESS );

	err = ccs_create_objective_space("height", &ospace);
	assert( err == CCS_SUCCESS );
	err = ccs_objective_space_add_hyperparameter(ospace, hyperparameter3);
	assert( err == CCS_SUCCESS );
	err = ccs_objective_space_add_objective(ospace, expression, CCS_MINIMIZE);
	assert( err == CCS_SUCCESS );

	err = ccs_release_object(hyperparameter1);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(hyperparameter2);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(hyperparameter3);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(expression);
	assert( err == CCS_SUCCESS );

	*cs = cspace;
	*os = ospace;
}

void test_tuner(ccs_tuner_t tuner, ccs_objective_space_t ospace) {
	ccs_error_t         err;

	for (size_t i = 0; i < 100; i++) {
		ccs_datum_t         values[2], res;
		ccs_configuration_t configuration;
		ccs_evaluation_t    evaluation;
		err = ccs_tuner_ask(tuner, 1, &configuration, NULL);
		assert( err == CCS_SUCCESS );
		err = ccs_configuration_get_values(configuration, 2, values, NULL);
		assert( err == CCS_SUCCESS );
		res = ccs_float((values[0].value.f - 1)*(values[0].value.f - 1) +
		                (values[1].value.f - 2)*(values[1].value.f - 2));
		ccs_create_evaluation(ospace, configuration, CCS_SUCCESS, 1, &res, &evaluation);
		err = ccs_tuner_tell(tuner, 1, &evaluation);
		assert( err == CCS_SUCCESS );
		err = ccs_release_object(configuration);
		assert( err == CCS_SUCCESS );
		err = ccs_release_object(evaluation);
		assert( err == CCS_SUCCESS );
	}

	size_t           count;
	ccs_evaluation_t history[100];
	ccs_datum_t      min = ccs_float(INFINITY);
	err = ccs_tuner_get_history(tuner, 100, history, &count);
	assert( err == CCS_SUCCESS );
	assert( count == 100 );

	for (size_t i = 0; i < 100; i++) {
		ccs_datum_t      res;
		err = ccs_evaluation_get_objective_value(history[i], 0, &res);
		assert( err == CCS_SUCCESS );
		if (res.value.f < min.value.f)
			min.value.f = res.value.f;
	}

	ccs_evaluation_t evaluation;
	ccs_datum_t      res;
	err = ccs_tuner_get_optimums(tuner, 1, &evaluation, NULL);
	assert( err == CCS_SUCCESS );
	err = ccs_evaluation_get_objective_value(evaluation, 0, &res);
	assert( res.value.f == min.value.f );
}

void test() {
	ccs_tuner_t               t;
	ccs_configuration_space_t cs;
	ccs_objective_space_t     os;
	ccs_error_t              err;

	create_problem(&cs, &os);

	PyObject *pName, *pModule, *pFunc;
	PyObject *pArgs, *pValue, *pHandle, *pAddr;
#if PY_VERSION_HEX <= 0x030b0000
	Py_SetProgramName(L"test_python");
	Py_Initialize();
#else
	PyStatus status;
	PyConfig config;
	PyConfig_InitPythonConfig(&config);
	config.configure_c_stdio = 0;
	config.install_signal_handlers = 0;
	config.parse_argv = 0;
	config.pathconfig_warnings = 0;
	status = PyConfig_SetString(&config, &config.program_name, L"test_python");
	if (PyStatus_Exception(status)) {
		goto exception;
	}
	status = Py_InitializeFromConfig(&config);
	if (PyStatus_Exception(status)) {
		goto exception;
	}
	PyConfig_Clear(&config);
#endif
	PyRun_SimpleString("import sys\n"
	                   "sys.path.insert(1, '.')\n"
	                   "sys.path.insert(1, '..')\n");
	pName = PyUnicode_DecodeFSDefault("test_python");
	pModule = PyImport_Import(pName);
	Py_DECREF(pName);

	if (pModule != NULL) {
		pFunc = PyObject_GetAttrString(pModule, "create_test_tuner");
		pArgs = PyTuple_New(2);
		pValue = PyLong_FromVoidPtr(cs);
		PyTuple_SetItem(pArgs, 0, pValue);
		pValue = PyLong_FromVoidPtr(os);
		PyTuple_SetItem(pArgs, 1, pValue);
		pValue = PyObject_CallObject(pFunc, pArgs);
		Py_DECREF(pArgs);
		Py_DECREF(pFunc);
		pHandle = PyObject_GetAttrString(pValue, "handle");
		pAddr = PyObject_GetAttrString(pHandle, "value");
		t = (ccs_tuner_t)PyLong_AsVoidPtr(pAddr);
		err = ccs_retain_object(t);
		Py_DECREF(pHandle);
		Py_DECREF(pAddr);
		Py_DECREF(pValue);
		assert( err == CCS_SUCCESS );
		test_tuner(t, os);
		err = ccs_release_object(t);
		assert( err == CCS_SUCCESS );
	} else {
		if (PyErr_Occurred())
			PyErr_Print();
		assert(0);
	}
	err = ccs_release_object(os);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(cs);
	assert( err == CCS_SUCCESS );
	Py_Finalize();
	return;
#if PY_VERSION_HEX >= 0x030b0000
exception:
	PyConfig_Clear(&config);
	Py_ExitStatusException(status);
#endif
}

int main() {
	ccs_init();
	test();
	return 0;
}
