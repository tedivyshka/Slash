#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "slash.h"


// Variables globales :
int errorCode=0;

char * pwd;
char * oldpwd;
char * pwdPhy;
char * home;



/***
 * checks if a malloc has failed
 * @param ptr pointer to check
 */
void testMalloc(void * ptr){
    if(ptr == NULL){
        perror("Erreur de malloc() ou realloc().\n");
        exit(EXIT_FAILURE);
    }
}

/***
 * free an instance of struct cmds_struct
 * @param array cmds_struct
 */
void freeCmdsArray(cmds_struct array) {
    for (int i = 0; i < array.taille_array; i++) {
        free(*(array.cmds_array + i));
    }
    free(array.cmds_array);
}

/***
 * double the size allocated to the variable if necessary
 * @param array string array
 * @param taille_array array size
 * @param taille_array_initiale initial array size
 * @return array parameter after reallocation
 */
char** checkArraySize(char** array,size_t taille_array,size_t* taille_array_initiale){
    if(taille_array==MAX_ARGS_NUMBER){
        perror("MAX ARGS NUMBER REACHED");
        exit(1);
    }
    if(taille_array>=*(taille_array_initiale)){
        *(taille_array_initiale)*=2;
        array=realloc(array,sizeof(char *)*(*taille_array_initiale));
        testMalloc(array);
    }
    return array;
}

/***
 * Checks that a path is valid physically.
 * @param path relative or absolute path
 * @return int val to represent boolean value. 0 = True, 1 = False.
 */
int isPathValidPhy(char * path){
    if(path[0] == '/'){
        struct stat st;
        stat(path,&st);
        return S_ISDIR(st.st_mode)?0:1;
    }
    else{
        char *newPathTmp = malloc(sizeof(char) * (strlen(pwdPhy) + strlen(path) + 2)); // on crée un chemin contenant pwd et path = newPathTmp
        sprintf(newPathTmp,"%s/%s",pwdPhy,path);
        struct stat st;
        stat(newPathTmp,&st);
        free(newPathTmp);
        return S_ISDIR(st.st_mode)?0:1;
    }
}

/***
 * Checks that a path is valid logically.
 * @param path absolute path
 * @return int val to represent boolean value. 0 = True, 1 = False.
 */
int isPathValidLo(char* path){
    struct stat st;
    int res = stat(path,&st);
    return (res == 0 && S_ISDIR(st.st_mode))?0:1;
}

/***
 *
 * @param liste
 */
void printListe(cmds_struct liste) {
    int count=0;
    int countString=0;
    char* string=*liste.cmds_array;
    char c;
    while(count!=liste.taille_array){
        c=*(string+countString);
        while(c!=EOF && c!='\0'){
            printf("%d : %c\n",count,c);
            countString++;
            c=*(string+countString);
        }
        printf("\n");
        countString=0;
        count++;
        string=*(liste.cmds_array+count);
    }
}

/***
 * An implementation of the intern command cd.
 * Change the working directory.
 * This function is commented throughout to explain how it works locally.
 * @param option for cd call. "-P", "-L" or ""
 * @param path relative or absolute path
 * @return 0. return 1 if error encountered
 */
