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
// 0 == true
int isValidPhy(char * path){
    if(path[0] == '/'){
        struct stat st;
        stat(path,&st);
        return S_ISDIR(st.st_mode)?0:1;
    }else{
        char *newPathTmp = malloc(strlen(pwdPhy) + strlen(path) + 2); // on crée un chemin contenant pwd et path = newPathTmp
        sprintf(newPathTmp,"%s/%s",pwdPhy,path);
        struct stat st;
        stat(newPathTmp,&st);
        free(newPathTmp);
        if(S_ISDIR(st.st_mode)){
            return 0;
        }
        return 1;
    }
}

int isValidLo(char * path){
    struct stat st;
    stat(path,&st);
    return S_ISDIR(st.st_mode)?0:1;
}

/*
 * todo faire des malloc 1024, puis realloc si + grand
 */
int process_cd(char * option, char * path){
    if(strcmp(option,"-P") == 0){
        if(isValidPhy(path) != 0) {
            perror("bash: cd: a: Aucun fichier ou dossier de ce type\n");
            exit(EXIT_FAILURE);
        }
        if(path[0] == '-'){ // cas du retour en arriere
            char * temp = malloc(BUFSIZE);
            strcpy(temp,oldpwd);
            strcpy(oldpwd,pwd);

            chdir(temp);
            getcwd(pwd, BUFSIZE); //todo plus grandes valeurs si chemin + long ?
            getcwd(pwdPhy,BUFSIZE);
            free(temp);
        }else {
            // oldpwd prend l'ancienne valeur de pwd
            strcpy(oldpwd, pwd);

            if (path[0] == '/') { // chemin absolu : pwdPhy et pwd prennent la valeur du chemin, transformé en chemin physique
                chdir(path);
                getcwd(pwdPhy, BUFSIZE);
                getcwd(pwd,BUFSIZE);

            } else { // chemin relatif : pwdPhy et pwd prennent la valeur du pwdPhy + "/" + chemin, transformé en chemin physique
                char *newPath = malloc((strlen(pwdPhy) + 2 + strlen(path)));
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
            char * tmp = malloc(BUFSIZE);
            strcpy(tmp,oldpwd);
            strcpy(oldpwd,pwd);
            strcpy(pwd,tmp);


            chdir(tmp); // pwdPhy prend l'ancienne valeur de oldpwd, qu'on transforme en valeur physique
            getcwd(pwdPhy,BUFSIZE);
            free(tmp);
        }else {
            char * pathSave = malloc(strlen(path) + 1);
            testMalloc(pathSave);
            strcpy(pathSave,path);
            if (path[0] != '/') { // si le chemin est relatif
                char *newPathTmp = malloc(strlen(pwd) + strlen(path) + 2); // on crée un chemin contenant pwd et path = newPathTmp
                sprintf(newPathTmp,"%s/%s",pwd,path);
                path = realloc(path, strlen(newPathTmp) + 1);
                strcpy(path, newPathTmp);
            }

            char **partByPartNewPath = malloc(sizeof(char *));

            int i = 0;
            *(partByPartNewPath + i) = strtok(path, "/");
            do{
                i += 1;
                partByPartNewPath = realloc(partByPartNewPath, sizeof(char *) * (i + 1));
                testMalloc(partByPartNewPath);

            }while((*(partByPartNewPath + i) = strtok(NULL,"/")));

            int cpt = 0;
            while (i > cpt) {
                if (strcmp(partByPartNewPath[cpt], "..") == 0) { // a chaque fois qu'on rencontre ..
                    strcpy(partByPartNewPath[cpt], "-");
                    int j = 1;
                    while (strcmp(partByPartNewPath[cpt - j], "-") == 0 && cpt - j >= 0) {
                        j += 1;
                    }
                    if (cpt - j >= 0) {
                        strcpy(partByPartNewPath[cpt - j], "-"); // on supprime la premiere sous partie précédente qui n'est pas deja supprimée
                    } else { //si il n'y en a pas, interpretation logique qui n'a pas ou peu de sens, on interprete physiquement
                        if(pathSave != NULL) return process_cd("-P", pathSave);
                    }
                } else if (strcmp(partByPartNewPath[cpt], ".") == 0) {// a chaque fois qu'on rencontre .
                    strcpy(partByPartNewPath[cpt], "-"); // on supprime la sous partie
                }
                cpt += 1;
            }

            char *newPath = malloc(BUFSIZE); // on crée un string
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
                return process_cd("-P", pathSave);
            }else {
                // on modifie les variables globales
                strcpy(oldpwd, pwd);
                strcpy(pwd, newPath);

                // on prend aussi le chemin physique, pour les prochains cd
                chdir(newPath);
                getcwd(pwdPhy, BUFSIZE);
            }
        }
    }
    //printf("pwd = %s , pwdPhy = %s , oldpwd = %s\n", pwd, pwdPhy, oldpwd);
    return 1;
}





void push_string(char* str1,char* str2){
  char *tmp = strdup(str1);
  strcpy(str1, str2);
  strcat(str1, tmp);
  free(tmp);
}

