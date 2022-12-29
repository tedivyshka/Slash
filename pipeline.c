#include "pipeline.h"

static char* input_redir;
static char** output_redir;
static char** error_redir;

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
            process_external_command_pipeline(list,pid);
        }
        exit(errorCode);
    }
}

char* in_redir(cmd_struct cmd){
    for(int i=0;i<cmd.taille_array;i++){
        if(strcmp(*(cmd.cmd_array+i),"<")==0){
            if(i+1==cmd.taille_array) perror_exit("input redirection");
            return *(cmd.cmd_array+i+1);
        }
    }
    return "";
}

char** out_redir(cmd_struct cmd){
    char** res= malloc(sizeof(char*)*2);
    for(int i=0;i<cmd.taille_array;i++){
        if((strcmp(*(cmd.cmd_array+i),">")==0 || strcmp(*(cmd.cmd_array+i),">|")==0 || strcmp(*(cmd.cmd_array+i),">>")==0)){
            if(i+1==cmd.taille_array) perror_exit("out redirection");
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

char** err_redir(cmd_struct cmd){
    char** res=malloc(sizeof(char*)*2);
    for(int i=0;i<cmd.taille_array;i++){
        if((strcmp(*(cmd.cmd_array+i),"2>")==0 || strcmp(*(cmd.cmd_array+i),"2>|")==0 || strcmp(*(cmd.cmd_array+i),"2>>")==0)){
            if(i+1==cmd.taille_array) perror_exit("stderr redirection");
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

void handle_pipe(cmds_struct cmds){
    size_t num_commands=cmds.taille_array;
    input_redir=in_redir(*cmds.cmds_array);
    output_redir=out_redir(*(cmds.cmds_array+num_commands-1));
    int output_fd;
    int error_fd;

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
        if(child_pids[i]==0) break;
    }

    // set up the pipeline and redirections in the child processes
    for (int i=0; i<num_commands; i++) {
        if (child_pids[i]==0) {
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
                    //dprintf(STDERR_FILENO,"%s\n",*(error_redir));
                    //dprintf(STDERR_FILENO,"%s\n",*(error_redir+1));
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
        if(strcmp(*((cmds.cmds_array+i)->cmd_array),"exit")==0){
            exit(errorCode);
        }
        if(errorCode==1) break;
        if(WIFSIGNALED(status)) errorCode = -1;
    }
}