int process_cd(char * option, char * path){
    if(strcmp(option,"-P") == 0){ // option "-P"
        if(isPathValidPhy(path) != 0) { // if path isn't valid, an error message is displayed and the return value is 1.
            char* er = malloc(sizeof(char) * (strlen(path) + 48));
            sprintf(er,"bash: cd: %s: Aucun fichier ou dossier de ce type",path);
            perror(er);
            free(er);
            return 1;
        }
        if(path[0] == '-'){ // case of "-", we return to the last directory where we were.
            char * temp = malloc(sizeof(char) * MAX_ARGS_STRLEN);
            strcpy(temp,oldpwd);
            strcpy(oldpwd,pwd);

            chdir(temp); // chdir() and getcwd() function are used to get the physical path of the parameter path.
            getcwd(pwd, MAX_ARGS_STRLEN);
            getcwd(pwdPhy,MAX_ARGS_STRLEN);
            free(temp);
        }
        else {
            strcpy(oldpwd, pwd); // oldpwd takes the old value of pwd

            if (path[0] == '/') { // Absolute path : pwdPhy and pwd take the value of the path, transformed in physical path
                chdir(path);
                getcwd(pwdPhy, BUFSIZE);
                getcwd(pwd,BUFSIZE);
            }
            else{ // Relative path : pwdPhy and pwd take the value of pwdPhy + "/" + path, transformed into physical path.
                char *newPath = malloc(sizeof(char) * ((strlen(pwdPhy) + 2 + strlen(path))));
                testMalloc(newPath);
                sprintf(newPath,"%s/%s",pwdPhy,path);
                chdir(newPath);
                getcwd(pwdPhy, BUFSIZE);
                getcwd(pwd,BUFSIZE);
                free(newPath);
            }
        }
    }
    else{ // option -L or no option
        if(path[0] == '-'){ // case of "-", we return to the last directory where we were.
            char * tmp = malloc(sizeof(char) * MAX_ARGS_STRLEN);
            strcpy(tmp,oldpwd);
            strcpy(oldpwd,pwd);
            strcpy(pwd,tmp);


            chdir(tmp); // pwdPhy takes the old value of oldpwd
            getcwd(pwdPhy,BUFSIZE); // which we transform into a physical value with chdir + getcwd
            free(tmp);
        }else { // case of relative and absolute logical path
            char * pathCopy = malloc(sizeof(char) * (strlen(path) + 1));
            testMalloc(pathCopy);
            strcpy(pathCopy,path); // We create a copy of path to work with
            if (pathCopy[0] != '/') { // if the path is relative
                // we create a path containing pwd and path
                char *newPathTmp = malloc(sizeof(char) * (strlen(pwd) + strlen(pathCopy) + 2));
                sprintf(newPathTmp,"%s/%s",pwd,pathCopy);
                pathCopy = realloc(pathCopy, sizeof(char) * (strlen(newPathTmp) + 1));
                strcpy(pathCopy, newPathTmp);
                free(newPathTmp);
            }
            // We create a char** to keep the information of the path.
            // More precisely, each char* will correspond to a subpart of path, delimited by "/".

            char** partByPartNewPath=malloc(sizeof(char*)*MAX_ARGS_STRLEN);
            testMalloc(partByPartNewPath);
            char* token;
            size_t* taille_array_init= malloc(sizeof(size_t));
            testMalloc(taille_array_init);
            size_t taille_token;
            size_t taille_array=0;
            *taille_array_init=1;

            // each subbpart is obtained with strtok.
            // this is also the reason why we needed a copy of the path. strtok damage his parameter.
            token=strtok(pathCopy,"/");
            do{
                if(token!=NULL){
                    taille_token=strlen(token);
                    if(taille_array==MAX_ARGS_STRLEN){
                        perror("MAX ARGS LEN REACHED");
                        exit(EXIT_FAILURE);
                    }

                    *(partByPartNewPath+taille_array)=malloc(sizeof(char)*(taille_token+1));
                    *(partByPartNewPath+taille_array)=memcpy(*(partByPartNewPath+taille_array),token,taille_token+1);
                    taille_array++;
                }
            }
            while((token = strtok(NULL, "/")));

            free(token);
            free(taille_array_init);
            free(pathCopy);

            int counter = 0;
            while (taille_array > counter) { // We go through the whole array.
                if (strcmp(partByPartNewPath[counter], "..") == 0) { // Each time we encounter "..".
                    strcpy(partByPartNewPath[counter], "-"); // We replace the content of the case with "-".
                    int reverseCounter = 1;
                    while (strcmp(partByPartNewPath[counter - reverseCounter], "-") == 0 && counter - reverseCounter >= 0) {
                        reverseCounter += 1;
                    }
                    if (counter - reverseCounter > 0) {
                        strcpy(partByPartNewPath[counter - reverseCounter], "-"); //As well as the previous case not containing already "-".
                    } else { // If there is none -> logical interpretation that makes little or no sense -> physical interpretation.
                        // todo free avant de sortir de la fonction ?
                        return process_cd("-P", path);
                    }
                } else if (strcmp(partByPartNewPath[counter], ".") == 0) {// Each time we encounter "."
                    strcpy(partByPartNewPath[counter], "-"); // We replace the content of the case with "-".
                }
                counter += 1;
            }

            char *newPath = malloc(sizeof(char) * (MAX_ARGS_STRLEN)); // Creation of the new path, without ".." and "." subpart.
            testMalloc(newPath);

            strcpy(newPath,"");
            counter = 0;
            while (taille_array > counter) {
                if (strcmp(partByPartNewPath[counter], "-") != 0) { // it will contain all the sub-parts of the initial path that have not been replaced by "-".
                    strcat(newPath, "/"); // by placing a "/" between each part.
                    strcat(newPath, partByPartNewPath[counter]);
                }
                free(partByPartNewPath[counter]);
                counter += 1;
            }

            if(isPathValidLo(newPath) != 0) { // if the path is not valid, we call process_cd with the physical option.
                free(newPath);
                free(partByPartNewPath);
                return process_cd("-P", path);
            }
            else{
                // We modify the global variables.
                strcpy(oldpwd, pwd);
                strcpy(pwd, newPath);

                // We also save the physical version of the path, for the next cd.
                chdir(newPath);
                getcwd(pwdPhy, BUFSIZE);
                free(newPath);
            }

            free(partByPartNewPath);
        }
    }
    return 0;
}

