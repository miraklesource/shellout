#include "utils.h"

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void _log_error_internal(const char* file, const char* func, int line, const char* fmt, ...) {
    va_list args;

    // Print the error in red
    fprintf(stderr, "\033[1;31m[ERROR]\033[0m [%s:%d in %s()] ", file, line, func);

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    if (errno != 0) {
        fprintf(stderr, " : %s", strerror(errno));
        errno = 0;
    }

    fprintf(stderr, "\n");
}

int apply_all_redirections(char** args) {
    for (int i = 0; args[i] != NULL; i++) {
        int flags = 0;
        int target_fd = -1;
        int stream_to_redirect = -1;

        if (strcmp(args[i], ">") == 0 || strcmp(args[i], "1>") == 0) {
            flags = O_WRONLY | O_CREAT | O_TRUNC;
            stream_to_redirect = STDOUT_FILENO;
        } else if (strcmp(args[i], ">>") == 0 || strcmp(args[i], "1>>") == 0) {
            flags = O_WRONLY | O_CREAT | O_APPEND;
            stream_to_redirect = STDOUT_FILENO;
        } else if (strcmp(args[i], "2>") == 0) {
            flags = O_WRONLY | O_CREAT | O_TRUNC;
            stream_to_redirect = STDERR_FILENO;
        } else if (strcmp(args[i], "2>>") == 0) {
            flags = O_WRONLY | O_CREAT | O_APPEND;
            stream_to_redirect = STDERR_FILENO;
        } else if (strcmp(args[i], "<") == 0) {
            flags = O_RDONLY;
            stream_to_redirect = STDIN_FILENO;
        }

        if (stream_to_redirect != -1) {
            char* filename = args[i + 1];
            if (!filename) {
                fprintf(stderr, "syntax error near unexpected token `newline'\n");
                return -1;
            }

            int fd = open(filename, flags, 0666);
            if (fd == -1) {
                perror("open");
                return -1;
            }

            if (dup2(fd, stream_to_redirect) == -1) {
                perror("dup2");
                close(fd);
                return -1;
            }
            close(fd);

            // Clean up args: remove operator and filename
            args[i] = NULL;
            args[i+1] = NULL;
            
            // Shift remaining args to fill the gap
            for (int j = i; args[j+2] != NULL; j++) {
                args[j] = args[j+2];
                args[j+1] = NULL;
            }
            
            // Re-check the current index since we shifted the array
            i--; 
        }
    }
    return 0;
}

char* find_executable(const char* name) {
    char* path_env = getenv("PATH");
    if (!path_env)
        return NULL;

    char* path_copy = strdup(path_env);
    char* current_dir = strtok(path_copy, ":");

    while (current_dir != NULL) {
        size_t len = strlen(current_dir) + strlen(name) + 2;
        char* full_path = malloc(len);
        snprintf(full_path, len, "%s/%s", current_dir, name);

        if (access(full_path, X_OK) == 0) {
            free(path_copy);
            return full_path;
        }
        free(full_path);
        current_dir = strtok(NULL, ":");
    }
    free(path_copy);
    return NULL;
}

char** get_path_dirs() {
    char* path_env = getenv("PATH");
    if (!path_env)
        return NULL;

    char* path_copy = strdup(path_env);
    char** dirs = NULL;
    int count = 0;
    char* saveptr;
    char* token = strtok_r(path_copy, ":", &saveptr);

    while (token != NULL) {
        char** tmp = realloc(dirs, sizeof(char*) * (count + 1));
        if (!tmp) {
            log_error("Memory allocation failed during PATH parsing");
            goto get_path_dirs_error;
        }
        dirs = tmp;
        dirs[count] = strdup(token);
        count++;
        token = strtok_r(NULL, ":", &saveptr);
    }

    dirs = realloc(dirs, sizeof(char*) * (count + 1));
    dirs[count] = NULL;

    free(path_copy);
    return dirs;

get_path_dirs_error:
    free(path_copy);
    for (size_t i = 0; i < count; i++) {
        free(dirs[i]);
        dirs[i] = NULL;
    }

    free(dirs);

    return NULL;
}

void tokenize(char* line, char** args) {
    int arg_count = 0;
    char* src = line;

    while (*src) {
        // Drop leading whitespace
        while (*src == ' ' || *src == '\t')
            src++;

        if (*src == '\0')
            break;

        // Save the current argument
        args[arg_count++] = src;
        char* dst = src;   // Pointer used to ignore quotes

        // Parse the argument
        while (*src && *src != ' ' && *src != '\t') {
            if (*src == '\\') {
                /*
                 * Backslash outside of quotes: Ignore the special meaning of the next character and use
                 * the raw character instead
                 */
                src++;
                if (*src) {
                    *dst++ = *src++;
                }

                // Move on to the next iteration to avoid handling the same character twice
                continue;
            }

            if (*src == '\'') {
                // Single quotes: treat everything inside literally

                src++;   // Skip the leading '
                while (*src && *src != '\'') {
                    *dst++ = *src++;
                }
                if (*src == '\'')
                    src++;   // Skip the trailing '

            } else if (*src == '\"') {
                // Double quotes: handle escape characters

                src++;   // Skip the leading "

                while (*src && *src != '\"') {
                    if (*src == '\\' && (*(src + 1) == '\"' || *(src + 1) == '\\')) {
                        src++;   // Skip escaping backslash
                    }
                    *dst++ = *src++;
                }
                if (*src == '\"')
                    src++;   // Skip the trailing "
            } else {
                // Normal data:
                *dst++ = *src++;
            }
        }

        // If we are done handling the current argument but the line has more content
        if (*src)
            src++;

        // Terminate the current argument
        *dst = '\0';
    }

    // The args array must end with NULL to be passed to exec() functions
    args[arg_count] = NULL;
}