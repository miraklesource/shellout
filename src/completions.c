#include "completions.h"
#include "builtins.h"
#include "utils.h"

#include <dirent.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static char** global_path_dirs;

void cleanup_completions(void) {
    if (!global_path_dirs)
        return;

    for (size_t i = 0; global_path_dirs[i] != NULL; i++) {
        free(global_path_dirs[i]);
        global_path_dirs[i] = NULL;
    }

    free(global_path_dirs);
    global_path_dirs = NULL;
}

char* command_generator(const char* text, int state) {
    static size_t builtin_index;
    static int path_index;
    static DIR* directory;
    static size_t len;

    if (!state) {
        builtin_index = 0;
        path_index = 0;
        len = strlen(text);
        if (directory) {
            closedir(directory);
            directory = NULL;
        }

        // We only reload the PATH if it's NULL to avoid leaks
        if (global_path_dirs == NULL) {
            global_path_dirs = get_path_dirs();
        }
    }

    // Look for a matching builtin first
    while (builtin_index < num_builtins) {
        const char* name = all_builtins[builtin_index++].name;
        if (strncmp(name, text, len) == 0)
            return strdup(name);
    }

    // Check in PATH for a matching command
    while (global_path_dirs && global_path_dirs[path_index] != NULL) {
        if (directory == NULL) {
            directory = opendir(global_path_dirs[path_index]);
            if (directory == NULL) {
                path_index++;
                continue;
            }
        }

        struct dirent* entry = readdir(directory);
        if (entry == NULL) {
            closedir(directory);
            directory = NULL;
            path_index++;
            continue;
        }

        if (strncmp(entry->d_name, text, len) == 0) {
            // TODO: check access(full_path, X_OK) to be rigorous
            return strdup(entry->d_name);
        }
    }

    return NULL;   // No completion found
}

char** command_completion(const char* text, int start, int end) {
    // Disable filename completion for the command name
    rl_attempted_completion_over = 1;

    if (start == 0) {
        return rl_completion_matches(text, command_generator);
    }

    // For arguments other than the first, we enable filename completion
    rl_attempted_completion_over = 0;

    return NULL;
}