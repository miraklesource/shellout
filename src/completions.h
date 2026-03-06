#ifndef COMPLETIONS_H
#define COMPLETIONS_H

/**
 * @brief Generator function for readline completions
 */
char* builtin_generator(const char* text, int state);

/**
 * @brief Cleanup function to free resources allocated by command_completion
 * Called on exit
 */
void cleanup_completions(void);

/**
 * @brief Used to enable custom command completion for readline
 */
char** command_completion(const char* text, int start, int end);

#endif