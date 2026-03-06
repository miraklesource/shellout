#ifndef UTILS_H
#define UTILS_H

#define log_error(fmt, ...) _log_error_internal(__FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)

void _log_error_internal(const char* file, const char* func, int line, const char* fmt, ...);

typedef struct {
    char* file;
    int flags;
    int redirected_stream;
} redirection_t;

/**
 * @brief Scans args for redirection operators and applies them.
 * @param args arguments of a command to scan for redirection operators
 * @return Returs 0 on success or -1 if a syntax error was detected
 */
int apply_all_redirections(char** args);

/**
 * @brief Looks for an executable program in the PATH environment variable
 * @param name Name of the program to search for
 * @return Returns a dynamically allocated string containing the full path of the program or NULL if no
 * matches were found. MEMORY SHOULD BE FREED BY THE CALLER!
 */
char* find_executable(const char* name);

/**
 * @brief Gets the directories contained in the PATH environment variable
 * @return Returns a dynamically allocated array of directories in the PATH.
 * MEMORY SHOULD BE FREED BY THE CALLER!
 */
char** get_path_dirs();

/**
 * @brief Tokenizes the line entered by the user and stores the result in args
 * @param line Line of input to tokenize
 * @param args Array of strings large enough to store all arguments
 */
void tokenize(char* line, char** args);

#endif