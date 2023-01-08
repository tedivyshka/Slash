#ifndef SLASH_UTILITIES_H
#define SLASH_UTILITIES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

#define MAX_ARGS_NUMBER 4096
#define MAX_ARGS_STRLEN 4096
#define BUFSIZE 1024

// Variables globales :
extern int errorCode;

extern char * pwd;
extern char * oldpwd;
extern char * pwdPhy;
extern char * home;

/**
 * A structure of command with a list of string and the size of this list.
 */
typedef struct cmd_struct{
    char** cmd_array;
    size_t taille_array;
}cmd_struct;

/**
 * A structure of commands with a list cmd_struct and the size of this list.
 */
typedef struct cmds_struct{
    cmd_struct* cmds_array;
    size_t taille_array;
}cmds_struct;


/**
 * Exec perror with a message and exit with error status 1
 * @param msg the perror message
 */
void perror_exit(char* msg);

/**
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

/**
 * Copy an array of string and NULL terminate it
 * @param liste the array to copy
 * @return the new array of strings
 */
char** copyStringArray(char** liste);

/**
 * Same as copyStringArray() but with n string copied
 * @param liste array
 * @param n the amount of string to copy
 * @return the new array of strings
 */
char** copyNStringArray(char** liste,size_t n);

/**
 * Free an array of string
 * @param array double pointer of char
 */
void freeArray(char** array);

/**
 * Double the size allocated to the variable if necessary.
 * @param array string array
 * @param taille_array array size
 * @param taille_array_initiale initial array size
 * @return array parameter after reallocation
 */
char** checkArraySize(char** array,size_t taille_array,size_t* taille_array_initiale);

/**
 * Checks that a path is valid physically.
 * @param path relative or absolute path
 * @return int val to represent boolean value. 0 = True, 1 = False
 */
int isPathValidPhy(char * path);

/**
 * Checks that a path is valid logically.
 * @param path absolute path for a directory
 * @return int val to represent boolean value. 0 = True, 1 = False
 */
int isPathValidLo(char* path);

/**
 * Function to combine two char** variables into a single char**
 * @param arr1 char ** null terminated
 * @param arr2 char ** null terminated
 * @return char ** null terminated
 */
char** combine_char_array(char** arr1, char** arr2);

/**
 * A function that return the number of char
 * inside a char ** null terminated.
 * @param strings
 * @return count of char in strings
 */
int count_chars(char **strings);

/**
* If the line contains more than MAX_ARGS_STRLEN, the program is exited.
 * @param strings arguments of command line
 */
void test_Arg_Len(char ** strings);

/**
 * Check if the line contains only whitespaces
 * @param ligne the line to check from
 * @return 1 if empty else 0
 */
int is_empty(char* ligne);


#endif //SLASH_UTILITIES_H
