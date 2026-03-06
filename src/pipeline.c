#include "pipeline.h"
#include "execute.h"
#include "utils.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void execute_pipeline(char** args) {
    int num_cmds = 0;

    command_t* commands = segment_pipeline(args, &num_cmds);

    int in_fd = STDIN_FILENO;   // The first command reads from stdin
    int pipe_fds[2];

    for (int i = 0; i < num_cmds; i++) {
        // Unless it's the last command, we create a pipe
        if (i < num_cmds - 1) {
            if (pipe(pipe_fds) == -1) {
                log_error("Failed to create a pipe");
                return;
            }
        }

        pid_t pid = fork();
        if (pid == 0) {
            // We are in the child process

            // Unless it's the first command, redirect the input to be the read end of the pipe
            if (in_fd != STDIN_FILENO) {
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }

            // Unless it's the last command, redirect the output to be the write end of the pipe
            if (i < num_cmds - 1) {
                dup2(pipe_fds[1], STDOUT_FILENO);
                close(pipe_fds[1]);
            }

            // Execute the current command
            execute_single_command(commands[i]);
            // Terminate the child process after it is done executing the command
            exit(EXIT_SUCCESS);
        } else {
            // We are in the parent process

            // Close the read end of the pipe for the parent process
            if (in_fd != STDIN_FILENO) {
                close(in_fd);
            }

            // Close the write end of the pipe (it is already inherited by the child)
            if (i < num_cmds - 1) {
                close(pipe_fds[1]);
                in_fd = pipe_fds[0];   // Set the input fd for the next process
            }
        }
    }

    // Wait for all children to terminate
    for (int i = 0; i < num_cmds; i++) {
        wait(NULL);
    }

    free(commands);
}

command_t* segment_pipeline(char** args, int* num_commands) {
    if (!args || !num_commands) {
        log_error("parameters are NULL!");
        return NULL;
    }

    if (!args[0]) {
        log_error("No command in arguments!");
        return NULL;
    }

    command_t* commands = malloc(128 * sizeof(char*));

    int cmd_count = 0;
    commands[cmd_count++] = &args[0];

    for (size_t i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            args[i] = NULL;
            commands[cmd_count++] = &args[i + 1];
        }
    }

    *num_commands = cmd_count;

    return commands;
}