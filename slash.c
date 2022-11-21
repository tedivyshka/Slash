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
        printf("Erreur de malloc() ou realloc().\n");
        exit(EXIT_FAILURE);
    }
}

int process_cd(char * option, char * path){
        // todo verifier que path valide pour pwdPhy
    if(strcmp(option,"-P") == 0){
        if(path[0] == '-'){ // cas du retour en arriere
            char * temp = oldpwd;
            oldpwd = malloc(strlen(pwd)); // oldpwd prend la valeur de pwd
            strcpy(oldpwd,pwd);
            pwd = malloc(strlen(temp)); // pwd et pwdPhy prennent l'ancienne valeur de oldpwd, qu'on transforme en valeur physique
            chdir(temp);
            getcwd(pwd, BUFSIZ);
            getcwd(pwdPhy,BUFSIZ);
            free(temp);
        }else {
            // oldpwd prend l'ancienne valeur de pwd
            oldpwd = malloc(strlen(pwd));
            testMalloc(oldpwd);
            oldpwd = pwd;

            if (path[0] == '/') { // chemin absolu : pwdPhy et pwd prennent la valeur du chemin, transformé en chemin physique
                chdir(path);
                pwdPhy = malloc(strlen(path));
                pwd = malloc(strlen(path));
                testMalloc(pwdPhy);
                testMalloc(pwd);
                getcwd(pwdPhy, BUFSIZ);
                getcwd(pwd,BUFSIZ);

            } else { // chemin relatif : pwdPhy et pwd prennent la valeur du pwd + "/" + chemin, transformé en chemin physique
                char *newPath = malloc(sizeof(char) * (strlen(pwdPhy) + 1 + strlen(path)));
                testMalloc(newPath);
                strcpy(newPath, pwdPhy);
                newPath[strlen(pwdPhy)] = '/';
                strcat(newPath, path);
                chdir(newPath);
                pwdPhy = malloc(strlen(newPath));
                pwd = malloc(strlen(newPath));
                testMalloc(pwdPhy);
                testMalloc(pwd);
                getcwd(pwdPhy, BUFSIZ);
                getcwd(pwd,BUFSIZ);
            }
        }
    }
        // todo verifier que path valide pour pwd
    else{ // option -L or no option
        printf("test1\n");
        if(path[0] == '-'){ // cas du retour en arriere
            char * temp = oldpwd;
            oldpwd = malloc(strlen(pwd)); // oldpwd prend la valeur de pwd
            strcpy(oldpwd,pwd);
            pwd = malloc(strlen(temp)); // pwd prend l'ancienne valeur de oldpwd
            testMalloc(pwd);
            strcat(pwd,temp);

            chdir(temp); // pwdPhy prend l'ancienne valeur de oldpwd, qu'on transforme en valeur physique
            getcwd(pwdPhy,BUFSIZ);
            free(temp);
        }else {
            if (path[0] != '/') { // si le chemin est relatif
                char *newPathTmp = malloc(strlen(pwd) + strlen(path) + 1); // on crée un chemin contenant pwd et path = newPathTmp
                strcpy(newPathTmp, pwd);
                strcat(newPathTmp, "/");
                strcat(newPathTmp, path);
                path = malloc(strlen(newPathTmp));
                strcpy(path, newPathTmp);
            }
            printf("test2\n");

            char **partByPartNewPath = malloc(sizeof(char *));
            printf("test2.1\n");

            int i = 0;
            *(partByPartNewPath + i) = strtok(path, "/");
            do{
                i += 1;
                realloc(partByPartNewPath, sizeof(char *) * (i + 1));
                testMalloc(partByPartNewPath);

            }while((*(partByPartNewPath + i) = strtok(NULL,"/")));
            printf("test3\n");

            int cpt = 0;
            while (i > cpt) {
                if (strcmp(partByPartNewPath[cpt], "..") == 0) { // a chaque fois qu'on rencontre ..
                    partByPartNewPath[cpt] = "-";
                    int j = 1;
                    while (strcmp(partByPartNewPath[cpt - j], "-") == 0 && cpt - j >= 0) {
                        j += 1;
                    }
                    if (cpt - j >= 0) {
                        partByPartNewPath[cpt - j] = "-"; // on supprime la premiere sous partie précédente qui n'est pas deja supprimée
                    } else { //si il n'y en a pas, interpretation logique qui n'a pas ou peu de sens, on interprete physiquement
                        return process_cd("-P", path);
                    }
                } else if (strcmp(partByPartNewPath[cpt], ".") == 0) {// a chaque fois qu'on rencontre .
                    partByPartNewPath[cpt] = "-"; // on supprime la sous partie
                }
                cpt += 1;
            }
            printf("test4\n");

            char *newPath = malloc(strlen(path)); // on crée un string
            testMalloc(newPath);
            int cpt2 = 0;
            while (i > cpt2) {
                if (strcmp(partByPartNewPath[cpt2], "-") != 0) { // qui va contenir toutes les sous parties du chemin intiniale qui n'ont pas été supprimée
                    strcat(newPath, "/"); // en plaçant un "/" entre chaque parties
                    strcat(newPath, partByPartNewPath[cpt2]);
                }
                cpt2 += 1;
            }
            printf("test5\n");
            // on modifie les variables globales
            printf("%s -- %lu\n", pwd, strlen(pwd));
            oldpwd = malloc(strlen(pwd));
            printf("test6\n");

            testMalloc(oldpwd);
            printf("test7\n");

            oldpwd = pwd;
            printf("test8\n");

            pwd = malloc(strlen(newPath));
            printf("test9\n");

            testMalloc(pwd);
            printf("test10\n");

            pwd = newPath;

            printf("test11\n");

            // on prend aussi le chemin physique, pour les prochains cd
            chdir(newPath);
            pwdPhy = malloc(strlen(newPath));
            testMalloc(pwdPhy);
            getcwd(pwdPhy, BUFSIZ);
        }
    }
    printf("pwd = %s , pwdPhy = %s , oldpwd = %s\n", pwd, pwdPhy, oldpwd);
    return 1;
}
/*
 // fonction vérifiant que le chemin donné est bien un directory
int is_directory(char * path){
    struct stat st;
    lstat(path,&st);
    if(S_ISDIR(st.st_mode)){
        return 0;
    }else return 1;
}

int process_cd(char * option, char * path){
    if(strcmp(option,"-P") == 0){
        printf("cd physique\n");
        // cd physique
    }
    else if(strcmp(option,"-L") == 0){
        printf("cd logique\n");
        if(path[0] == '/'){ // référence absolue
            int val = is_directory(path);
            if(val == 0){
                printf("before mod -> previous : %s   current : %s\n",oldpwd,pwd);
                setenv("PWD",path, 1);
                setenv("OLDPWD", pwd, 1);
                oldpwd = malloc(strlen(pwd));
                strcpy(oldpwd, pwd);
                pwd = malloc(strlen(path));
                strcpy(pwd, path);
                printf("after mod -> previous : %s   current : %s\n",oldpwd,pwd);
                printf("L - ref absolue - changements effectués\n");
            }else{
                printf("erreur\n");
                return val;
                // return erreur val
            }
        }else if(path[0] == '-') { // retour a la référence précédente
            printf("before mod -> previous : %s   current : %s\n",oldpwd,pwd);
            setenv("PWD",oldpwd, 1);
            setenv("OLDPWD", pwd, 1);
            char * temp = oldpwd;
            oldpwd = malloc(strlen(pwd));
            strcpy(oldpwd, pwd);
            pwd = malloc(strlen(temp));
            strcpy(pwd, temp);
            free(temp);
            printf("after mod -> previous : %s   current : %s\n",oldpwd,pwd);
            printf("L - ref précédente - changements effectués\n");

        }else{ // référence logique
            char * newPath = malloc(sizeof(char) * (strlen(pwd) + 1 + strlen(path))); // + 2 ? (pour le \0)
            strcpy(newPath,pwd);
            newPath[strlen(pwd)] = '/';
            strcat(newPath,path);
            int val = is_directory(newPath);
            if(val == 0){
                printf("before mod -> previous : %s   current : %s\n",oldpwd,pwd);
                setenv("PWD",newPath, 1);
                setenv("OLDPWD", pwd, 1);
                oldpwd = malloc(strlen(pwd));
                strcpy(oldpwd, pwd);
                pwd = malloc(strlen(newPath));
                strcpy(pwd, newPath);
                printf("after mod -> previous : %s   current : %s\n",oldpwd,pwd);
                printf("L - ref logique - changements effectués\n");
            }else {
                printf("erreur\n");
                return val;
                // return erreur val
            }
        }
    }else {
        printf("erreur option = %s & path = %s\n", option, path);
        return 1; // erreur
    }
    /*DIR * fdNew = opendir(pwd);
    struct dirent * entry = readdir(fdNew);

    while(strcmp(entry->d_name,"/") != 0){

<<<<<<< Updated upstream
void testMalloc(void * ptr){
    if(ptr == NULL){
        perror("Erreur de malloc() ou realloc().\n");
        exit(EXIT_FAILURE);
=======
>>>>>>> Stashed changes
    }

    chdir(pwd);
    char * test = malloc(1024);
    getcwd(test,1024);
    printf("%s -- %s -- %s", pwd, getenv("PWD"),test);
    return 0;
}
*/



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

