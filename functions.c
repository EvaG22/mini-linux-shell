#include "functions.h"
#include "basic_functions.h"

alias aliases[MAX_ALIASES];
int num_aliases = 0;
//CD
void handle_cd_command(char **tokens, int num_tokens) {
    if (num_tokens < 2) {
        // no argument provided, go to home directory
        if (chdir(getenv("HOME")) != 0) {
            perror("cd");
        }
    } else {
        // argument provided, change directory to the specified path
        if (chdir(tokens[1]) != 0) {
            perror("cd");
        }
    }
}

void expand_wildcards(char** tokens, int* num_tokens) {
    DIR* dir;
    struct dirent* ent;
    int token_count = *num_tokens;
    for (int i = 0; i < token_count; i++) {
        char* token = tokens[i];
        int len = strlen(token);
        int found = 0;
        for (int j = 0; j < len; j++) {
            if (token[j] == '*' || token[j] == '?') {
                found = 1;
                break;
            }
        }
        if (found) {
            dir = opendir(".");
            if (dir != NULL) {
                while ((ent = readdir(dir)) != NULL && *num_tokens < MAX_NUM_TOKENS) {
                    if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                        continue;
                    }
                    if (fnmatch(token, ent->d_name, FNM_PATHNAME) == 0) {
                        tokens[*num_tokens] = strdup(ent->d_name);
                        (*num_tokens)++;
                    }
                }
                closedir(dir);
            }
        }
    }
}


//REDIRECTION
int handle_redirection(char **tokens, int num_tokens, int *input_fd, int *output_fd) {

    if (num_tokens > 1) {
        if (strcmp(tokens[num_tokens - 2], "<") == 0) {
            // input redirection
            *input_fd = open(tokens[num_tokens - 1], O_RDONLY);
            if (*input_fd == -1) {
                perror("open failed");
                return 1;
            }
            tokens[num_tokens - 2] = NULL;
        } else if (strcmp(tokens[num_tokens - 2], ">") == 0) {
            // output redirection
            *output_fd = open(tokens[num_tokens - 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if (*output_fd == -1) {
                perror("open failed");
                return 1;
            }
            tokens[num_tokens - 2] = NULL;
        } else if (strcmp(tokens[num_tokens - 2], ">>") == 0) {
            // output redirection (append)
            *output_fd = open(tokens[num_tokens - 1], O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if (*output_fd == -1) {
                perror("open failed");
                return 1;
            }
            tokens[num_tokens - 2] = NULL;
        } 
    }
    return 0;
}


//PIPES
int count_pipes(char** tokens, int num_tokens) {
    int count = 0;
    for (int i = 0; i < num_tokens; i++) {
        if (strcmp(tokens[i], "|") == 0) {
            count++;
        }
    }
    return count;
}

void handle_pipe_command(char *tokens[], int num_tokens) {
    // Locate the position of the pipe character '|'
    int pipe_pos = -1;
    for (int i = 0; i < num_tokens; i++) {
        if (strcmp(tokens[i], "|") == 0) {
            pipe_pos = i;
            break;
        }
    }
    if (pipe_pos == -1) {
        printf("Error: handle_pipe_command called with no pipe character\n");
        return;
    }

    // Create two subcommands: command1 and command2
    char *command1[pipe_pos + 1]; // +1 for NULL terminator
    for (int i = 0; i < pipe_pos; i++) {
        command1[i] = tokens[i];
    }
    command1[pipe_pos] = NULL;

    char *command2[num_tokens - pipe_pos];
    for (int i = pipe_pos + 1; i < num_tokens; i++) {
        command2[i - pipe_pos - 1] = tokens[i];
    }
    command2[num_tokens - pipe_pos - 1] = NULL;

    // Create the pipe
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return;
    }

    // Fork a child process to execute the first command (command1)
    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("fork failed");
        return;
    } else if (pid1 == 0) {
        // Child process: redirect stdout to the write end of the pipe
        close(pipefd[0]); // Close unused read end
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        // Execute command1
        int status_code = execvp(command1[0], command1);
        fprintf(stderr, "I am here 1 %d", status_code);
        fflush(stdout);
        if(status_code == -1){
            perror("execvp for command 1");

            exit(EXIT_FAILURE);
        }
    }

    // Fork another child process to execute the second command (command2)
    pid_t pid2 = fork();
    if (pid2 == -1) {
        perror("fork");
        return;
    } else if (pid2 == 0) {
        // Child process: redirect stdin to the read end of the pipe
        close(pipefd[1]); // Close unused write end
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);

        // Execute command2
        int status_code = execvp(command2[0], command2);
        fprintf(stderr, "I am here 2 %d", status_code);
        fflush(stdout);
        if(status_code == -1){
            perror("execvp for command 2");
            exit(EXIT_FAILURE);
        }
    }

    // Close the pipe in the parent process
    close(pipefd[0]);
    close(pipefd[1]);

    // Wait for both child processes to finish
    //int status1, status2;
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}


//SIGNALS
void handle_sigint() {
    printf("\nReceived SIGINT (CTRL-C) signal. Exit now.\n");
}

void handle_sigtstp(int sig) {
    printf("\nReceived SIGTSTP (CTRL-Z) signal. Give fg to restart.\n");
    exit(sig);
}


//ALIASES

// This function is useful for aliases. For example when user gives the command createalias lll "ls -las",
// I only want to keep ls -las
char* extract_quoted_string(char* str) {
    char* start = strchr(str, '\"');
    if (!start) {
        return NULL; // no double quotes found
    }
    char* end = strchr(start + 1, '\"');
    if (!end) {
        return NULL; // no matching double quote found
    }
    size_t len = end - start - 1;
    char* result = malloc(len + 1);
    memcpy(result, start + 1, len);
    result[len] = '\0';
    return result;
}

void create_alias(char* alias_name, char* alias_value) {
    if (num_aliases >= MAX_ALIASES) {
        printf("Maximum number of aliases reached.\n");
        return;
    }

    aliases[num_aliases].alias_name = malloc(sizeof(char) * (strlen(alias_name)+1));
    strcpy(aliases[num_aliases].alias_name, alias_name);
    aliases[num_aliases].alias_value = malloc(sizeof(char) * (strlen(alias_value)+1));
    strcpy(aliases[num_aliases].alias_value, alias_value);

    num_aliases++;
    printf("Alias created.\n");
}


char* get_alias_command(char* alias_name) {
    for (int i = 0; i < num_aliases; i++) {
        if (strcmp(aliases[i].alias_name, alias_name) == 0) {
            return aliases[i].alias_value;
        }
    }
    return NULL;
}

void execute_alias_command(char* alias_value, int num_aliases) {
    char* alias_tokens[MAX_ALIASES];
    tokenize_input(alias_value, alias_tokens, &num_aliases);

    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        execvp(alias_tokens[0], alias_tokens);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("fork");
    }
}

void destroy_alias(char* alias_name) {
    int i;
    for (i = 0; i < num_aliases; i++) {
        if (strcmp(aliases[i].alias_name, alias_name) == 0) {
            free(aliases[i].alias_name);
            free(aliases[i].alias_value);
            num_aliases--;

            // Shift all elements after i one position to the left
            for (int j = i; j < num_aliases; j++) {
                aliases[j].alias_name = aliases[j + 1].alias_name;
                aliases[j].alias_value = aliases[j + 1].alias_value;
            }

            printf("Alias destroyed.\n");
            return;
        }
    }

    printf("Alias not found.\n");
}