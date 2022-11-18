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

typedef struct cmds_struct{
    char** cmds_array;
    size_t taille_array;
}cmds_struct;

int errorCode=0;
char* PWD="";

void testMalloc(void * ptr){
    if(ptr == NULL){
        printf("Erreur de malloc() ou realloc().\n");
        exit(EXIT_FAILURE);
    }
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
    if(strcmp(*liste.cmds_array,"cd")==0){
        if(liste.taille_array>3){
            perror("Trop d'arguments pour la commande cd");
            exit(EXIT_FAILURE);
        }
        // appel de cd avec *(liste.cmds_array+1) et *(liste.cmds_array+2)
    }
    else if(strcmp(*liste.cmds_array,"pwd")==0){
        if(liste.taille_array>2){
            perror("Trop d'arguments pour la commande pwd");
            exit(EXIT_FAILURE);
        }
        // appel de pwd avec *(liste.cmds_array+1)
        DIR* dir;
        int size = liste.taille_array - 1;
        if(size > 0 && strcmp(liste.cmds_array[0],"-P")){
          dir = opendir(".");
          printf("%s\n",pwd_physique(dir));
        }
        else{
          //Handle pwd -L
        }
    }
    else{
        if(liste.taille_array>2){
            perror("Trop d'arguments pour la commande exit");
            exit(EXIT_FAILURE);
        }
        // appel de exit avec *(liste.cmds_array+1)
    }
}

cmds_struct lexer(char* ligne){
    char** cmds_array= malloc(sizeof(char*));
    char c=*ligne;
    size_t* taille_string_init= malloc(sizeof(size_t));
    size_t* taille_array_init= malloc(sizeof(size_t));
    size_t taille_array=0,taille_string=0,compteur=0;
    *taille_string_init=2;
    *taille_array_init=1;

    while(c != EOF && c != '\0'){
        if(isspace(c)){
            taille_string=0;
            compteur++;
            c=*(ligne+compteur);
        }
        else{
            if(taille_string==0){
                if(isspace(*(ligne+compteur-1))){
                    taille_array++;
                }
                cmds_array=checkArraySize(cmds_array,taille_array,taille_array_init);

                *(cmds_array+taille_array)=malloc(sizeof(char)*2);
                testMalloc(*(cmds_array+taille_array));
                /*
                if(*(cmds_array+taille_array)==NULL)
                    perror("malloc");
                    */
                **(cmds_array+taille_array)=c;
                taille_string++;
            }
            else{
                *(cmds_array+taille_array)=checkStringSize(*(cmds_array+taille_array),taille_string,taille_string_init);

                *((*(cmds_array+taille_array))+taille_string)=c;

                taille_string++;
            }

            if(isspace(*(ligne+compteur+1)) || *(ligne+compteur+1)=='\0'){
                *(cmds_array+taille_array)=reallocToSizeString(*(cmds_array+taille_array),taille_string);
            }

            compteur++;
            c=*(ligne+compteur);
        }
    }
    taille_array++;
    if(taille_array!=0 && *taille_array_init!=taille_array)
        cmds_array=reallocToSizeArray(cmds_array,taille_array);
    cmds_struct cmdsStruct = {.cmds_array=cmds_array, .taille_array=taille_array};
    return cmdsStruct;
}


char* promptGeneration(){
    char* res=malloc(sizeof(char)*40);
    sprintf(res,"\001\033[32m\002[%d]\001\033[36m\002.../...\001\033[00m\002$ ",errorCode);
    return res;
}

void run(){
    //rl_outstream=stderr; NE FONCTIONNE PAS IDK WHY
    char* ligne;
    while(1){
        ligne=readline(promptGeneration());
        if(ligne && *ligne){
            add_history(ligne);

            cmds_struct liste=lexer(ligne);

            //printListe(liste);

            interpreter(liste);
        }
        free(ligne);
    }
}



int main(void) {
    run();
}
