#ifndef EXECUTE_H
#define EXECUTE_H

#include "pipeline.h"

/**
 * @brief Executes a command which can either be a builtin or an external program
 * @param command Command to execute
 */
void execute_single_command(command_t command);

#endif