/* Cynex: python link testi */
#include "Python.h"
extern void CynexPy_SetHome(const char* libDir);
int cynex_pytest(const char* home)
{
	CynexPy_SetHome(home);
	Py_NoSiteFlag = 1;
	Py_Initialize();
	int r = PyRun_SimpleString("import sys, math, time, struct, cPickle, zlib, datetime, _sre\nx = math.sqrt(16)\n");
	Py_Finalize();
	return r;
}
