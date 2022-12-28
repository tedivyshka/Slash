#ifndef SLASH_LINETREATMENT_H
#define SLASH_LINETREATMENT_H

#include "utilities.h"
#include "commands.h"
#include "dirent.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/***
 * Turns a line into a command structure.
 * @param ligne line from the prompt
 * @return struct cmds_struct
 */
cmd_struct lexer(char* ligne);

void joker_solo_asterisk(cmd_struct liste);

/**
 * Compare a string to multiple redirection signs (<,>,>|,>>,2>,2>|,2>>)
 * @param str the string to compare to
 * @return boolean: 1=true 0=false
 */
int strcmp_redirections(char* str);

#endif //SLASH_LINETREATMENT_H
