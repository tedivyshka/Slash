#include "pipeline.h"

static char* input_redir;
static char** output_redir;
static char** error_redir;

int syntax_error=0;

/**
 * Execute a command in a pipeline (already in a fork)
 * @param list the list of commands
 * @param pid execute only commands from child
 */
void exec_command_pipeline(cmd_struct list,pid_t pid){
    if(pid==0){
        defaultSignals();
        if(strcmp(*list.cmd_array,"cd")==0){
            process_cd_call(list);
        }
        else if(strcmp(*list.cmd_array,"pwd")==0){
            process_pwd_call(list);
        }
        else if(strcmp(*list.cmd_array,"exit")==0){
            process_exit_call(list);
        }
        else{
            process_external_command(list);
        }
        exit(errorCode);
    }
}

/**
 * Returns the input redirection filename or an empty string if there's none
 * @param cmd the command to check input redirection from
 * @return the redirection filename or an empty string
 */
char* in_redir(cmd_struct cmd){
    for(int i=0;i<cmd.taille_array;i++){
        if(strcmp(*(cmd.cmd_array+i),"<")==0){
            // check for syntax error
            if(i+1==cmd.taille_array || strcmp_redirections(*(cmd.cmd_array+i+1))==1 || strcmp(*(cmd.cmd_array+i+1),"|")==0){
                syntax_error=1;
                errorCode=2;
            }
            return *(cmd.cmd_array+i+1);
        }
    }
    return "";
}

/**
 * Returns the output redirection symbol and the filename or an empty string if there's none
 * @param cmd the command to check output redirection from
 * @return the redirection symbol and the filename or an empty string
 */
char** out_redir(cmd_struct cmd){
    char** res= malloc(sizeof(char*)*2);
    for(int i=0;i<cmd.taille_array;i++){
        if((strcmp(*(cmd.cmd_array+i),">")==0 || strcmp(*(cmd.cmd_array+i),">|")==0 || strcmp(*(cmd.cmd_array+i),">>")==0)){
            // check for syntax error
            if(i+1==cmd.taille_array || strcmp_redirections(*(cmd.cmd_array+i+1))==1 || strcmp(*(cmd.cmd_array+i+1),"|")==0){
                syntax_error=1;
                errorCode=2;
            }
            *res=malloc(sizeof(char)*strlen(*(cmd.cmd_array+i))+1);
            *(res+1)=malloc(sizeof(char)*strlen(*(cmd.cmd_array+i+1))+1);
            strcpy(*res,*(cmd.cmd_array+i));
            strcpy(*(res+1),*(cmd.cmd_array+i+1));
            return res;
        }
    }
    *res="";
    return res;
}

/**
 * Returns the error redirection symbol and the filename or an empty string if there's none
 * @param cmd the command to check error redirection from
 * @return the redirection symbol and the filename or an empty string
 */
