#include "PythonrootlibManager.h"
#include "Python27/Python.h"
#ifdef _DEBUG
	#pragma comment (lib, "rootlib_d.lib")
#else
	#pragma comment (lib, "rootlib.lib")
#endif

struct rootlib_SMethodDef
{
	char* func_name;
	void (*func)();
};

PyMODINIT_FUNC initsystem();
PyMODINIT_FUNC inituiareyouinornot();
PyMODINIT_FUNC inituiautohunt();
PyMODINIT_FUNC inituiautosell();
PyMODINIT_FUNC inituibattlepass();
PyMODINIT_FUNC inituibiyolog();
PyMODINIT_FUNC inituibosstime();
PyMODINIT_FUNC inituidcitemshop();
PyMODINIT_FUNC inituidcitemshopadmin();
PyMODINIT_FUNC inituiikashop();
PyMODINIT_FUNC inituiluckybox();
PyMODINIT_FUNC inituimenusystem();
PyMODINIT_FUNC inituinpcekran();
PyMODINIT_FUNC inituioffergui();
PyMODINIT_FUNC inituipublisherinfo();
PyMODINIT_FUNC inituirank();
PyMODINIT_FUNC inituitpwizardinfo();
PyMODINIT_FUNC inituiwheeloffortune();

rootlib_SMethodDef rootlib_init_methods[] =
{
	{ "system", initsystem },
	{ "uiareyouinornot", inituiareyouinornot },
	{ "uiautohunt", inituiautohunt },
	{ "uiautosell", inituiautosell },
	{ "uibattlepass", inituibattlepass },
	{ "uibiyolog", inituibiyolog },
	{ "uibosstime", inituibosstime },
	{ "uidcitemshop", inituidcitemshop },
	{ "uidcitemshopadmin", inituidcitemshopadmin },
	{ "uiikashop", inituiikashop },
	{ "uiluckybox", inituiluckybox },
	{ "uimenusystem", inituimenusystem },
	{ "uinpcekran", inituinpcekran },
	{ "uioffergui", inituioffergui },
	{ "uipublisherinfo", inituipublisherinfo },
	{ "uirank", inituirank },
	{ "uitpwizardinfo", inituitpwizardinfo },
	{ "uiwheeloffortune", inituiwheeloffortune },
	{ NULL, NULL },
};

static PyObject* rootlib_isExist(PyObject *self, PyObject *args)
{
	char* func_name;

	if(!PyArg_ParseTuple(args, "s", &func_name))
		return NULL;

	for (int i = 0; NULL != rootlib_init_methods[i].func_name;i++)
	{
		if (0 == _stricmp(rootlib_init_methods[i].func_name, func_name))
		{
			return Py_BuildValue("i", 1);
		}
	}
	return Py_BuildValue("i", 0);
}

static PyObject* rootlib_moduleImport(PyObject *self, PyObject *args)
{
	char* func_name;

	if(!PyArg_ParseTuple(args, "s", &func_name))
		return NULL;

	for (int i = 0; NULL != rootlib_init_methods[i].func_name;i++)
	{
		if (0 == _stricmp(rootlib_init_methods[i].func_name, func_name))
		{
			rootlib_init_methods[i].func();
			if (PyErr_Occurred())
				return NULL;
			PyObject* m = PyDict_GetItemString(PyImport_GetModuleDict(), rootlib_init_methods[i].func_name);
			if (m == NULL) {
				PyErr_SetString(PyExc_SystemError,
					"dynamic module not initialized properly");
				return NULL;
			}
			Py_INCREF(m);
			return Py_BuildValue("S", m);
		}
	}
	return NULL;
}

static PyObject* rootlib_getList(PyObject *self, PyObject *args)
{
	int iTupleSize = 0;
	while (NULL != rootlib_init_methods[iTupleSize].func_name) {iTupleSize++;}

	PyObject* retTuple = PyTuple_New(iTupleSize);
	for (int i = 0; NULL != rootlib_init_methods[i].func_name; i++)
	{
		PyObject* retSubString = PyString_FromString(rootlib_init_methods[i].func_name);
		PyTuple_SetItem(retTuple, i, retSubString);
	}
	return retTuple;
}

void initrootlibManager()
{
	static struct PyMethodDef methods[] =
	{
		{"isExist", rootlib_isExist, METH_VARARGS},
		{"moduleImport", rootlib_moduleImport, METH_VARARGS},
		{"getList", rootlib_getList, METH_VARARGS},
		{NULL, NULL},
	};

	PyObject* m;
	m = Py_InitModule("rootlib", methods);
}
