#ifndef BASIC_FUNCTIONS
#define BASIC_FUNCTIONS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fnmatch.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
#define HISTORY_SIZE 20

extern char* tokens[];//[MAX_NUM_TOKENS];
extern int history_count;// = 0;
extern char* history[];//[HISTORY_SIZE];

int tokenize_input(char* , char** , int* );

void add_history(char* );
void print_history();

int execute_command(char** , int , char* );

#endif