#include "slash.h"

/**
 * Generate the prompt for the readline call.
 * @return the prompt
 */
char* promptGeneration(){
    char* curr_path_cpy=pwd;
    size_t curr_path_size=strlen(curr_path_cpy);
    char* tmp_curr=malloc(sizeof(char)*26);
    testMalloc(tmp_curr);
    char* new_curr=malloc(sizeof(char)*26);
    testMalloc(new_curr);

    // copy only the 22 characters wanted to create a prompt of 30 characters
    if(curr_path_size>25){
        size_t wanted_size_path;
        if(errorCode == -1) wanted_size_path=20;
        else wanted_size_path=22;
        size_t beginning_size=curr_path_size-wanted_size_path;
        strcpy(tmp_curr,curr_path_cpy+beginning_size);
        sprintf(new_curr,"...%s",tmp_curr);
    }
    else{
        strcpy(new_curr,curr_path_cpy);
    }
    char* res=malloc(sizeof(char)*(55+strlen(new_curr)));
    testMalloc(res);

    if(errorCode==0){
        sprintf(res,"\001\033[32m\002[%d]\001\033[00m\002\001\033[34m\002%s\001\033[00m\002$ ",errorCode,new_curr);
    }
    else if(errorCode==-1){
      sprintf(res,"\n\001\033[91m\002[SIG]\001\033[00m\002\001\033[34m\002%s\001\033[00m\002$ ",new_curr);
    }
    else{
        sprintf(res,"\001\033[91m\002[%d]\001\033[00m\002\001\033[34m\002%s\001\033[00m\002$ ",errorCode,new_curr);
    }
    free(new_curr);
    free(tmp_curr);
    return res;
}


/**
 * Variables initialization
 */
void initVar(){
    home = malloc(MAX_ARGS_STRLEN * sizeof(char));
    testMalloc(home);
    strcpy(home,getenv("HOME"));
    pwd = malloc(MAX_ARGS_STRLEN * sizeof(char));
    testMalloc(pwd);
    strcpy(pwd, getenv("PWD"));
    oldpwd = malloc(MAX_ARGS_STRLEN * sizeof(char));
    testMalloc(oldpwd);
    if(getenv("OLDPWD") != NULL){
        strcpy(oldpwd,getenv("OLDPWD"));
    }
    chdir(pwd);
    pwdPhy = malloc(MAX_ARGS_STRLEN * sizeof(char));
    testMalloc(pwdPhy);
    getcwd(pwdPhy, MAX_ARGS_STRLEN * sizeof(char));
}


/**
 * Main loop of the program
 */
void run(){
    rl_outstream=stderr;
    initVar();
    char* ligne;
    cmd_struct liste;
    while(1){
        char* tmp=promptGeneration();
        ligne=readline(tmp);
        // checks if ligne is empty after readline return
        // exit when reached EOF
        if(ligne==NULL){
            exit(errorCode);
        }
        else if(ligne && *ligne && !is_empty(ligne)){
            if(strcmp(ligne,"\n")!=0){
                add_history(ligne);
                liste=lexer(ligne);
                joker_expansion(liste);
            }
            freeCmdArray(liste);
        }
        free(ligne);
        free(tmp);
    }
}

/**
 * Call run() function
 * @return exit value
 */
int main(void) {
    initSignals();
    run();
    exit(errorCode);
}
