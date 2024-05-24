#ifndef FUNCTIONS
#define FUNCTIONS
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

#define MAX_ALIASES 10
typedef struct {
    char* alias_name;
    char* alias_value;
} alias;
//alias aliases[MAX_ALIASES];
extern alias aliases[];
extern int num_aliases;

void handle_cd_command(char** , int );

void expand_wildcards(char** , int* );

int handle_redirection(char**, int , int* , int* );

int count_pipes(char** , int );
void handle_pipe_command(char* [], int );

void handle_sigint();
void handle_sigtstp(int );

char* extract_quoted_string(char* );
void create_alias(char* , char* );
char* get_alias_command(char* );
void execute_alias_command(char* , int );
void destroy_alias(char* );


#endif