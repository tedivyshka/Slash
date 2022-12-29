#ifndef SLASH_UTILITIES_H
#define SLASH_UTILITIES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_ARGS_NUMBER 4096
#define MAX_ARGS_STRLEN 4096
#define BUFSIZE 1024

// Variables globales :
extern int errorCode;

extern char * pwd;
extern char * oldpwd;
extern char * pwdPhy;
extern char * home;

/***
 * A structure of commands with a list of String and the size of this list.
 */
typedef struct cmd_struct{
    char** cmd_array;
    size_t taille_array;
}cmd_struct;

typedef struct cmds_struct{
    cmd_struct* cmds_array;
    size_t taille_array;
}cmds_struct;


/**
 * Exec perror with a message and exit with error status 1
 * @param msg the perror message
 */
void perror_exit(char* msg);

/***
 * Checks if a malloc has failed.
 * @param ptr pointer to check
 */
void testMalloc(void * ptr);

/**
 * Free an instance of struct cmd_struct.
 * @param array cmds_struct
 */
void freeCmdArray(cmd_struct array);

/**
 * Free an instance of struct cmds_struct
 * @param array
 */
void freeCmdsArray(cmds_struct array);

cmd_struct copyCmdsStruct(cmd_struct liste);

char** copyStringArray(char** liste);

char** copyNStringArray(char** liste,size_t n);

void freeArray(char** array);

/***
 * Double the size allocated to the variable if necessary.
 * @param array string array
 * @param taille_array array size
 * @param taille_array_initiale initial array size
 * @return array parameter after reallocation
 */
char** checkArraySize(char** array,size_t taille_array,size_t* taille_array_initiale);

/***
 * Checks that a path is valid physically.
 * @param path relative or absolute path
 * @return int val to represent boolean value. 0 = True, 1 = False
 */
int isPathValidPhy(char * path);

/***
 * Checks that a path is valid logically.
 * @param path absolute path for a directory
 * @return int val to represent boolean value. 0 = True, 1 = False
 */
int isPathValidLo(char* path);

char** combine_char_array(char** arr1, char** arr2);

void print_char_double_ptr(char **str);

int count_chars(char **strings);

void test_Arg_Len(char ** strings);


#endif //SLASH_UTILITIES_H
