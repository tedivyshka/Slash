#include "redirection.h"

static int saved[3]={-1,-1,-1};
static int duped[3]={0,1,2};

void reset_redi(){
    for(int i=0; i<3; i++){
        if(saved[i]!=-1)
            dup2(saved[i],duped[i]);
        close(saved[i]);
        saved[i]=-1;
    }
}

void exec_command_redirection(cmd_struct list){
    if(strcmp(*list.cmd_array,"cd")==0){
        process_cd_call(list);
    }
    else if(strcmp(*list.cmd_array,"pwd")==0){
        process_pwd_call(list);
    }
    else{
        process_exit_call(list);
    }
    fflush(stdout);
    reset_redi();
}

void exec_command_extern(cmd_struct list,pid_t pid){
    defaultSignals();
    process_external_command_pipeline(list,pid);
    exit(errorCode);
}

void handle_redirection_intern(cmd_struct cmd){
    char** line=cmd.cmd_array;
    int in_flags=O_RDONLY;
    int out_flags;
    int err_flags;

    int i=0;
    while(i<cmd.taille_array){
        // input redirection
        if(strcmp(*(line+i),"<")==0){
            i++;
            int in_fd=open(*(line+i),in_flags,0666);
            if(in_fd<0){
                errorCode=1;
                return;
            }
            saved[0]=dup(0);
            if(dup2(in_fd,STDIN_FILENO)<0) perror_exit("dup2");
            if(close(in_fd)<0) perror_exit("close");
        }
        // output redirection
        else if(strcmp(*(line+i),">")==0 || strcmp(*(line+i),">|")==0 || strcmp(*(line+i),">>")==0){
            if(strcmp(*(line+i),">")==0){
                out_flags=O_WRONLY | O_CREAT | O_EXCL;
            }
            else if(strcmp(*(line+i),">>")==0){
                out_flags=O_WRONLY | O_CREAT | O_APPEND;
            }
            else if(strcmp(*(line+i),">|")==0){
                out_flags=O_WRONLY | O_CREAT | O_TRUNC;
            }
            i++;
            int out_fd=open(*(line+i),out_flags, 0666);
            if(out_fd<0){
                errorCode=1;
                return;
            }
            saved[1]=dup(1);
            if(dup2(out_fd,STDOUT_FILENO)<0) perror_exit("dup2");
            if(close(out_fd)<0) perror_exit("close");
        }
        // error redirection
        else if(strcmp(*(line+i),"2>")==0 || strcmp(*(line+i),"2>|")==0 || strcmp(*(line+i),"2>>")==0){
            if(strcmp(*(line+i),"2>")==0){
                err_flags=O_WRONLY | O_CREAT | O_EXCL;
            }
            else if(strcmp(*(line+i),"2>>")==0){
                err_flags=O_WRONLY | O_CREAT | O_APPEND;
            }
            else if(strcmp(*(line+i),"2>|")==0){
                err_flags=O_WRONLY | O_CREAT | O_TRUNC;
            }
            i++;
            int err_fd=open(*(line+i),err_flags,0666);
            if(err_fd<0){
                errorCode=1;
                return;
            }
            saved[2]=dup(2);
            if(dup2(err_fd,STDERR_FILENO)<0) perror_exit("dup2");
            if(close(err_fd)<0) perror_exit("close");
        }
        i++;
    }
    exec_command_redirection(cmd);
}

void handle_redirection_extern(cmd_struct cmd){
    char** line=cmd.cmd_array;
    int in_flags=O_RDONLY;
    int out_flags;
    int err_flags;

    fflush(stdout);
    pid_t pid=fork();

    if(pid==-1){
        perror_exit("fork");
    }
    else if(pid==0){
        for(int i=0; i<cmd.taille_array; ++i){
            // input redirection
            if(strcmp(*(line+i),"<")==0){
                i++;
                int in_fd=open(*(line+i),in_flags,0666);
                if(in_fd<0){
                    errorCode=1;
                    exit(errorCode);
                }
                saved[0]=dup(0);
                if(dup2(in_fd,STDIN_FILENO)<0) perror_exit("dup2");
                if(close(in_fd)<0) perror_exit("close");
            }
            // output redirection
            else if(strcmp(*(line+i),">")==0 || strcmp(*(line+i),">|")==0 || strcmp(*(line+i),">>")==0){
                if(strcmp(*(line+i),">")==0){
                    out_flags=O_WRONLY | O_CREAT | O_EXCL;
                }
                else if(strcmp(*(line+i),">>")==0){
                    out_flags=O_WRONLY | O_CREAT | O_APPEND;
                }
                else if(strcmp(*(line+i),">|")==0){
                    out_flags=O_WRONLY | O_CREAT | O_TRUNC;
                }
                i++;
                int out_fd=open(*(line+i),out_flags, 0666);
                // kill child if the file can't be opened
                if(out_fd<0){
                    errorCode=1;
                    perror_exit("slash");
                }
                saved[1]=dup(1);
                if(dup2(out_fd,STDOUT_FILENO)<0) perror_exit("dup2");
                if(close(out_fd)<0) perror_exit("close");
            }
            // error redirection
            else if(strcmp(*(line+i),"2>")==0 || strcmp(*(line+i),"2>|")==0 || strcmp(*(line+i),"2>>")==0){
                if(strcmp(*(line+i),"2>")==0){
                    err_flags=O_WRONLY | O_CREAT | O_EXCL;
                }
                else if(strcmp(*(line+i),"2>>")==0){
                    err_flags=O_WRONLY | O_CREAT | O_APPEND;
                }
                else if(strcmp(*(line+i),"2>|")==0){
                    err_flags=O_WRONLY | O_CREAT | O_TRUNC;
                }
                i++;
                int err_fd=open(*(line+i),err_flags,0666);
                // kill child if the file can't be opened
                if(err_fd<0){
                    errorCode=1;
                    perror_exit("slash");
                }
                saved[2]=dup(2);
                if(dup2(err_fd,STDERR_FILENO)<0) perror_exit("dup2");
                if(close(err_fd)<0) perror_exit("close");
            }
        }
        cmd_struct removed_cmd=remove_redirections(cmd);
        exec_command_extern(removed_cmd,pid);
        perror_exit("execvp");
    }
    else{
        reset_redi();
        int status;
        waitpid(pid,&status,0);
        errorCode = WEXITSTATUS(status);
        if(WIFSIGNALED(status)) errorCode = -1;
    }
}

