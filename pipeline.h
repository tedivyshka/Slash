#ifndef SLASH_PIPELINE_H
#define SLASH_PIPELINE_H

#include "lineTreatment.h"


extern int syntax_error;

/**
 * Handle the pipes in a command line iteratively.\n
 * We first get the input and output redirections informations.
 * We create an array of pipes and initialize them.
 * We create an array to store the childs pids.
 * Then for each command we dup the file descriptors for each redirection and each pipe, and then execute the command.
 * The parent wait for each child, check for the signal in the child process status and exit slash if 'exit' was called in the line.
 *
 * @param cmds the line to handle pipe from
 */
void handle_pipe(cmds_struct cmds);

#endif //SLASH_PIPELINE_H
