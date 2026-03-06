#define _XOPEN_SOURCE 500

#include "builtins.h"
#include "completions.h"
#include "execute.h"
#include "pipeline.h"
#include "utils.h"

#include <readline/history.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    // Register cleanup functions for the autocompletion system
    if (atexit(cleanup_completions) != 0) {
        log_error("Failed to register cleanup_completions with at_exit()");
        exit(EXIT_FAILURE);
    }

    char* line;
    char* args[256];

    rl_attempted_completion_function = command_completion;

    while (1) {
        line = readline("$ ");

        // Handle Ctrl+D (EOF)
        if (!line) {
            printf("\n");
            break;
        }

        if (*line != '\0') {
            add_history(line);
            tokenize(line, args);

            if (args[0] == NULL) {
                free(line);
                continue;
            }

            // Check if the command contains at least one pipe
            int has_pipe = 0;
            for (int i = 0; args[i] != NULL; i++) {
                if (strcmp(args[i], "|") == 0) {
                    has_pipe = 1;
                    break;
                }
            }

            if (has_pipe) {
                // Pipelines create their own child processes
                execute_pipeline(args);
            } else {
                // Check if it is a builtin that MUST run in the parent (cd, exit)
                // because they modify the shell state itself.
                if (strcmp(args[0], "cd") == 0 || strcmp(args[0], "exit") == 0) {
                    builtin_func_t builtin_func = get_builtin_func(args[0]);
                    if (builtin_func) {
                        builtin_func(args);
                    }
                } else {
                    // For everything else (external commands or other builtins),
                    // we fork and use a centralized execution function.
                    pid_t pid = fork();
                    if (pid == -1) {
                        log_error("fork() failed");
                    } else if (pid == 0) {
                        // Child process
                        execute_single_command(args);

                        // execute_single_command calls exit(), so this should never be reached
                        exit(EXIT_FAILURE);
                    } else {
                        // Parent process waits for the command to finish
                        wait(NULL);
                    }
                }
            }
        }
        free(line);
    }

    return 0;
}