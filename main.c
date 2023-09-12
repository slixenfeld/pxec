#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "pxec.h"

#define C_RED    "\x1b[31m"
#define C_CYAN   "\x1b[36m"
#define C_RESET  "\x1b[0m"
#define C_GREEN  "\x1b[32m"
#define C_YELLOW "\x1b[33m"

/* pxec
 * (C) 2023, Simon Lixenfeld
 *
 * commands:
 * built-in: add, rm, ls, find, exit, help, clear, edit
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
char VERSION[] = "0.1";
char STORED[2048][1000];
int MAXBUFFER = 1000;
char* MAPFILE;

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

void save_to_file()
{
	FILE *fp;

	fp = fopen(MAPFILE, "w+");
	char * outstr = malloc(50000 * sizeof(char));
	strcpy(outstr, "");
	for (int i = 0; i < MAX_WORDS ; i++)
	{
		if (strcmp(STORED[i],"") != 0)
		{
			char * temp = malloc(2048 * sizeof(char));
			strcpy(temp, STORED[i]);
			strcat(temp, "\n");
			strcat(outstr, temp);
			free(temp);
		}
	}
	if (strcmp(outstr, "") != 0)
		fprintf(fp,outstr);

	fclose(fp);
	free(outstr);
}

int http_check(char* text)
{
	return (strstr(text, "https:") != NULL ||
			strstr(text, "http:") != NULL);
}

int number_check(char* in)
{
	char* numbers = "1234567890";
	for (int i = 0; i < strlen(in) ; i++)
	{
		int valid = 0;
		for(int j = 0; j < strlen(numbers) ; j++)
		{
			if (in[i] == numbers[j]) 
			{
				valid = 1;
			}
		}
		if (!valid) return 1;
	}
	return 0;
}

void run_cmd(char* in, char* argstr)
{

	int i = check_cmd_exists(in);
	if (i == -1)
	{
		beep(200,20);
		printf(C_RED"could not find \'%s\'\n"C_RESET, in);
		return;
	}
	printf(C_GREEN "-> %s"
			C_RESET "\n", in);

	char * cmd = malloc(1000 * sizeof(char));
	int position = 0;
	int cutoff = 25;
	char* delimiter = malloc(10 * sizeof(char)); 

	delimiter = "/";
#ifdef _WIN32
	delimiter = "\\";
#endif
	char *ptr;
	int delims = 0;
	int cur_delim = 0;
	char* path = malloc(0x40 * sizeof(char*));

	char* s = malloc(0x40 * sizeof(char*));
	strcpy(s, STORED[i+1]);
	ptr = strtok(s, delimiter);
	while(ptr != NULL)
	{
		delims++;
		ptr = strtok(NULL, delimiter);
	}

	if (delims > 1)
	{
		char* s = malloc(0x40 * sizeof(char*));
		char *ptr2;
		strcpy(path, "");
		strcpy(s, STORED[i+1]);
		ptr2 = strtok(s, delimiter);

#ifdef _WIN32

#else
		if ( STORED[i+1][0] == '/') 
		{
			strcat(path,delimiter);
		}
#endif
		strcat(path, ptr2);
		strcat(path,delimiter);
		while(ptr2 != NULL)
		{
			cur_delim++;
			ptr2 = strtok(NULL, delimiter);
			strcat(path, ptr2);
#ifdef _WIN32
			strcat(path,delimiter);
			if ( cur_delim == delims-2)
			{
				break;
			}
#else
			strcat(path,delimiter);
			if ( cur_delim == delims-2)
			{
				break;
			}
#endif
			//printf("%s\n",path);
			if (cur_delim == delims-1) break;
		} 
	}

	free(s);

	int type = 1; // 1 = WEB, 2 = APP, 3 = CMD

	if (http_check(STORED[i+1]))
	{
		type = 1;
	}
	else if ( delims > 1)
	{
		type = 2;
	}
	else
	{
		type = 3;
	}
	///////////////////////////////////////////////////////////
	strcpy(cmd, "");

#ifdef _WIN32
	if (type == 2)
	{
		strcpy(cmd, "start \"\" ");

		// Set missing beginning quotes around executable
		if (STORED[i+1][0] != '\"')
			strcat(cmd, "\"");
	}
#else
#endif
	strcat(cmd, STORED[i+1]);
	if (type == 2)
	{
		chdir(path); // set running dir
#ifdef _WIN32
		// Set missing end quotes around executable
		if (STORED[i+1][strlen(STORED[i+1])-1] != '\"')
			strcat(cmd, "\" ");
#endif
	}


	strcat(cmd, argstr);

	beep(440,10);
	int status = system( cmd );
	beep(500,10);
	free(cmd);
}


void print_list_entry(int i, int counter)
{
	int position = 0;
	int cutoff = 25;
	char* delimiter = malloc(10 * sizeof(char));
	delimiter = "/";
#ifdef _WIN32
	delimiter = "\\";
#endif
	char *ptr;
	int delims = 0;
	int cur_delim = 0;
	char* path = malloc(0x40 * sizeof(char*));

	char* s = malloc(0x40 * sizeof(char*));
	strcpy(s, STORED[i+1]);
	ptr = strtok(s, delimiter);
	while(ptr != NULL)
	{
		delims++;
		ptr = strtok(NULL, delimiter);
	}




	if (counter<10)
	{
		printf("[ 00%d ]", counter);
	} 
	else if (counter<100)
	{
		printf("[ 0%d ]", counter);
	} 
	else if (counter<1000)
	{
		printf("[ %d ]", counter);
	}

	if (http_check(STORED[i+1]))
	{
		printf( C_CYAN"  WEB  ");
	}
	else if ( delims > 1)
	{
		printf( C_GREEN"  APP  ");
	}
	else
	{
		printf( "  CMD  ");
	}

	position = 12;


	for( int c = 0 ; c < strlen(STORED[i]) ; c++)
	{
		printf("%c",STORED[i][c]);
		position++;
		if (position+2 >= cutoff)
		{
			printf("..");
			position += 2;
			break;
		}
	}
	for( int c = position ; c < cutoff ; c++)
	{
		printf(" ");
	}
	printf("  --->  ");
	printf(STORED[i+1]);
	printf(C_RESET);
	printf("\n");
}

void list(char* filter)
{
	int counter = 0;
	for (int i = 0 ; i < MAX_WORDS ; i++)
	{
		if (i % 2 == 0 && strcmp(STORED[i],"") != 0)
		{
			counter++;
			if (strlen(filter) == 0 
					|| (strstr(STORED[i], filter) != NULL))
			{
				print_list_entry(i, counter);
			}
			printf(C_RESET);
		}
	}
}

int check_cmd_exists(char* cmd)
{
	int ret_idx = -1;
	int cmd_count = 0;
	for (int i = 0 ; i < MAX_WORDS ; i++)
	{
		char count_str[5];
		if (i % 2 == 0)
		{
			cmd_count++;
		}
		if (!number_check(cmd))
		{
			sprintf(count_str, "%d", cmd_count);
		}
		if ( (i % 2 == 0 && strcmp(STORED[i+1],"") !=0) &&
				( (strcmp(cmd,STORED[i]) == 0)
				  || strcmp(count_str,cmd) == 0 ) )
		{
			ret_idx = i;
		}
	}
	return ret_idx;
}

void edit(char* edit_choice)
{
	if (strcmp(edit_choice, "*") == 0)
	{
		char * cmd = malloc(2048);
		printf("editing save file..\n");
		strcpy(cmd,"vim ");
		strcat(cmd, MAPFILE);

		int status = system( cmd );

		free(cmd);
	}
	else
	{
		int entry_id = check_cmd_exists(edit_choice);
		if (entry_id == -1)
		{
			printf(C_YELLOW"could not find \'%s\'\n", edit_choice);
			return;
		}

		char* entry = malloc(1024 * sizeof(char));
		strcpy(STORED[entry_id+1], entry);
		printf(C_YELLOW "%s will run" C_RESET "-> ", STORED[entry_id]);

		getline(&entry, &MAXBUFFER, stdin);
		remove_newline(entry);

		char *path = malloc(MAXBUFFER * sizeof(char));
		strcpy(path, "");
		strcat(path, entry);
		strcpy(STORED[entry_id+1], path);
		free(path);

		// Save Entry+Path
		save_to_file();

		printf(C_YELLOW "edited %s -> %s"
				C_RESET "\n",
				STORED[entry_id],
				STORED[entry_id+1]);
	}
}
void add_entry(char* in, int* entry_count)
{
	printf(C_GREEN "adding" C_RESET " -> ");
	getline(&in, &MAXBUFFER, stdin);
	remove_newline(in);

	strcpy(STORED[*entry_count], in);
	printf(C_GREEN "%s will run" C_RESET "-> ", in);
	getline(&in, &MAXBUFFER, stdin);
	remove_newline(in);

	char *path = malloc(MAXBUFFER * sizeof(char));
	strcpy(path, "");
	strcat(path, in);
	strcpy(STORED[*entry_count+1], path);
	free(path);

	// Save Entry+Path
	*entry_count+=2;
	save_to_file();

	printf(C_GREEN "added %s -> %s"
			C_RESET "\n",
			STORED[*entry_count-2],
			STORED[*entry_count-1]);

}

void remove_entry(char* in)
{
	printf(C_RED "removing " C_RESET " -> ");
	getline(&in, &MAXBUFFER, stdin);
	remove_newline(in);

	int i = check_cmd_exists(in);
	if (i == -1)
	{
		beep(200,20);
		printf(C_RED"could not find \'%s\'\n"C_RESET, in);
		return;
	}

	save_to_file();
	printf(C_RED "removed %s\n"C_RESET, in);
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

	MAPFILE = malloc(1024*sizeof(char));
	strcpy(MAPFILE, "");
	FILE *fp;

	char * line = NULL;
	size_t len = 0;
	ssize_t read;


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
	// Load File

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
		strcpy(STORED[i], "");

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
		strcpy(STORED[entry_count],line);
		entry_count++;
	}
	fclose(fp);

	if (line) free(line);

	///////////////////////////////////////////////////////////////////////////
	/// Commands
	while(1)
	{
		char *in = malloc(MAXBUFFER * sizeof(char));
		if (run_arg == 0)
		{
			getline(&in, &MAXBUFFER, stdin);
			remove_newline(in);
		}
		else
		{
			strcpy(in, cmdstr);
		}

		if (strcmp(in, "add") == 0)
			add_entry(in, &entry_count);
		else if (strcmp(in, "rm") == 0) // in, MAPFILE
			remove_entry(in);
		else if ( strcmp(in,"ls") == 0)
			list("");
		else if ( strcmp(in,"find") == 0)
		{
			printf( C_CYAN"find"C_RESET" -> ");
			getline(&in, &MAXBUFFER, stdin);
			remove_newline(in);
			list(in);	
		}
		else if ( strcmp(in,"exit") == 0
				||strcmp(in,"quit") == 0
				||strcmp(in,"q") == 0
			)
			break;
		else if ( strcmp(in,"help") == 0)
			printf(
					"add   --> add new alias\n"
					"rm    --> remove alias\n"
					"ls    --> list all aliases\n"
					"find  --> find alias\n"
					"clear --> clear screen\n"
					"help  --> display this message\n"
					"exit  --> exit\n"
					"q     --> exit\n"
					"quit  --> exit\n");
		else if ( strcmp(in, "clear") == 0)
			clear_screen();
		else if (strcmp(in, "edit") == 0)
		{
			printf( C_YELLOW"edit <*> or <name>"C_RESET" -> ");
			char* edit_choice = malloc(0xFF);
			getline(&edit_choice, &MAXBUFFER, stdin);
			remove_newline(edit_choice);
			edit(edit_choice);

			free(edit_choice);

			if (strcmp(edit_choice, "*") == 0) break;
		}
		else
		{
			run_cmd(in, argstr);
		}

		free(in);

		if(run_arg == 1)
			break;
	}
	///////////////////////////////////////////////////////////////////////////
	free(MAPFILE);
	free(cmdstr);
	free(argstr);

	return 0;
}
