#ifndef SLASH_REDIRECTION_H
#define SLASH_REDIRECTION_H

#include "lineTreatment.h"

/**
 * Handle a line with redirections (or not) with an internal command
 * @param list the line to execute the command from
 */
void handle_redirection_intern(cmd_struct cmd);

/**
 * Handle a line with redirections (or not) with an external command
 * @param list the line to execute the command from
 */
void handle_redirection_extern(cmd_struct cmd);

#endif //SLASH_REDIRECTION_H