// 1 si racine
// 0 sinon
// -1 si erreur
int is_root(DIR* dir){
    struct stat root;
    if(stat("/",&root)<0) return -1;

    struct stat st;
    if(fstat(dirfd(dir),&st)<0) return -1;

    return (root.st_ino==st.st_ino && root.st_dev==st.st_dev);
}

char* pwd_physique(DIR* dir){
  char* result =  malloc(sizeof(char) * BUFSIZE);
  //Get current dir ino and dev number
  struct stat st;
  int dir_fd = dirfd(dir);
  if(fstat(dir_fd,&st)<0) {
      perror("Error fstat");
      exit(1);
    }
  ino_t dir_ino=st.st_ino;
  dev_t dir_dev=st.st_dev;

  //Open parent directory
  int parent_fd = openat(dir_fd,"..",O_RDONLY | O_DIRECTORY);
  DIR* parent = fdopendir(parent_fd);

  //Iterate parent directory, search for current directory
  struct dirent *entry;
  while((entry=readdir(parent))){
    if(strcmp(entry->d_name,".")==0 || strcmp(entry->d_name,"..")==0) continue;

    //Get entry stat
    if(fstatat(dirfd(parent),entry->d_name,&st,0) < 0) {
      perror("Erro fstatat");
      exit(1);
    }
    //Compare entry with current directory
    if(st.st_ino == dir_ino && st.st_dev == dir_dev){
      push_string(result,entry->d_name);
      break;
    }
  }

  if(entry == NULL) {
    perror("Directory not found..."); exit(1);
  }
  //If parent isnt root, continue recursively
  if(!is_root(parent)){
    push_string(result,"/");
    push_string(result,pwd_physique(parent));
  }
  else push_string(result,"/");
  return result;
}

