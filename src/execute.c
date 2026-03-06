#define _GNU_SOURCE

#include "execute.h"
#include "builtins.h"
#include "utils.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define ARGS_BUF_SIZE 1024

void execute_single_command(command_t command) {
    if (!command || !command[0])
        return;

    // Apply all redirections found in the command line
    if (apply_all_redirections(command) == -1) {
        exit(EXIT_FAILURE);
    }

    // Check if the command is a builtin
    builtin_func_t builtin_func = get_builtin_func(command[0]);
    if (builtin_func != NULL) {
        builtin_func(command);

        // Builtins execute in a subshell, so we exit afterwards
        exit(EXIT_SUCCESS);
    }

    // Otherwise, look for a matching program in the PATH
    char* full_path = find_executable(command[0]);
    if (full_path) {
        execv(full_path, command);
        log_error("Call to execv() failed");
        free(full_path);

        // The running program is replaced when calling exec functions,
        // so this should never be reached
        exit(EXIT_FAILURE);
    } else {
        fprintf(stderr, "%s: command not found\n", command[0]);
        exit(127);
    }
}