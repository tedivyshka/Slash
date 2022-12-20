#include "commands.h"

/***
 * Runs external command
 * @param liste struct for the command
 */
void process_external_command(cmds_struct liste){
    char** args = malloc(sizeof(char*) * (liste.taille_array + 1));
    //Fill args array
    for(int i = 0; i < liste.taille_array; i++){
        args[i] = malloc(sizeof(char) * strlen(*(liste.cmds_array + i)) + 1);
        strcpy(args[i], *(liste.cmds_array + i));
    }
    args[liste.taille_array] = NULL;

    int status, r = fork();
    if(r == 0){
        defaultSignals();
        execvp(args[0],args);
        exit(1);
    }
    else{
        wait(&status);
        errorCode = WEXITSTATUS(status);
        if(WIFSIGNALED(status)) errorCode = -1;

        for(int i = 0; i < liste.taille_array+1; i++){
            free(args[i]);
        }
        free(args);
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
        if(path[0] == '-'){ // case of "-", we return to the last directory where we were.
            if(strlen(oldpwd) == 0){
                errno = ECANCELED;
                perror("bash: cd: « OLDPWD » undefined");
                return 1;
            }
            char * temp = malloc(sizeof(char) * MAX_ARGS_STRLEN);
            strcpy(temp,oldpwd);
            strcpy(oldpwd,pwd);

            chdir(temp); // chdir() and getcwd() function are used to get the physical path of the parameter path.
            getcwd(pwd, MAX_ARGS_STRLEN);
            getcwd(pwdPhy,MAX_ARGS_STRLEN);
            free(temp);
        }
        else{
            if(isPathValidPhy(path) != 0) { // if path isn't valid, an error message is displayed and the return value is 1.
                char* er = malloc(sizeof(char) * (strlen(path) + 48));
                sprintf(er,"bash: cd: %s",path);
                errno = ENOENT;
                perror(er);
                free(er);
                return 1;
            }
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
            if(strlen(oldpwd) == 0){
                errno = ECANCELED;
                perror("slash: cd: « OLDPWD » undefined");
                return 1;
            }
            char * tmp = malloc(sizeof(char) * MAX_ARGS_STRLEN);
            strcpy(tmp,oldpwd);
            strcpy(oldpwd,pwd);
            strcpy(pwd,tmp);


            chdir(tmp); // pwdPhy takes the old value of oldpwd
            getcwd(pwdPhy,BUFSIZE); // which we transform into a physical value with chdir + getcwd
            free(tmp);
        }
        else { // case of relative and absolute logical path
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

            // each sub part is obtained with strtok.
            // this is also the reason why we needed a copy of the path. strtok damage his parameter.
            token=strtok(pathCopy,"/");
            do{
                if(token!=NULL){
                    taille_token=strlen(token);
                    if(taille_array==MAX_ARGS_STRLEN){
                        perror("slash: cd: MAX ARGS LEN REACHED");
                        exit(EXIT_FAILURE);
                    }

                    *(partByPartNewPath+taille_array)=malloc(sizeof(char)*(taille_token+1));
                    testMalloc(*(partByPartNewPath+taille_array));
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
                    while (counter - reverseCounter >= 0 && strcmp(partByPartNewPath[counter - reverseCounter], "-") == 0) {
                        reverseCounter += 1;
                    }
                    if (counter - reverseCounter > 0) {
                        strcpy(partByPartNewPath[counter - reverseCounter], "-"); //As well as the previous case not containing already "-".
                    } else { // If there is none -> logical interpretation that makes little or no sense -> physical interpretation.
                        return process_cd("-P", path);
                    }
                }
                else if (strcmp(partByPartNewPath[counter], ".") == 0) {// Each time we encounter "."
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
 * Interprets the cd arguments to call process_cd with good parameters.
 * @param liste struct for the command
 */
void process_cd_call(cmds_struct liste){
    if(liste.taille_array>3){ // too many arguments
        errno = EINVAL;
        perror("slash: cd");
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
 * Interprets the pwd arguments to call get_cwd with good parameters or print global variable pwd.
 * @param liste struct for the command
 */
void process_pwd_call(cmds_struct liste){
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
    errorCode = 0;
}

/***
 * Interprets the exit arguments to call exit() with good value
 * @param liste struct for the command
 */
void process_exit_call(cmds_struct liste){
    if(liste.taille_array>2){
        errno = EINVAL;
        perror("slash: exit");
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