char** err_redir(cmd_struct cmd){
    char** res=malloc(sizeof(char*)*2);
    for(int i=0;i<cmd.taille_array;i++){
        if((strcmp(*(cmd.cmd_array+i),"2>")==0 || strcmp(*(cmd.cmd_array+i),"2>|")==0 || strcmp(*(cmd.cmd_array+i),"2>>")==0)){
            // check for syntax error
            if(i+1==cmd.taille_array || strcmp_redirections(*(cmd.cmd_array+i))==1 || strcmp(*(cmd.cmd_array+i),"|")==0){
                syntax_error=1;
                errorCode=2;
            }
            *res=malloc(sizeof(char)*strlen(*(cmd.cmd_array+i))+1);
            *(res+1)=malloc(sizeof(char)*strlen(*(cmd.cmd_array+i+1))+1);
            strcpy(*res,*(cmd.cmd_array+i));
            strcpy(*(res+1),*(cmd.cmd_array+i+1));
            return res;
        }
    }
    *res="";
    return res;
}

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
void handle_pipe(cmds_struct cmds){
    size_t num_commands=cmds.taille_array;
    input_redir=in_redir(*cmds.cmds_array);
    output_redir=out_redir(*(cmds.cmds_array+num_commands-1));
    int output_fd;
    int error_fd;

    if(syntax_error==1){
        dprintf(STDERR_FILENO,"slash: syntax error\n");
        errorCode=2;
        return;
    }

    // create a pipe for each pair of adjacent commands
    size_t num_pipes=num_commands-1;
    int pipes[num_pipes][2];
    for (int i=0; i<num_pipes; i++) {
        if (pipe(pipes[i])<0) {
            perror_exit("pipe");
        }
    }

    // create a child process for each command
    pid_t child_pids[num_commands];
    for (int i=0; i<num_commands; i++) {
        child_pids[i]=fork();
        if (child_pids[i]<0) {
            perror_exit("fork");
        }
        // we exit the 'for' loop for the childs
        if(child_pids[i]==0) break;
    }

    // set up the pipeline and redirections in the child processes
    for (int i=0; i<num_commands; i++) {
        if (child_pids[i]==0) {
            // when there's two pipes next to each other, we exit the child process
            if(strcmp(*((cmds.cmds_array+i)->cmd_array),"\0")==0){
                dprintf(STDERR_FILENO,"slash: syntax error near unexpected token '|'\n");
                errorCode=2;
                exit(errorCode);
            }
            // redirect standard input
            if (i==0){
                if(strcmp(input_redir,"")!=0) {
                    // redirect standard input from a file
                    int input_fd=open(input_redir,O_RDONLY, 0666);
                    if (input_fd<0){
                        perror_exit("open");
                    }
                    dup2(input_fd,STDIN_FILENO);
                    close(input_fd);
                }
            }
            // redirect the write end of the last pipe
            else if (i>0) {
                dup2(pipes[i-1][0],STDIN_FILENO);
            }
            // redirect the read end of the current pipe
            if (i<num_commands-1) {
                dup2(pipes[i][1],STDOUT_FILENO);
            }
            // redirect standard output
            else if(strcmp(*output_redir,"")!=0){
                // redirect standard output to a file
                if(strcmp(*output_redir,">") == 0){
                    output_fd = open(*(output_redir+1), O_WRONLY | O_CREAT | O_EXCL, 0666);
                }
                else if(strcmp(*output_redir,">>") == 0){
                    output_fd = open(*(output_redir+1), O_WRONLY | O_CREAT | O_APPEND, 0666);
                }
                else if(strcmp(*output_redir,">|") == 0){
                    output_fd = open(*(output_redir+1), O_WRONLY | O_CREAT | O_TRUNC, 0666);
                }

                if (output_fd<0) {
                    perror_exit("open");
                }
                dup2(output_fd,STDOUT_FILENO);
                close(output_fd);
            }

            //redirect error output
            error_redir=err_redir((*(cmds.cmds_array+i)));
            if(strcmp(*error_redir,"")!=0){
                // redirect error output to a file
                if(strcmp(*error_redir,"2>")==0){
                    error_fd=open(*(error_redir+1),O_WRONLY | O_CREAT | O_EXCL,0666);
                }
                else if(strcmp(*error_redir,"2>>")==0){
                    error_fd=open(*(error_redir+1),O_WRONLY | O_CREAT | O_APPEND,0666);
                }
                else if(strcmp(*error_redir,"2>|")==0){
                    error_fd=open(*(error_redir+1),O_WRONLY | O_CREAT | O_TRUNC,0666);
                }
                if(error_fd<0){
                    perror_exit("open");
                }
                dup2(error_fd,STDERR_FILENO);
                close(error_fd);
            }

            // close all unnecessary pipe ends
            for (int j=0; j<num_pipes; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // execute the command
            cmd_struct removed_cmd=remove_redirections(*(cmds.cmds_array+i));
            exec_command_pipeline(removed_cmd,child_pids[i]);
            perror_exit("execvp");
        }
    }

    // close all unnecessary pipe ends in the parent process
    for (int i=0; i<num_pipes; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // wait for all child processes to complete
    for (int i=0; i<num_commands; i++) {
        int status;
        waitpid(child_pids[i], &status, 0);
        errorCode = WEXITSTATUS(status);
        // exit 'slash' if an exit call was in the line
        if(strcmp(*((cmds.cmds_array+i)->cmd_array),"exit")==0){
            exit(errorCode);
        }
        if(WIFSIGNALED(status)) errorCode = -1;
    }
}
