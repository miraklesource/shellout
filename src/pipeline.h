#ifndef PIPELINE_H
#define PIPELINE_H

typedef char** command_t;

/**
 * @brief Splits args into separate commands delimited by pipes (|)
 * @param args Array of commands to split. Arguments containing "|" are set to NULL
 * @param num_commands After calling the function, num_commands stores the number of separate commands found
 * @return Returns a dynamically allocated array containing pointers to the start of each command.
 * MEMORY SHOULD BE FREED BY THE CALLER!
 */
command_t* segment_pipeline(char** args, int* num_commands);

void execute_pipeline(char** args);

#endif