#include <stdio.h>
#include "compiler.h"

int main(int argc, char *args[])
{
	setlocale (LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain (PACKAGE);
	if (argc < 2)
	{
		goto error;
	}
	//	printf("hello world!\n");
	int res = compile_file(args[1], args[2], 0);
	if (res == COMPILER_FAILED_WITH_ERROR)
	{
	error:
		printf(_("Compile failed or compile error\n"));
	}
	else if (res == COMPILER_FILE_COMPILE_OK)
	{
		printf(_("Everything compile fine\n"));
	}
	else
	{
		printf(_("Unknow error occurred\n"));
	}
	return 0;
}
