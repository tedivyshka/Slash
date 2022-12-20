/***
 * A function to set the signal behavior of the program as wanted :
 * ignore SIGINT and SIGTERM
 */
void initSignals();

/***
 * A function to reset the signal behavior of child process for external commands.
 */
void defaultSignals();