/***
 * interprets the cd arguments to call process_cd with good parameters
 * @param liste struct for the command
 */
void process_cd_call(cmds_struct liste){
    if(liste.taille_array>3){ // too many arguments
        perror("Trop d'arguments pour la commande cd");
        errorCode=1;
    }
    else if(liste.taille_array == 1){ // no option and no path
        errorCode = process_cd("-L", home);
    }
    else if(liste.taille_array == 3) { // option and path
        errorCode = process_cd(*(liste.cmds_array + 1), *(liste.cmds_array + 2));
    }
    else{ // no option or no path
        // no path
        if(strcmp(*(liste.cmds_array + 1),"-L") == 0 || strcmp(*(liste.cmds_array + 1),"-P") == 0){
            errorCode = process_cd(*(liste.cmds_array + 1), home);
        }
        else {// no option
            errorCode = process_cd("-L", *(liste.cmds_array + 1));
        }
    }
}

/***
 * interprets the pwd arguments to call get_cwd with good parameters or print
 * global variable pwd
 * @param liste struct for the command
 */
void process_pwd_call(cmds_struct liste){
  if(liste.taille_array>2){
      perror("Trop d'arguments pour la commande pwd");
      errorCode=1;
  }
  // appel de get_cwd avec *(liste.cmds_array+1)
  size_t size = liste.taille_array - 1;
  if(size > 0 && (strcmp(liste.cmds_array[1],"-P")==0)){
      char* pwd_physique = malloc(sizeof(char)*BUFSIZE);
      getcwd(pwd_physique,BUFSIZE);
      printf("%s\n",pwd_physique);
      free(pwd_physique);
  }
  else{
      printf("%s\n", pwd);
  }
}

/***
 * interprets the exit arguments to call exit() with good value
 * @param liste struct for the command
 */
void process_exit_call(cmds_struct liste){
  if(liste.taille_array>2){
      perror("Trop d'arguments pour la commande exit");
      errorCode=1;
  }
  if(liste.taille_array == 2){
      int exit_value = atoi(*(liste.cmds_array+1));
      exit(exit_value);
  }
  else{
      exit(errorCode);
  }
}

