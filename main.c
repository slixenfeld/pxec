#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/*
 * pxec 
 * built-in: add, rm, ls, exit, help
 * (C) slixenfeld
 * */

int MAX_WORDS = 2048;
char VERSION[] = "0.1.0";

void print_version()
{
	printf("%s", VERSION);
	exit(0);
}
char* remove_new_line(char* string)
{
    size_t length = strlen(string);
    if((length > 0) && (string[length-1] == '\n'))
    {
        string[length-1] ='\0';
    }
    return string;
}

void remove_newline(char* line)
{
	int len;
		// remove newline
		len = strlen(line);
		if( line[len-1] == '\n' )
			line[len-1] = 0;
}


void save_map(char stored[][455])
{
	FILE *fp = NULL;
	// Write to File
	fp = fopen("./map.pxec", "w+");
	char * outstr = malloc(2400 * sizeof(char));
	strcpy(outstr, "");
	for (int i = 0; i < MAX_WORDS ; i++)
	{
		if (strcmp(stored[i],"") != 0)
		{
			char * temp = malloc(2400 * sizeof(char));
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

	FILE *fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	char stored[MAX_WORDS][455];

	for (int i = 0; i < MAX_WORDS ; i++)
	{
		strcpy(stored[i], "");
	}

	int entry_count = 0;

	fp = fopen("./map.pxec", "r");
	if (fp == NULL)
	{
		fp = fopen("./map.pxec", "w+");
		fprintf(fp, "exit\nexit\ntest\ntest");
		printf("save file created.\n");
		fclose(fp);
	}

	// Read From File
	while ((read = getline(&line, &len, fp)) != -1)
	{
		remove_newline(line);
		strcpy(stored[entry_count],line);
		entry_count++;
	}

	fclose(fp);

	if (line) free(line);

	int input_type = 0;

	// Infinite Input
	while(1)
	{
		int maxbuf = 455;
		char *in = (char *)malloc(maxbuf + sizeof(char));
		getline(&in, &maxbuf, stdin);
		int len;
		
		remove_newline(in);

		if (input_type == 1)
		{
			strcpy(stored[entry_count], in);
			input_type = 2;
			printf("path: ");
		}
		else if (input_type == 2)
		{
			char *path = (char *)malloc(maxbuf + sizeof(char));
			strcpy(path, "");
			strcat(path, "\"");
			strcat(path, in);
			strcat(path, "\"");
			strcpy(stored[entry_count+1], path);
			entry_count+=2;

			// Write to File
			save_map(stored);

			free(path);

			input_type = 0;
		}
		else if (input_type == 3)
		{
			// remove entry [key, value]
			for (int i = 0; i < MAX_WORDS ; i++)
			{
				if ( i % 2 == 0 && strcmp(stored[i],in) == 0)
				{
					strcpy(stored[i], "");
					strcpy(stored[i+1], "");
				}
			}
			// Write to File
			save_map(stored);

			input_type = 0;
		}
		else if (input_type == 0) // parse stored
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
				return 0;
			}
			else if ( strcmp(in,"help") == 0)
			{
				printf("add[Add a new program], rm[Remove a program], ls[List programs], help[Show this message], exit[Exit pxec]\n");
			}
			else if ( strcmp(in, "add") == 0)
			{
				input_type = 1;
				printf("adding word: ");
			}
			else if ( strcmp(in, "rm") == 0)
			{
				input_type = 3;
				printf("removing word: ");
			}
			for (int i = 0 ; i < MAX_WORDS ; i++)
			{
				if ( i % 2 == 0 && strcmp(stored[i+1],"") !=0 && strcmp(in,stored[i]) == 0)
				{
					printf("[%s]\n", in);
					char * cmd = malloc(1000 * sizeof(char));
					strcpy(cmd,"start /min cmd /c ");
					strcat(cmd, stored[i+1]);
					int status = system( cmd );
					free(cmd);
				}
			}
		}
	free(in);
	}

	return 0;
}
