#pragma once

#define SET_EXCEPTION(x) PyErr_SetString(PyExc_RuntimeError, #x)

bool PyTuple_GetString(PyObject* poArgs, int pos, char** ret);
bool PyTuple_GetInteger(PyObject* poArgs, int pos, unsigned char* ret);
bool PyTuple_GetInteger(PyObject* poArgs, int pos, int* ret);
bool PyTuple_GetInteger(PyObject* poArgs, int pos, WORD* ret);
bool PyTuple_GetInteger(PyObject* poArgs, int pos, short* ret);
bool PyTuple_GetByte(PyObject* poArgs, int pos, unsigned char* ret);
bool PyTuple_GetUnsignedInteger(PyObject* poArgs, int pos, unsigned int* ret);
bool PyTuple_GetLong(PyObject* poArgs, int pos, LONG* ret);
#ifdef CYNEX_MOBILE
// Cynex: android'de int64_t = long, long long ayri tip
bool PyTuple_GetLongLong(PyObject* poArgs, int pos, int64_t* ret);
bool PyTuple_GetUnsignedLongLong(PyObject* poArgs, int pos, uint64_t* ret);
#endif
bool PyTuple_GetUnsignedLong(PyObject* poArgs, int pos, DWORD* ret);
bool PyTuple_GetUnsignedLongLong(PyObject* poArgs, int pos, unsigned long long* ret);
bool PyTuple_GetLongLong(PyObject* poArgs, int pos, long long* ret);
bool PyTuple_GetFloat(PyObject* poArgs, int pos, float* ret);
bool PyTuple_GetDouble(PyObject* poArgs, int pos, double* ret);
bool PyTuple_GetObject(PyObject* poArgs, int pos, PyObject** ret);
bool PyTuple_GetBoolean(PyObject* poArgs, int pos, bool* ret);

bool PyCallClassMemberFunc(PyObject* poClass, const char* c_szFunc, PyObject* poArgs);
bool PyCallClassMemberFunc(PyObject* poClass, const char* c_szFunc, PyObject* poArgs, bool* pisRet);
bool PyCallClassMemberFunc(PyObject* poClass, const char* c_szFunc, PyObject* poArgs, LONG * plRetValue);

bool PyCallClassMemberFunc_ByPyString(PyObject* poClass, PyObject* poFuncName, PyObject* poArgs);
bool PyCallClassMemberFunc(PyObject* poClass, PyObject* poFunc, PyObject* poArgs);

PyObject * Py_BuildException(const char * c_pszErr = NULL, ...);
PyObject * Py_BadArgument();
PyObject * Py_BuildNone();
PyObject * Py_BuildEmptyTuple();
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
