#include "PythonuiscriptlibManager.h"
#include "Python27/Python.h"
#ifdef _DEBUG
	#pragma comment (lib, "uiscriptlib_d.lib")
#else
	#pragma comment (lib, "uiscriptlib.lib")
#endif

struct uiscriptlib_SMethodDef
{
	char* func_name;
	void (*func)();
};

PyMODINIT_FUNC initautoselldialog();
PyMODINIT_FUNC initbattlepasswindow();
PyMODINIT_FUNC initbosstime();
PyMODINIT_FUNC initluckyboxwindow();
PyMODINIT_FUNC initmenusystemdialog();
PyMODINIT_FUNC initnpcekran();
PyMODINIT_FUNC initoffer_banner();
PyMODINIT_FUNC initoffer_window();
PyMODINIT_FUNC initranking();
PyMODINIT_FUNC inittpwizarddropwindow();
PyMODINIT_FUNC inittpwizardinfowindow();

uiscriptlib_SMethodDef uiscriptlib_init_methods[] =
{
	{ "autoselldialog", initautoselldialog },
	{ "battlepasswindow", initbattlepasswindow },
	{ "bosstime", initbosstime },
	{ "luckyboxwindow", initluckyboxwindow },
	{ "menusystemdialog", initmenusystemdialog },
	{ "npcekran", initnpcekran },
	{ "offer_banner", initoffer_banner },
	{ "offer_window", initoffer_window },
	{ "ranking", initranking },
	{ "tpwizarddropwindow", inittpwizarddropwindow },
	{ "tpwizardinfowindow", inittpwizardinfowindow },
	{ NULL, NULL },
};

static PyObject* uiscriptlib_isExist(PyObject *self, PyObject *args)
{
	char* func_name;

	if(!PyArg_ParseTuple(args, "s", &func_name))
		return NULL;

	for (int i = 0; NULL != uiscriptlib_init_methods[i].func_name;i++)
	{
		if (0 == _stricmp(uiscriptlib_init_methods[i].func_name, func_name))
		{
			return Py_BuildValue("i", 1);
		}
	}
	return Py_BuildValue("i", 0);
}

static PyObject* uiscriptlib_moduleImport(PyObject *self, PyObject *args)
{
	char* func_name;

	if(!PyArg_ParseTuple(args, "s", &func_name))
		return NULL;

	for (int i = 0; NULL != uiscriptlib_init_methods[i].func_name;i++)
	{
		if (0 == _stricmp(uiscriptlib_init_methods[i].func_name, func_name))
		{
			uiscriptlib_init_methods[i].func();
			if (PyErr_Occurred())
				return NULL;
			PyObject* m = PyDict_GetItemString(PyImport_GetModuleDict(), uiscriptlib_init_methods[i].func_name);
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

static PyObject* uiscriptlib_getList(PyObject *self, PyObject *args)
{
	int iTupleSize = 0;
	while (NULL != uiscriptlib_init_methods[iTupleSize].func_name) {iTupleSize++;}

	PyObject* retTuple = PyTuple_New(iTupleSize);
	for (int i = 0; NULL != uiscriptlib_init_methods[i].func_name; i++)
	{
		PyObject* retSubString = PyString_FromString(uiscriptlib_init_methods[i].func_name);
		PyTuple_SetItem(retTuple, i, retSubString);
	}
	return retTuple;
}

void inituiscriptlibManager()
{
	static struct PyMethodDef methods[] =
	{
		{"isExist", uiscriptlib_isExist, METH_VARARGS},
		{"moduleImport", uiscriptlib_moduleImport, METH_VARARGS},
		{"getList", uiscriptlib_getList, METH_VARARGS},
		{NULL, NULL},
	};

	PyObject* m;
	m = Py_InitModule("uiscriptlib", methods);
}
