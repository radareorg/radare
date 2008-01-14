#include <stdio.h>
#include "radare.h"

int main()
{
	char line[1024];
	FILE * fd = fopen("Radare.py", "r");

	Py_Initialize();

	//init_radare();

	PyRun_SimpleString("_Radare = Radare");
	PyRun_SimpleFile(fd, "Radare.py");
	
	//PyRun_SimpleFile(fd, "_radare");
	//PyRun_SimpleString("import radare.py");
	//#PyRun_SimpleString("import _radare");

	while(!feof(stdin)) {
		line[0]='\0';
		fgets(line,1024,stdin);
		if (!feof(stdin))
			line[strlen(line)-1]='\0';
		else
			break;
		PyRun_SimpleString(line);
	}

	return 0;
}
