/* Cynex: 32 bit python27.dll ile py2 yorumlayicisi (yalniz sinama) */
#include <windows.h>
#include <stdio.h>

typedef int (*PFN_MAIN)(int, char**);
typedef void (*PFN_SETHOME)(char*);

int main(int argc, char** argv)
{
	HMODULE h;
	PFN_MAIN pyMain;
	PFN_SETHOME setHome;
	if (argc < 3)
	{
		printf("py2host <python27.dll> <home> [args]\n");
		return 2;
	}
	h = LoadLibraryA(argv[1]);
	if (!h)
	{
		printf("LoadLibrary failed %lu\n", GetLastError());
		return 3;
	}
	setHome = (PFN_SETHOME)GetProcAddress(h, "Py_SetPythonHome");
	pyMain = (PFN_MAIN)GetProcAddress(h, "Py_Main");
	if (!pyMain || !setHome)
		return 4;
	setHome(argv[2]);
	argv[2] = argv[0];
	return pyMain(argc - 2, argv + 2);
}
