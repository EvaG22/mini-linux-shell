#include "functions.h"
#include "basic_functions.h"

int main() {

    char* line = (char*) malloc(MAX_INPUT_SIZE);
    int num_tokens = 0;

    // register signal handlers
    signal(SIGINT, handle_sigint);
    signal(SIGTSTP, handle_sigtstp);

    while (1) {

        printf("\033[0;34m"); // I'm just setting the color to blue to make it pretty
        printf("in-mysh-now:> ");
        printf("\033[0m");

        if (!fgets(line, MAX_INPUT_SIZE, stdin)) {
            break;
        }   

        line[strcspn(line, "\n")] = '\0'; // remove newline character
        add_history(line);

        if (tokenize_input(line, tokens, &num_tokens) > 0) {
            if (strstr(line, "|")) {
                // check for pipes
                int num_pipes = 0;
                num_pipes = count_pipes(tokens, num_tokens);
                if (num_pipes > 0) {
                    // handle pipes
                    handle_pipe_command(tokens, num_tokens);
                }
            }
            expand_wildcards(tokens, &num_tokens);
            int should_exit = execute_command(tokens, num_tokens, line);
            if (should_exit) {
                break;
            }
        }
    }

    free(line);

    return 0;
}
