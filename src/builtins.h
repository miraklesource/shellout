#ifndef BUILTINS_H
#define BUILTINS_H

#include <stddef.h>

// Function pointer to dispatch calls to builtins
typedef void (*builtin_func_t)(char**);

typedef struct {
    const char* name;
    builtin_func_t func;
} builtin_t;

/**
 * @brief Retrieves the builtin function corresponding to cmd
 * @param cmd Command to search for in the builtins
 * @return Function pointer to the corresponding bultin function or NULL if no matches are found
 */
builtin_func_t get_builtin_func(const char* cmd);

// Make these globals accessible to the outside world
extern builtin_t all_builtins[];
extern size_t num_builtins;

// Function prototypes for builtins
void handle_cd(char** args);
void handle_echo(char** args);
void handle_exit(char** unused);
void handle_pwd(char** unused);
void handle_type(char** args);

#endif