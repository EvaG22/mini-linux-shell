#include "functions.h"
#include "basic_functions.h"

char* tokens[MAX_NUM_TOKENS];
int history_count = 0;
char* history[HISTORY_SIZE];

// Function to parse the input line into individual tokens
int tokenize_input(char* line, char** tokens, int* num_tokens) {
    int i = 0;
    *num_tokens = 0;
    char* tempLine = malloc(sizeof(char) * (strlen(line) + 1));
    strcpy(tempLine, line);
    char* token = strtok(tempLine, " \t\n\r");      //Tokenize input using whitespace characters as delimiters
    while (token != NULL) {
        if (i >= MAX_NUM_TOKENS) {
            fprintf(stderr, "Error: Maximum number of tokens exceeded\n");
            exit(EXIT_FAILURE);
        }
        tokens[i] = malloc(sizeof (char) * (strlen(token) + 1));
        tokens[i] = token;
        i++;
        token = strtok(NULL, " \t\n\r");
    }
    tokens[i] = NULL;
    *num_tokens = i;
    return i ;
}

void add_history(char* line) {
    if (history_count == HISTORY_SIZE) {
        free(history[0]);
        for (int i = 0; i < HISTORY_SIZE - 1; i++) {
            history[i] = history[i + 1];
        }
        history_count--;
    }
    history[history_count++] = strdup(line);
}

void print_history() {
    for (int i = 0; i < history_count; i++) {
        printf("%d: %s\n", i + 1, history[i]);
    }
}

int execute_command(char** tokens, int num_tokens, char* line) {

    if (num_tokens == 0) {
        return 0;
    }

    // Now I will check what kind of command the user gave
    int input_fd = -1;
    int output_fd = -1;
    if (handle_redirection(tokens, num_tokens, &input_fd, &output_fd) != 0) {   // check for redirection operators
        return 1;
    } else if (strcmp(tokens[0], "cd") == 0) {
        handle_cd_command(tokens, num_tokens);
    } else if (strcmp(tokens[0], "exit") == 0) {
        return 1;
    } else if (strcmp(tokens[0], "history") == 0) {
        if (num_tokens == 2){           // means it's history and a number of the command the user wants to execute
            int x = atoi(tokens[1]);
            printf("%s\n", history[x - 1]);     // print the name of the command
            char *command = history[x - 1];
            if (tokenize_input(command, tokens, &num_tokens)) {
                if (strstr(command, "|")) {
                    handle_pipe_command(tokens, num_tokens);
                }
                expand_wildcards(tokens, &num_tokens);
                int should_exit = execute_command(tokens, num_tokens, line);
                if (should_exit) {
                    return 1;
                }
            } 
        } else {
            print_history();
        }
        return 0;
    } else if (strcmp(tokens[num_tokens - 1], "&") == 0) {      // check if command is to be executed in the background
        tokens[num_tokens - 1] = NULL; // remove "&" from arguments
        pid_t pid = fork();             // fork a new process
        if (pid < 0) {                  
            printf("Fork failed.\n");
            exit(1);
        } else if (pid == 0) {          // child process
            execvp(tokens[0], tokens);
            printf("Invalid command.\n");
            exit(1);
        } else {                        // parent process
            printf("Process %d is running in the background.\n", pid);
            return 0;
        }
    } else if (strcmp(tokens[0], "createalias") == 0) {
        char* value = extract_quoted_string(line);
        create_alias(tokens[1], value);
    } else if (strcmp(tokens[0], "destroyalias") == 0) {
        destroy_alias(tokens[1]);
        return 0;
    } else if (num_tokens == 1){  // num_tokens == 1 will mean that the command is an alias
        
        char* alias_command = get_alias_command(tokens[0]);
        
        if (alias_command != NULL) {           // if alias is found
            line = alias_command;
            execute_alias_command(alias_command, num_aliases);
        }

    }

    
    pid_t pid = fork();
    if (pid == 0) {
        // child process
        // Set up input redirection if necessary
        if (input_fd != STDIN_FILENO) {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }
        // Set up output redirection if necessary
        if (output_fd != STDOUT_FILENO) {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }
        execvp(tokens[0], tokens);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    } else {
        waitpid(pid, NULL, 0);
    }

    return 0;
}