/***
 * interprets the commands to call the corresponding functions.
 * @param liste struct for the command
 */
void interpreter(cmds_struct liste) {
    if(strcmp(*liste.cmds_array,"cd")==0){
        process_cd_call(liste);
    }
    else if(strcmp(*liste.cmds_array,"pwd")==0){
        process_pwd_call(liste);
    }
    else if(strcmp(*liste.cmds_array,"exit")==0){
      process_exit_call(liste);
    }
}

/***
 * turns a line into a command structure
 * @param ligne line from the prompt
 * @return struct cmds_struct
 */
cmds_struct lexer(char* ligne){
    char** cmds_array=malloc(sizeof(char*));
    testMalloc(cmds_array);
    char* token;
    size_t* taille_array_init= malloc(sizeof(size_t));
    testMalloc(taille_array_init);
    size_t taille_token;
    size_t taille_array=0;
    *taille_array_init=1;

    token=strtok(ligne," ");
    do{
        taille_token=strlen(token);

        if(taille_token>=MAX_ARGS_STRLEN){
            perror("MAX_ARGS_STRLEN REACHED");
            exit(1);
        }

        cmds_array=checkArraySize(cmds_array,taille_array,taille_array_init);
        *(cmds_array+taille_array)=malloc(sizeof(char)*(taille_token+1));
        *(cmds_array+taille_array)=memcpy(*(cmds_array+taille_array),token,taille_token+1);
        taille_array++;
    }
    while((token = strtok(NULL, " ")));


    free(token);
    free(taille_array_init);
    cmds_struct cmdsStruct = {.cmds_array=cmds_array, .taille_array=taille_array};
    return cmdsStruct;
}

/***
 *
 * @return
 */
char* promptGeneration(){
    char* curr_path_cpy=pwd;
    size_t curr_path_size=strlen(curr_path_cpy);
    char* tmp_curr=malloc(sizeof(char)*26);
    testMalloc(tmp_curr);
    char* new_curr=malloc(sizeof(char)*26);
    testMalloc(new_curr);

    if(curr_path_size>25){
        size_t beginning_size=curr_path_size-22;
        strcpy(tmp_curr,curr_path_cpy+beginning_size);
        sprintf(new_curr,"...%s",tmp_curr);
    }
    else{
        strcpy(new_curr,curr_path_cpy);
    }
    char* res=malloc(sizeof(char)*(55+strlen(new_curr)));

    sprintf(res,"\001\033[32m\002[%d]\001\033[36m\002%s\001\033[00m\002$ ",errorCode,new_curr);
    free(new_curr);
    free(tmp_curr);
    return res;
}
/***
 * Variables initialization
 */
void initVar(){
    home = malloc(MAX_ARGS_STRLEN * sizeof(char));
    strcpy(home,getenv("HOME"));
    pwd = malloc(MAX_ARGS_STRLEN * sizeof(char));
    strcpy(pwd, getenv("PWD"));
    oldpwd = malloc(MAX_ARGS_STRLEN * sizeof(char));
    strcpy(oldpwd,pwd);

    chdir(pwd);
    pwdPhy = malloc(MAX_ARGS_STRLEN * sizeof(char));
    testMalloc(pwdPhy);
    getcwd(pwdPhy, MAX_ARGS_STRLEN * sizeof(char));
}

/***
 *
 */
void run(){
    rl_outstream=stderr;
    initVar();
    char* ligne;
    cmds_struct liste;
    while(1){
        char* tmp=promptGeneration();
        ligne=readline(tmp);

        if(ligne && *ligne){
            add_history(ligne);

            liste=lexer(ligne);

            interpreter(liste);
        }
        else if(rl_point==rl_end){
            exit(errorCode);
        }
        freeCmdsArray(liste);
        free(ligne);
        free(tmp);
    }
}

/***
 * call run() function
 * @return exit value
 */
int main(void) {
    run();
}
