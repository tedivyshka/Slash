#ifndef SLASH_LINETREATMENT_H
#define SLASH_LINETREATMENT_H

#include "utilities.h"
#include "commands.h"
#include "dirent.h"

/***
 * Turns a line into a command structure.
 * @param ligne line from the prompt
 * @return struct cmds_struct
 */
cmds_struct lexer(char* ligne);

void joker_solo_asterisk(cmds_struct liste);

#endif //SLASH_LINETREATMENT_H
