#define _GNU_SOURCE

#include "builtins.h"
#include "utils.h"

#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

builtin_t all_builtins[] = {
    {"echo", handle_echo}, {"cd", handle_cd},     {"exit", handle_exit},
    {"pwd", handle_pwd},   {"type", handle_type},
};

size_t num_builtins = sizeof(all_builtins) / sizeof(all_builtins[0]);

builtin_func_t get_builtin_func(const char* cmd) {
    for (size_t i = 0; i < num_builtins; i++) {
        if (strcmp(cmd, all_builtins[i].name) == 0) {
            return all_builtins[i].func;
        }
    }
    return NULL;
}

void handle_cd(char** args) {
    char* target = args[1];

    if (!target) {
        target = getenv("HOME");
        if (!target) {
            fprintf(stderr, "cd: HOME not set\n");
            return;
        }
    }

    char* dynamic_path = NULL;
    if (target[0] == '~') {
        char* home = getenv("HOME");
        if (home) {
            if (asprintf(&dynamic_path, "%s%s", home, target + 1) == -1) {
                log_error("Failed to copy directory path");
                return;
            }
            target = dynamic_path;
        }
    }

    // Check if the supplied path is a valid directory
    struct stat statbuf;
    if (stat(target, &statbuf) != 0) {
        fprintf(stderr, "cd: %s: No such file or directory\n", target);
    } else if (!S_ISDIR(statbuf.st_mode)) {
        fprintf(stderr, "cd: %s: Not a directory\n", target);
    } else {
        if (chdir(target) == -1) {
            log_error("Failed to change directory to: %s", target);
        }
    }

    free(dynamic_path);
}

void handle_echo(char** args) {
    for (int i = 1; args[i] != NULL; i++) {
        printf("%s%s", args[i], (args[i + 1] != NULL) ? " " : "");
    }
    printf("\n");
}

void handle_exit(char** unused) {
    (void) unused;

    exit(EXIT_SUCCESS);
}

void handle_pwd(char** unused) {
    (void) unused;

    char path_buf[PATH_MAX];
    char* cwd = getcwd(path_buf, sizeof(path_buf));
    if (cwd != NULL) {
        printf("%s\n", cwd);
    } else {
        log_error("Failed to get current working directory");
        exit(EXIT_FAILURE);
    }
}

void handle_type(char** args) {
    if (!args[1]) {
        fprintf(stderr, "type: missing operand\n");
        return;
    }

    char* cmd_to_check = args[1];

    if (get_builtin_func(cmd_to_check) != NULL) {
        printf("%s is a shell builtin\n", cmd_to_check);
    } else {
        char* path = find_executable(cmd_to_check);
        if (path != NULL) {
            printf("%s is %s\n", cmd_to_check, path);
            free(path);
        } else {
            printf("%s: not found\n", cmd_to_check);
        }
    }
}