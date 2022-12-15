#include <stdio.h>
#include <string.h>
#include "slash.h"


void print_char_double_ptr(char **str) {
    printf("print_char_double_ptr\n");
    int i = 0;
    while (str[i] != NULL) {
        printf("i = %d\n", i);
        printf("%s\n", str[i]);
        i++;
    }
}


/***
 * Temporary function used to debug.
 * @param liste list of string
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