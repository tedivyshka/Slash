#define MAX_ARGS_NUMBER 4096
#define MAX_ARGS_STRLEN 4096
#define BUFSIZE 1024

#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

typedef struct cmds_struct{
    char** cmds_array;
    size_t taille_array;
}cmds_struct;

// Variables globales :
int errorCode=0;

char * pwd;
char * oldpwd;
char * pwdPhy;
char * home;



void testMalloc(void * ptr){
    if(ptr == NULL){
        perror("Erreur de malloc() ou realloc().\n");
        exit(EXIT_FAILURE);
    }
}

void freeCmdsArray(cmds_struct array) {
    for (int i = 0; i < array.taille_array; i++) {
        free(*(array.cmds_array + i));
    }
    free(array.cmds_array);
}

void freeArray(char** array,size_t taille){
    for(int i=0;i<taille-3;i++){
        free(array[i]);
    }
}


// double la taille allouée à la variable si nécessaire
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

char** reallocToSizeArray(char** array,size_t taille_array){
    array=realloc(array, sizeof(char*)*(taille_array));
    return array;
}

// 0 == true
int isValidPhy(char * path){
    if(path[0] == '/'){
        struct stat st;
        stat(path,&st);
        if(S_ISDIR(st.st_mode)){
            return 0;
        }
        return 1;
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

int isValidLo(char* path){
    struct stat st;
    int res;
    res=stat(path,&st);
    if(res==0 && S_ISDIR(st.st_mode)){
        return 0;
    }
    return 1;
}

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

/*
 * todo faire des malloc 1024, puis realloc si + grand
 */
int process_cd(char * option, char * path){
    if(strcmp(option,"-P") == 0){
        if(isValidPhy(path) != 0) {
            char* er = malloc(sizeof(char) * (strlen(path) + 48));
            sprintf(er,"bash: cd: %s: Aucun fichier ou dossier de ce type",path);
            perror(er);
            return 1;
        }
        if(path[0] == '-'){ // cas du retour en arriere
            char * temp = malloc(sizeof(char) * MAX_ARGS_STRLEN);
            strcpy(temp,oldpwd);
            strcpy(oldpwd,pwd);

            chdir(temp);
            getcwd(pwd, BUFSIZE); //todo plus grandes valeurs si chemin + long ?
            getcwd(pwdPhy,BUFSIZE);
            free(temp);
        }
        else {
            // oldpwd prend l'ancienne valeur de pwd
            strcpy(oldpwd, pwd);

            if (path[0] == '/') { // chemin absolu : pwdPhy et pwd prennent la valeur du chemin, transformé en chemin physique
                chdir(path);
                getcwd(pwdPhy, BUFSIZE);
                getcwd(pwd,BUFSIZE);
            }
            else{ // chemin relatif : pwdPhy et pwd prennent la valeur du pwdPhy + "/" + chemin, transformé en chemin physique
                char *newPath = malloc(sizeof(char) * ((strlen(pwdPhy) + 2 + strlen(path))));
                testMalloc(newPath);
                sprintf(newPath,"%s/%s",pwdPhy,path);
                chdir(newPath);
                getcwd(pwdPhy, BUFSIZE);
                getcwd(pwd,BUFSIZE);
            }
        }
    }
    else{ // option -L or no option
        if(path[0] == '-'){ // cas du retour en arriere
            char * tmp = malloc(sizeof(char) * MAX_ARGS_STRLEN);
            strcpy(tmp,oldpwd);
            strcpy(oldpwd,pwd);
            strcpy(pwd,tmp);


            chdir(tmp); // pwdPhy prend l'ancienne valeur de oldpwd, qu'on transforme en valeur physique
            getcwd(pwdPhy,BUFSIZE);
            free(tmp);
        }else {
            char * pathSave = malloc(sizeof(char) * (strlen(path) + 1));
            testMalloc(pathSave);
            strcpy(pathSave,path);
            if (pathSave[0] != '/') { // si le chemin est relatif
                char *newPathTmp = malloc(sizeof(char) * (strlen(pwd) + strlen(pathSave) + 2)); // on crée un chemin contenant pwd et path = newPathTmp
                sprintf(newPathTmp,"%s/%s",pwd,pathSave);
                pathSave = realloc(pathSave, sizeof(char) * (strlen(newPathTmp) + 1));
                strcpy(pathSave, newPathTmp);
                free(newPathTmp);
            }

            /*
            char** partByPartNewPath=malloc(sizeof(char*));
            testMalloc(partByPartNewPath);
            char* token;
            size_t* taille_array_init= malloc(sizeof(size_t));
            testMalloc(taille_array_init);
            size_t taille_token;
            size_t taille_array=0;
            *taille_array_init=1;

            token=strtok(pathSave,"/");
            do{
                taille_token=strlen(token);
                if(taille_array==MAX_ARGS_STRLEN){
                    perror("MAX ARGS LEN REACHED");
                    exit(EXIT_FAILURE);
                }

                *(partByPartNewPath+taille_array)=malloc(sizeof(char)*(taille_token+1));
                *(partByPartNewPath+taille_array)=memcpy(*(partByPartNewPath+taille_array),token,taille_token+1);
                taille_array++;
            }
            while((token = strtok(NULL, "/")));

            free(token);
            free(taille_array_init);

            size_t i=taille_array;
            */

            char **partByPartNewPath = malloc(sizeof(char *));
            testMalloc(partByPartNewPath);

            int i = 0;
            *(partByPartNewPath + i) = strtok(pathSave, "/");
            do{
                i += 1;
                partByPartNewPath = realloc(partByPartNewPath, sizeof(char *) * (i+1));
                testMalloc(partByPartNewPath);

            }
            while((*(partByPartNewPath + i) = strtok(NULL,"/")));

            int cpt = 0;
            while (i > cpt) {
                if (strcmp(partByPartNewPath[cpt], "..") == 0) { // a chaque fois qu'on rencontre ..
                    *(partByPartNewPath+cpt)="-";
                    //strcpy(partByPartNewPath[cpt], "-");
                    int j = 1;
                    while (strcmp(partByPartNewPath[cpt - j], "-") == 0 && cpt - j >= 0) {
                        j += 1;
                    }
                    if (cpt - j > 0) {
                        *(partByPartNewPath+(cpt-j))="-";
                        //strcpy(partByPartNewPath[cpt - j], "-"); // on supprime la premiere sous partie précédente qui n'est pas deja supprimée
                    } else { //si il n'y en a pas, interpretation logique qui n'a pas ou peu de sens, on interprete physiquement
                        //free(partByPartNewPath);
                        return process_cd("-P", path);
                    }
                } else if (strcmp(partByPartNewPath[cpt], ".") == 0) {// a chaque fois qu'on rencontre .
                    *(partByPartNewPath+cpt)="-";
                    //strcpy(partByPartNewPath[cpt], "-"); // on supprime la sous partie
                }
                cpt += 1;
            }

            char *newPath = malloc(sizeof(char) * (BUFSIZE)); // on crée un string
            testMalloc(newPath);

            strcpy(newPath,"");
            int cpt2 = 0;
            while (i > cpt2) {
                if (strcmp(partByPartNewPath[cpt2], "-") != 0) { // qui va contenir toutes les sous parties du chemin intiniale qui n'ont pas été supprimée
                    strcat(newPath, "/"); // en plaçant un "/" entre chaque parties
                    strcat(newPath, partByPartNewPath[cpt2]);
                }
                cpt2 += 1;
            }

            if(isValidLo(newPath) != 0) { // si le chemin n'est pas valide, on appelle avec cd physique
                free(newPath);
                //free(partByPartNewPath);
                return process_cd("-P", path);
            }
            else{
                // on modifie les variables globales
                strcpy(oldpwd, pwd);
                strcpy(pwd, newPath);

                // on prend aussi le chemin physique, pour les prochains cd
                chdir(newPath);
                getcwd(pwdPhy, BUFSIZE);
                free(newPath);
            }
            //free(partByPartNewPath);
        }
    }
    return 0;
}

void interpreter(cmds_struct liste) {
    if(strcmp(*liste.cmds_array,"cd")==0){ // todo faire une fonction qui choisit le bon appel ?
        if(liste.taille_array>3){
            perror("Trop d'arguments pour la commande cd");
            errorCode=1;
        }
        else if(liste.taille_array == 1){
            errorCode = process_cd("-L", home);
        }
        else if(liste.taille_array == 3) {
            errorCode = process_cd(*(liste.cmds_array + 1), *(liste.cmds_array + 2));
        }
        else{ // quand il n'y a pas d'option/ pas de path
            if(strcmp(*(liste.cmds_array + 1),"-L") == 0 || strcmp(*(liste.cmds_array + 1),"-P") == 0){ // pas de path
                errorCode = process_cd(*(liste.cmds_array + 1), home);
            }
            else {// pas d'option
               errorCode = process_cd("-L", *(liste.cmds_array + 1));
            }
        }
    }
    else if(strcmp(*liste.cmds_array,"pwd")==0){
        if(liste.taille_array>2){
            perror("Trop d'arguments pour la commande pwd");
            errorCode=1;
        }
        // appel de cwd avec *(liste.cmds_array+1)
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
    else if(strcmp(*liste.cmds_array,"exit")==0){
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
}

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


    //reallocToSizeArray(cmds_array,taille_array);
    free(token);
    free(taille_array_init);
    cmds_struct cmdsStruct = {.cmds_array=cmds_array, .taille_array=taille_array};
    return cmdsStruct;
}


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

void initVar(){
    home = malloc(BUFSIZE * sizeof(char));
    strcpy(home,getenv("HOME"));
    pwd = malloc((BUFSIZE * sizeof(char)));
    strcpy(pwd, getenv("PWD"));
    oldpwd = malloc(BUFSIZE * sizeof(char));
    strcpy(oldpwd,pwd);

    chdir(pwd);
    pwdPhy = malloc(BUFSIZE * sizeof(char)); //todo taille dynamique
    testMalloc(pwdPhy);
    getcwd(pwdPhy, BUFSIZE * sizeof(char)); //todo taille dynamique
}

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

int main(void) {
    run();
}
