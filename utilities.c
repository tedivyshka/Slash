#include "utilities.h"

// Variables globales :
int errorCode = 0;

char * pwd;
char * oldpwd;
char * pwdPhy;
char * home;


/***
 * Checks if a malloc has failed.
 * @param ptr pointer to check
 */
void testMalloc(void * ptr){
    if(ptr == NULL){
        errno = 1;
        perror("Erreur de malloc() ou realloc().\n");
        exit(EXIT_FAILURE);
    }
}

/***
 * Free an instance of struct cmds_struct.
 * @param array cmds_struct
 */
void freeCmdsArray(cmds_struct array) {
    for (int i = 0; i < array.taille_array; i++) {
        free(*(array.cmds_array + i));
    }
    free(array.cmds_array);
}

cmds_struct copyCmdsStruct(cmds_struct liste){
    cmds_struct cmds_cpy;
    cmds_cpy.taille_array= liste.taille_array;
    cmds_cpy.cmds_array= malloc(sizeof(char *) * liste.taille_array);
    memcpy(cmds_cpy.cmds_array,liste.cmds_array,sizeof(char *)*liste.taille_array);
    for (int i = 0; i < liste.taille_array; i++){
        *(cmds_cpy.cmds_array+i)= malloc(sizeof(char)* strlen(*(liste.cmds_array+i)));
        strcpy(*(cmds_cpy.cmds_array+i),*(liste.cmds_array+i));
    }
    return cmds_cpy;
}

char** copyStringArray(char** liste){
    char** liste_cpy;
    size_t taille=0;
    while(*(liste+taille) != NULL) taille++;
    liste_cpy = malloc(sizeof(char *) * (taille+1));
    memcpy(liste_cpy,liste,sizeof(char *)*taille);
    for (int i = 0; i < taille; i++) {
        *(liste_cpy+i)= malloc(sizeof(char)* (strlen(*(liste+i))+1));
        strcpy(*(liste_cpy+i),*(liste+i));
    }
    *(liste_cpy+taille)=NULL;
    return liste_cpy;
}

void freeArray(char** array) {
    for (int i = 0; *(array+i) != NULL; i++) {
        free(*(array+i));
    }
    free(array);
}

/***
 * Double the size allocated to the variable if necessary.
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
 * @return int val to represent boolean value. 0 = True, 1 = False
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
 * @param path absolute path for a directory
 * @return int val to represent boolean value. 0 = True, 1 = False
 */
int isPathValidLo(char* path){
    struct stat st;
    int res = stat(path,&st);
    return (res == 0 && S_ISDIR(st.st_mode))?0:1;
}



// Function to combine two char** variables into a single char**
char** combine_char_array(char** arr1, char** arr2) {
    // First, count the number of elements in each array
    int len1 = 0;
    for(int i = 0; arr1[i] != NULL; i++) len1++;

    int len2 = 0;
    for(int i = 0; arr2[i] != NULL; i++) len2++;

    // Allocate a new char** array with the combined length of the two input arrays
    char** combined = malloc(sizeof(char*) * (len1 + len2 + 1));
    if(len1 + len2 == 0){
        *combined = NULL;
        return combined;
    }

    // Copy the elements from each input array into the combined array
    int i;
    if(len1 != 0) {
        for (i = 0; i < len1; i++) {
            combined[i] = malloc(sizeof(char) * (strlen(arr1[i]) + 1));
            strcpy(combined[i], arr1[i]);
        }
        if(len2 != 0) {
            for (int j = 0; j < len2; j++) {
                combined[i] = malloc(sizeof(char) * (strlen(arr2[j]) + 1));
                strcpy(combined[i], arr2[j]);
                i++;
            }
        }
    }else if(len2 != 0){
        for (i = 0; i < len2; i++) {
            combined[i] = malloc(sizeof(char) * (strlen(arr2[i]) + 1));
            strcpy(combined[i], arr2[i]);
        }
    }

    // Add a NULL terminator at the end of the combined array
    combined[i] = NULL;
    return combined;
}


void print_char_double_ptr(char **str) {
    printf("print_char_double_ptr\n");
    int i = 0;
    while (str[i] != NULL) {
        printf("i = %d\n", i);
        printf("%s\n", str[i]);
        i++;
    }
}

