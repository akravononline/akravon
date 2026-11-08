#include "StdAfx.h"
#include "../eterLib/Profiler.h"

PyObject * profilerPush(PyObject * poSelf, PyObject * poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	return Py_BuildNone();
}

PyObject * profilerPop(PyObject * poSelf, PyObject * poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	return Py_BuildNone();
}

void initProfiler()
{
	static PyMethodDef s_methods[] =
	{
		{ "Push",				profilerPush,				METH_VARARGS },
		{ "Pop",				profilerPop,				METH_VARARGS },

		{ NULL,					NULL,						NULL		 },
	};

	Py_InitModule("profiler", s_methods);
}
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
