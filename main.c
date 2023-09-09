#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "pxec.h"

#define C_RED              "\x1b[31m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define C_RESET            "\x1b[0m"
#define C_GREEN            "\x1b[32m"

/* pxec
 * (C) 2023, Simon Lixenfeld
 *
 * commands:
 * built-in: add, rm, ls, exit, help, clear, edit
 *
 * License: GPLv3(+), see LICENSE for details
 *
 * pxec is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * pxec is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

void save_to_file(char stored[][1000], char* file)
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

void run_cmd(char* in, char stored[][1000], char* argstr)
{
	int cmd_found = 0;
	for (int i = 0 ; i < MAX_WORDS ; i++)
	{
		if ( i % 2 == 0 && strcmp(stored[i+1],"") !=0 
				&& strcmp(in,stored[i]) == 0)
		{
			cmd_found = 1;

			printf(C_GREEN "-> %s"
					C_RESET "\n", in);

			char * cmd = malloc(1000 * sizeof(char));

			strcpy(cmd, stored[i+1]);
			strcat(cmd, argstr);

			beep(440,20);
			int status = system( cmd );
			free(cmd);
		}
	}
	if (cmd_found == 0) {
		beep(200,20);
		printf(C_RED "could not find \'%s\' \n"
				C_RESET, in);
	}

}

void list(char stored[][1000])
{
	int counter = 0;
	for (int i = 0 ; i < MAX_WORDS ; i++)
	{
		if (i % 2 == 0 && strcmp(stored[i],"") != 0)
		{
			counter++;
			if (strstr(stored[i+1], ".exe") == NULL)
			{
				printf(ANSI_COLOR_CYAN);
			}
			else
			{
				printf(C_GREEN);
			}
			if (counter % 5 == 0)
			{
				printf("\n");
			}
			printf((counter < 10) 
					? "0%d: %s  " 
					: "%d: %s  ", counter, stored[i]);

			printf(C_RESET);
		}
	}
}

void edit(char* MAPFILE)
{
	printf("editing save file..\n");
	char * cmd = malloc(1024 * sizeof(char));

	strcpy(cmd,"vim ");
	strcat(cmd, MAPFILE);

	int status = system( cmd );

	free(cmd);
}

void beep(int freq, int len)
{
#ifdef _WIN32

#else
	char * beepstr = malloc(0x20 + sizeof(char));
	strcpy(beepstr, "");
	snprintf(beepstr, 0x20, "beep -f %d -l %d", freq, len);
	system(beepstr);
	free(beepstr);
#endif
}
//////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
	beep(200, 10);
	beep(400, 10);

	int run_arg = 0;
	char * cmdstr = malloc(1024*sizeof(char));
	strcpy(cmdstr, "");
	char * argstr = malloc(1024*sizeof(char));
	strcpy(argstr, "");

	char *MAPFILE = malloc(1024*sizeof(char));
	strcpy(MAPFILE, "");
	FILE *fp;

	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	char stored[MAX_WORDS][1000];


	for (int i = 0; i < argc; ++i)
	{
		if(*argv[i] == *"-v")
		{
			print_version();
			free(MAPFILE);
			return 0;
		}
		else if(i == 1)
		{
			run_arg = 1;
			strcpy(cmdstr, argv[1]);
			strcpy(argstr, "");

		}
		else if(i > 0)
		{
			strcat(argstr, " ");
			strcat(argstr, argv[i]);
		}
	}

	///////////////////////////////////////////////////////////////////////////


#ifdef _WIN32
	// WINDOWS
	strcpy(MAPFILE, getenv("APPDATA"));
	strcat(MAPFILE, "\\map.pxec");
#else
	// LINUX
	strcat(MAPFILE, getenv("HOME"));
	strcat(MAPFILE, "/.map.pxec");
#endif
	for (int i = 0; i < MAX_WORDS ; i++)
		strcpy(stored[i], "");

	fp = fopen(MAPFILE, "r");
	if (fp == NULL)
	{
		fp = fopen(MAPFILE, "w+");
		fprintf(fp, "exit\nexit\n");
		printf("save file created.\n");
		fclose(fp);

		fp = fopen(MAPFILE, "r");
		if (fp == NULL)
		{
			free(MAPFILE);
			return 0;
		}
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

	///////////////////////////////////////////////////////////////////////////
	while(1)
	{
		int maxbuf = 1000;
		char *in = malloc(maxbuf * sizeof(char));
		if (run_arg == 0)
		{
			getline(&in, &maxbuf, stdin);
			remove_newline(in);
		}
		else
		{
			strcpy(in, cmdstr);
		}

		if (strcmp(in, "add") == 0)
		{
			// Read Entry Name
			printf(C_GREEN "adding" C_RESET " -> ");
			getline(&in, &maxbuf, stdin);
			remove_newline(in);

			strcpy(stored[entry_count], in);
			printf("path/exe -> ");

			// Read Path
			getline(&in, &maxbuf, stdin);
			remove_newline(in);

			char *path = malloc(maxbuf * sizeof(char));
			strcpy(path, "");
			strcat(path, in);
			strcpy(stored[entry_count+1], path);
			free(path);

			// Save Entry+Path
			entry_count+=2;
			save_to_file(stored, MAPFILE);

			printf(C_GREEN "added %s -> %s"
					C_RESET "\n",
					stored[entry_count-2],
					stored[entry_count-1]);
		}
		else if (strcmp(in, "rm") == 0)
		{
			printf(C_RED "removing " C_RESET " -> ");

			getline(&in, &maxbuf, stdin);
			remove_newline(in);

			char * key = malloc(1000*sizeof(char));
			char * val = malloc(1000*sizeof(char));

			int found = 0;
			// remove entry [key, value]
			for (int i = 0; i < MAX_WORDS ; i++)
			{
				if ( i % 2 == 0 && strcmp(stored[i],in) == 0)
				{
					strcpy(key,stored[i]);
					strcpy(val,stored[i+1]);
					strcpy(stored[i], "");
					strcpy(stored[i+1], "");
					found = 1;
				}
			}
			save_to_file(stored, MAPFILE);
			printf((found == 1) ?  C_RED "removed %s\n"C_RESET 
					: C_RED "could not find %s\n"
					C_RESET , in);

			free(key);
			free(val);

		}
		else if ( strcmp(in,"ls") == 0)
		{
			list(stored);
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
		else if (strcmp(in, "edit") == 0)
		{
			edit(MAPFILE);
			break;
		}
		else
		{
			run_cmd(in, stored, argstr);
		}

		free(in);

		if(run_arg == 1)
			break;
	}
	///////////////////////////////////////////////////////////////////////
	free(MAPFILE);
	free(cmdstr);
	free(argstr);

	return 0;
}
