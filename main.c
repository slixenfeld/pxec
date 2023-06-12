#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/*
 * pxec 
 * built-in: add, rm, ls
 * (C) slixenfeld
 * */

void print_help()
{
	printf("pxec\nrun programs linked by name in pxec\n");
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

int main(int argc, char **argv)
{
	for (int i = 0; i < argc; ++i)
	{
		if (*argv[i] == *"-h")
		{
			print_help();
		}
	}

	int MAX_WORDS = 2048;
	FILE *fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	char stored[MAX_WORDS][455];
	for (int i = 0; i < MAX_WORDS ; i++)
	{
		strcpy(stored[i], "");
	}

	int i = 0;
	int entry_count = 0;

	fp = fopen("./map.pxec", "r");
	if (fp == NULL)
	{
		fp = fopen("./map.pxec", "w+");
		fprintf(fp, "exit\nexit\ntest\ntest");
		printf("save file created.\n");
		fclose(fp);
	}

	while ((read = getline(&line, &len, fp)) != -1)
	{
		remove_newline(line);

		strcpy(stored[i],line);
		i++;
		entry_count++;
	}

	fclose(fp);

	if (line) free(line);

	int input_type = 0;

	// user input
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
			// and write to file
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

			input_type = 0;
		}
		else if (input_type == 0) // parse stored
		{
			if ( strcmp(in,"ls") == 0)
			{
				for (int i = 0 ; i < MAX_WORDS ; i++)
				{
					if (i % 2 == 0 && strcmp(stored[i],"") != 0)
					printf("<%d>: [%s]\n",i, stored[i]);
				}
			}
			else if ( strcmp(in,"exit") == 0)
			{
				return 0;
			}
			else if ( strcmp(in,"help") == 0)
			{
				printf("add[Add a new program], rm[Remove a program], ls[List programs]\n");
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
