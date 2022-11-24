#define MAX_ARGS_NUMBER 4096
#define MAX_ARGS_STRLEN 4096
#define BUFSIZE 1024

typedef struct cmds_struct{
    char** cmds_array;
    size_t taille_array;
}cmds_struct;


/***
 * An implementation of the intern command cd.
 * Change the working directory.
 * This function is commented throughout to explain how it works locally.
 * @param option for cd call. "-P", "-L" or ""
 * @param path relative or absolute path
 * @return 0. return 1 if error encountered
 */
int process_cd(char * option, char * path);


/***
 * interprets the cd arguments to call process_cd with good parameters
 * @param liste struct for the command
 */
void process_cd_call(cmds_struct liste);


/***
 * interprets the pwd arguments to call get_cwd with good parameters or print
 * global variable pwd
 * @param liste struct for the command
 */
void process_pwd_call(cmds_struct liste);


/***
 * interprets the exit arguments to call exit() with good value
 * @param liste struct for the command
 */
void process_exit_call(cmds_struct liste);


/***
 * interprets the commands to call the corresponding functions.
 * @param liste struct for the command
 */
void interpreter(cmds_struct liste);


/***
 * turns a line into a command structure
 * @param ligne line from the prompt
 * @return struct cmds_struct
 */
cmds_struct lexer(char* ligne);
