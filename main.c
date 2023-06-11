#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/*
 * pxec 
 * built-in: add, rm 
 * (C) slixenfeld
 * */

void print_help()
{
	printf("pxec\nrun programs linked by name in pxec\n");
}

int main(int argc, char **argv)
{
	for (int i = 0; i < argc; ++i)
	{
		if (*argv[i] == *"-h")
		{
			print_help();
		}
	}

	// user input
	while(1)
	{
		char in[500];
		scanf("%s", in);
		printf("%s\n\n", in);

		if ( strcmp(in,"bla") == 0)
		{
			int status = system("start mspaint.exe");
		}
	}

	return 0;
}
