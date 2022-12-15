#ifndef SLASH_COMMANDS_H
#define SLASH_COMMANDS_H

#include "utilities.h"
#include <errno.h>

/***
 * Interprets the cd arguments to call process_cd with good parameters.
 * @param liste struct for the command
 */
void process_cd_call(cmds_struct liste);


/***
 * Interprets the pwd arguments to call get_cwd with good parameters or print global variable pwd.
 * @param liste struct for the command
 */
void process_pwd_call(cmds_struct liste);


/***
 * Interprets the exit arguments to call exit() with good value.
 * @param liste struct for the command
 */
void process_exit_call(cmds_struct liste);

/***
 * Runs external command
 * @param liste struct for the command
 */
void process_external_command(cmds_struct liste);

#endif //SLASH_COMMANDS_H