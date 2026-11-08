/* Cynex: getpath.c yerine - lib yolu CynexPy_SetHome ile disaridan */
#include "Python.h"
#include <string.h>

static char s_home[1024] = "lib";
static char s_path[2048] = "lib";

void CynexPy_SetHome(const char* libDir)
{
	if (!libDir || !*libDir)
		return;
	strncpy(s_home, libDir, sizeof(s_home) - 1);
	s_home[sizeof(s_home) - 1] = '\0';
	/* Cynex: site-packages yok */
	PyOS_snprintf(s_path, sizeof(s_path), "%s", s_home);
}

char* Py_GetPath(void)
{
	return s_path;
}

char* Py_GetPrefix(void)
{
	return s_home;
}

char* Py_GetExecPrefix(void)
{
	return s_home;
}

char* Py_GetProgramFullPath(void)
{
	return Py_GetProgramName();
}
