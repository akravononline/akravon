// Cynex: Python-ast.h makrolari (Name, List, Set...) oyun kodunu bozuyor - yalnizca mod_ty bildir
#include <Python.h>
#ifndef Py_PYTHON_AST_H
typedef struct _mod* mod_ty;
#endif
#include <symtable.h>
#ifdef BYTE
#undef BYTE
#endif
