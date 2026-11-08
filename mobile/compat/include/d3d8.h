// Cynex: orijinal d3d8.h - yapi duzeni _WIN32'ye gore (DriverVersion vb.), disari sizmasin
#pragma once
#include <windows.h>
#include <objbase.h>
#ifndef _WIN32
#define _WIN32 1
#define CYNEX_D3D8_WIN32
#endif
#include "../../../client/Extern/include/d3d8.h"
#ifdef CYNEX_D3D8_WIN32
#undef _WIN32
#undef CYNEX_D3D8_WIN32
#endif
