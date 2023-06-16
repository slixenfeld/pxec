#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_GREEN   "\x1b[32m"

/*
 * pxec 
 * built-in: add, rm, ls, exit, help, clear
 * (C) slixenfeld
 * */

int MAX_WORDS = 2048;
char VERSION[] = "0.1.1";

void print_version()
{
	printf("%s", VERSION);
	exit(0);
}

void remove_newline(char* line)
{
	int len;
	len = strlen(line);
	if( line[len-1] == '\n' )
	{
		line[len-1] = 0;
	}
}

void clear_screen()
{
	for(int i = 0 ; i < 300 ; i++)
	{
		printf("\n");
	}
}

void save_to_file(char stored[][500], char* file)
{
	FILE *fp;

	fp = fopen(file, "w+");
	char * outstr = malloc(50000 * sizeof(char));
	strcpy(outstr, "");
	for (int i = 0; i < MAX_WORDS ; i++)
	{
		if (strcmp(stored[i],"") != 0)
		{
			char * temp = malloc(2048 * sizeof(char));
			strcpy(temp, stored[i]);
			strcat(temp, "\n");
			strcat(outstr, temp);
			free(temp);
		}
	}
	fprintf(fp,outstr);
	fclose(fp);
	free(outstr);
}

int main(int argc, char **argv)
{
	for (int i = 0; i < argc; ++i)
	{
		if(*argv[i] == *"-v")
		{
			print_version();
		}
	}

	char *MAPFILE = (char*) malloc(500*sizeof(char));
	strcpy(MAPFILE, "");
	strcpy(MAPFILE, getenv("APPDATA"));
	strcat(MAPFILE, "\\map.pxec");


	FILE *fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	char stored[MAX_WORDS][500];

	for (int i = 0; i < MAX_WORDS ; i++)
	{
		strcpy(stored[i], "");
	}


	fp = fopen(MAPFILE, "r");
	if (fp == NULL)
	{
		fp = fopen(MAPFILE, "w+");
		fprintf(fp, "exit\nexit\n");
		printf("save file created.\n");
		fclose(fp);
		exit(0);
	}

	int entry_count = 0;
	while ((read = getline(&line, &len, fp)) != -1)
	{
		remove_newline(line);
		strcpy(stored[entry_count],line);
		entry_count++;
	}
	fclose(fp);

	if (line) free(line);

	int input_type = 0;
	int cmd_found = 1;

	clear_screen();

	// Infinite Input
	while(1)
	{
		int maxbuf = 500;
		char *in = (char *)malloc(maxbuf + sizeof(char));
		getline(&in, &maxbuf, stdin);
		int len;
		
		remove_newline(in);

		if (input_type == 1)
		{
			strcpy(stored[entry_count], in);
			input_type = 2;
			printf("path/exe -> ");
		}
		else if (input_type == 2)
		{
			char *path = (char *)malloc(maxbuf + sizeof(char));
			strcpy(path, "");
			strcat(path, "\"");
			strcat(path, in);
			strcat(path, "\"");
			strcpy(stored[entry_count+1], path);
			free(path);

			entry_count+=2;

			save_to_file(stored, MAPFILE);

			printf(ANSI_COLOR_GREEN "added %s -> %s" ANSI_COLOR_RESET "\n", stored[entry_count-2], stored[entry_count-1]);
			input_type = 0;
		}
		else if (input_type == 3)
		{

			char * key = malloc(500*sizeof(char));
			char * val = malloc(500*sizeof(char));

			// remove entry [key, value]
			for (int i = 0; i < MAX_WORDS ; i++)
			{
				if ( i % 2 == 0 && strcmp(stored[i],in) == 0)
				{
					strcpy(key,stored[i]);
					strcpy(val,stored[i+1]);
					strcpy(stored[i], "");
					strcpy(stored[i+1], "");
				}
			}
			// Write to File
			save_to_file(stored, MAPFILE);

			free(key);
			free(val);

			printf(ANSI_COLOR_RED "removed %s" ANSI_COLOR_RESET "\n", key);
			input_type = 0;
		}
		else if (input_type == 0) 
		{
			if ( strcmp(in,"ls") == 0)
			{
				int counter = 0;
				for (int i = 0 ; i < MAX_WORDS ; i++)
				{
					if (i % 2 == 0 && strcmp(stored[i],"") != 0)
					{
						counter++;
						if (counter < 10)
							printf("0%d:[%s] -> %s\n", counter, stored[i], stored[i+1]);
						else
							printf("%d:[%s] -> %s\n", counter, stored[i], stored[i+1]);
					}
				}
			}
			else if ( strcmp(in,"exit") == 0)
			{
				break;
			}
			else if ( strcmp(in,"help") == 0)
			{
				printf("add, rm, ls, clear, help, exit\n");
			}
			else if ( strcmp(in, "clear") == 0)
			{
				clear_screen();
			}	
			else if ( strcmp(in, "add") == 0)
			{
				input_type = 1;
				printf(ANSI_COLOR_GREEN "adding" ANSI_COLOR_RESET " -> ");
			}
			else if ( strcmp(in, "rm") == 0)
			{
				input_type = 3;
				printf(ANSI_COLOR_RED "removing " ANSI_COLOR_RESET " -> ");
			}
			else
			{
				cmd_found = 0;
				for (int i = 0 ; i < MAX_WORDS ; i++)
				{
					if ( i % 2 == 0 && strcmp(stored[i+1],"") !=0 && strcmp(in,stored[i]) == 0)
					{
						cmd_found = 1;

						printf(ANSI_COLOR_GREEN "-> %s" ANSI_COLOR_RESET "\n", in);

						char * cmd = malloc(1000 * sizeof(char));

						strcpy(cmd,"start \"\" ");
						strcat(cmd, stored[i+1]);

						int status = system( cmd );
						free(cmd);
					}
				}

				if (cmd_found == 0)
				{
					int status = system( in );
				}
			}
		}
		free(in);
	}

	free(MAPFILE);
	return 0;
}