void interpreter(cmds_struct liste) {
    if(strcmp(*liste.cmds_array,"cd")==0){ // todo faire une fonction qui choisit le bon appel ?
        if(liste.taille_array>3){
            perror("Trop d'arguments pour la commande cd");
            exit(EXIT_FAILURE);
        }else if(liste.taille_array == 1){
            printf("cd pas d'option pas de path\n");
            process_cd("-L", home);
        }else if(liste.taille_array == 3) {
            printf("cd option\n");
            process_cd(*(liste.cmds_array + 1), *(liste.cmds_array + 2));
        }else{ // quand il n'y a pas d'option/ pas de path
            printf("cd pas d'option/ pas de path\n");
            if(strcmp(*(liste.cmds_array + 1),"-L") == 0 || strcmp(*(liste.cmds_array + 1),"-P") == 0){ // pas de path
                process_cd(*(liste.cmds_array + 1), home);
            }else {// pas d'option
                process_cd("-L", *(liste.cmds_array + 1));
            }
        }
    }
    else if(strcmp(*liste.cmds_array,"pwd")==0){
        if(liste.taille_array>2){
            perror("Trop d'arguments pour la commande pwd");
            exit(EXIT_FAILURE);
        }
        // appel de pwd avec *(liste.cmds_array+1)
        DIR* dir;
        int size = liste.taille_array - 1;
        if(size > 0 && (strcmp(liste.cmds_array[1],"-P")==0)){
          dir = opendir(".");
          printf("%s\n",pwd_physique(dir));
        }
        else{
          printf("%s\n", pwd);
        }
    }
    else{
        if(liste.taille_array>2){
            perror("Trop d'arguments pour la commande exit");
            exit(EXIT_FAILURE);
        }
        int exit_value = atoi(*(liste.cmds_array+1));
        exit(exit_value);
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
    char* curr_path_cpy=getenv("PWD");
    size_t curr_path_size=strlen(curr_path_cpy);
    char* tmp_curr=malloc(sizeof(char)*25);
    char* new_curr=malloc(sizeof(char)*25);

    if(curr_path_size>25){
        size_t beginning_size=curr_path_size-22;
        tmp_curr=strcpy(tmp_curr,curr_path_cpy+beginning_size);
        sprintf(new_curr,"...%s",tmp_curr);
    }
    else{
        new_curr=strcpy(tmp_curr,curr_path_cpy);
    }

    char* res=malloc(sizeof(char)*45);

    sprintf(res,"\001\033[32m\002[%d]\001\033[36m\002%s\001\033[00m\002$ ",errorCode,new_curr);
    free(new_curr);
    free(tmp_curr);
    return res;
}



void run(){
    rl_outstream=stderr;
    home = getenv("HOME");
    pwdPhy = malloc(1024);
    pwdPhy = getenv("PWD");
    pwd = getenv("PWD");
    oldpwd = getenv("OLDPWD");

    char* ligne;
    while(1){
        ligne=readline(promptGeneration());
        if(ligne && *ligne){
            add_history(ligne);

            cmds_struct liste=lexer(ligne);


            interpreter(liste);
        }
        free(ligne);
    }
}

int main(void) {
    run();
}