void printListe(cmds_struct liste) {
    int count=0;
    int countString=0;
    char* string=*liste.cmds_array;
    char c;
    while(count!=liste.taille_array){
        c=*(string+countString);
        while(c!=EOF && c!='\0'){
            printf("%c",c);
            countString++;
            c=*(string+countString);
        }
        printf("\n");
        countString=0;
        count++;
        string=*(liste.cmds_array+count);
    }
}

// double la taille allouée à la variable si nécessaire
char* checkStringSize(char* string,size_t taille_string,size_t* taille_string_initiale){
    if(taille_string==MAX_ARGS_STRLEN){
        perror("MAX ARGS STRLEN REACHED");
        exit(EXIT_FAILURE);
    }
    if(taille_string==*(taille_string_initiale)){
        *(taille_string_initiale)*=2;
        string=realloc(string,sizeof(char)*(*taille_string_initiale));
        //if(string==NULL) perror("malloc");
        testMalloc(string);
    }
    return string;
}

// double la taille allouée à la variable si nécessaire
char** checkArraySize(char** array,size_t taille_array,size_t* taille_array_initiale){
    if(taille_array==MAX_ARGS_NUMBER){
        perror("MAX ARGS NUMBER REACHED");
        exit(1);
    }
    if(taille_array==*(taille_array_initiale)){
        *(taille_array_initiale)*=2;
        array=realloc(array,sizeof(char *)*(*taille_array_initiale));
        testMalloc(array);
    }
    return array;
}

char* reallocToSizeString(char* string,size_t taille_string){
    *(string+taille_string)='\0';
    string=realloc(string, sizeof(char)*(taille_string+2));
    testMalloc(string);
    return string;
}

char** reallocToSizeArray(char** array,size_t taille_array){
    array=realloc(array, sizeof(char *)*taille_array);
    testMalloc(array);
    return array;
}

void interpreter(cmds_struct liste) {
    if(strcmp(*liste.cmds_array,"cd")==0){ // todo faire une fonction qui choisit le bon appel ?
        if(liste.taille_array>3){
            perror("Trop d'arguments pour la commande cd");
            errorCode=1;
        }else if(liste.taille_array == 1){
            process_cd("-L", home);
        }
        else if(liste.taille_array == 3) {
            process_cd(*(liste.cmds_array + 1), *(liste.cmds_array + 2));
        }
        else{ // quand il n'y a pas d'option/ pas de path
            if(strcmp(*(liste.cmds_array + 1),"-L") == 0 || strcmp(*(liste.cmds_array + 1),"-P") == 0){ // pas de path
                process_cd(*(liste.cmds_array + 1), home);
            }
            else {// pas d'option
                process_cd("-L", *(liste.cmds_array + 1));
            }
        }
    }
    else if(strcmp(*liste.cmds_array,"pwd")==0){
        if(liste.taille_array>2){
            perror("Trop d'arguments pour la commande pwd");
            errorCode=1;
        }
        // appel de cwd avec *(liste.cmds_array+1)
        int size = liste.taille_array - 1;
        if(size > 0 && (strcmp(liste.cmds_array[1],"-P")==0)){
          char* pwd_physique = malloc(sizeof(char)*BUFSIZE);
          getcwd(pwd_physique,BUFSIZE);
          printf("%s\n",pwd_physique);
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
    char* token;
    size_t* taille_array_init= malloc(sizeof(size_t));
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


char* promptGeneration(){
    char* curr_path_cpy=pwd;
    size_t curr_path_size=strlen(curr_path_cpy);
    char* tmp_curr=malloc(sizeof(char)*26);
    char* new_curr=malloc(sizeof(char)*26);

    if(curr_path_size>25){
        size_t beginning_size=curr_path_size-22;
        strcpy(tmp_curr,curr_path_cpy+beginning_size);
        sprintf(new_curr,"...%s",tmp_curr);
    }
    else{
        strcpy(new_curr,curr_path_cpy);
    }
    char* res=malloc(sizeof(char)*52);

    sprintf(res,"\001\033[32m\002[%d]\001\033[36m\002%s\001\033[00m\002$ ",errorCode,new_curr);
    free(new_curr);
    free(tmp_curr);
    return res;
}

void initVar(){
    home = malloc(BUFSIZE);
    strcpy(home,getenv("HOME"));
    pwd = malloc((BUFSIZE));
    strcpy(pwd, getenv("PWD"));
    oldpwd = malloc(BUFSIZE);
    strcpy(oldpwd,pwd);

    chdir(pwd);
    pwdPhy = malloc(BUFSIZE); //todo taille dynamique
    testMalloc(pwdPhy);
    getcwd(pwdPhy, BUFSIZE); //todo taille dynamique
}

void run(){
    rl_outstream=stderr;
    initVar();
    char* ligne;
    while(1){

        ligne=readline(promptGeneration());

        if(ligne && *ligne){
            add_history(ligne);

            cmds_struct liste=lexer(ligne);


            interpreter(liste);
        }
        else if(rl_point==rl_end){
            exit(errorCode);
        }
        //free(ligne);


    }
}

int main(void) {
    run();
}
