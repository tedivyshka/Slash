#include "lineTreatment.h"

/*todo Pour éviter d'éventuels débordements (notamment lors de l'expansion des jokers),
 * on pourra limiter le nombre et la taille des arguments autorisés sur une ligne de commande slash :
 * #define MAX_ARGS_NUMBER 4096
 * #define MAX_ARGS_STRLEN 4096
 */
// fonction qui renvoie la position de la premiere asterisk dans le string, ou alors -1 s'il n'y en a pas
int getAsteriskPos(char * asteriskString){
    for(int index = 0; index < strlen(asteriskString); index++){
        if(asteriskString[index] == '*') return index;
    }
    return -1;
}
char * getPrefixe(unsigned long pos, char * asteriskString){
    char * prefixe = malloc(sizeof(char) * (pos + 1));
    testMalloc(prefixe);
    if(pos > 0){
        memcpy(prefixe,asteriskString,pos);
    }
    prefixe[pos] = '\0';
    return prefixe;
}

char * getSuffixe(unsigned long pos, char * asteriskString){
    size_t len = strlen(asteriskString);
    size_t sub_len = len - pos;
    char *sub = malloc(sub_len + 1);
    testMalloc(sub);
    strncpy(sub, asteriskString + pos, sub_len);
    sub[sub_len] = '\0';
    return sub;
}


char * getAstefixe(int start, const char * asteriskString){
    int end = start;
    while(asteriskString[end] != '\0' && asteriskString[end] != '/' ) end++;
    char * asterisk = malloc(sizeof(char) * (end - start + 1));
    testMalloc(asterisk);
    memcpy(asterisk,asteriskString + start,end - start);
    asterisk[end - start] = '\0';
    return asterisk;
}

// vérifie que asterisk_string est suffixe de entry_name
int strstrSuffixe(char * entry_name, char * asterisk_string){
    if(strlen(entry_name) < strlen(asterisk_string)) return 0;
    int index = strlen(entry_name) - 1;
    for(int i = strlen(asterisk_string) - 1; i >= 0; i--){
        if(entry_name[index] != asterisk_string[i]) return 0;
        index--;
    }
    return 1;
}


char ** replaceAsterisk(char * asteriskString) {
    //printf("--------------------- replaceAsterisk ---------------------------- \n");
    char ** res = malloc(sizeof(char *) * 1);
    res[0] = NULL;
    int posAsterisk = getAsteriskPos(asteriskString);


    // cas ou il n'y a pas d'asterisk
    if (posAsterisk == -1){
        struct stat st;
        // on vérifie que le chemin existe
        if(lstat(asteriskString,&st) == 0){ // si oui, on va a end
            goto endWithAsterisk;
        }else{
            return res; // sinon on renvoie res (vide)
        }
    }


    // cas ou l'asterisk n'est pas préfixe
    if((posAsterisk != 0 && asteriskString[posAsterisk - 1] != '/')) goto endWithAsterisk;

    /*on cherche à récupérer 3 char * correspondants à
     * la partie avant *
     * la partie ou il y a * (mais sans l'*)
     * la partie apres *
     */
    char *prefixe = getPrefixe(posAsterisk, asteriskString);
    char *asterisk = getAstefixe(posAsterisk + 1, asteriskString);
    unsigned long tailleSuf = strlen(prefixe) + strlen(asterisk) + 1;
    char *suffixe = getSuffixe(tailleSuf, asteriskString);


    //desormais on va ouvrir le repertoire préfixe
    DIR *dir = NULL;
    struct dirent *entry;

    if (strlen(prefixe) == 0) {
        dir = opendir(".");
    } else {
        dir = opendir(prefixe);
    }

    // On vérifie que le repertoire a bien été ouvert.
    // S'il n'est pas ouvert, c'est que le chemin n'est pas valide
    if (dir == NULL) {

        free(prefixe);
        free(suffixe);
        free(asterisk);

        return res;
    }

    // on parcourt les entrées du répertoire
    while ((entry = readdir(dir)) != NULL) {
        // cas où le fichier est '.', '..' ou caché
        if (entry->d_name[0] == '.') continue;
        // cas ou il y a une suite à la sous chaine avec asterisk (*/test par exemple)
        if (strlen(suffixe) != 0) {
            // dans ce cas, si le fichier n'est PAS un répertoire, on passe au suivant
            if (entry->d_type != DT_DIR && entry->d_type != DT_LNK) continue;
        }
        // à chaque correspondance entre le substring et entry, on realloc le tableau
        //
        if (strstrSuffixe(entry->d_name, asterisk) == 1) {
            char *newString = malloc(sizeof(char) * (strlen(prefixe) + strlen(entry->d_name) + strlen(suffixe) + 1));
            testMalloc(newString);
            sprintf(newString, "%s%s%s", prefixe, entry->d_name, suffixe);
            char** tmp = copyStringArray(res);
            char** replace = replaceAsterisk(newString);
            freeArray(res);
            res = combine_char_array(tmp, replace);
            freeArray(replace);
            freeArray(tmp);
            free(newString);
        }
    }
    closedir(dir);

    free(prefixe);
    free(suffixe);
    free(asterisk);

    //printf("sortie normale replaceAsterisk \n");
    //print_char_double_ptr(res);
    return res;

    // cas où l'on doit retourner le chemin donné en argument, seul et inchangé
    endWithAsterisk:
    freeArray(res);
    res = malloc(sizeof(char *) * 2);
    res[0] = malloc(sizeof(char) * strlen(asteriskString) + 1);
    strcpy(res[0], asteriskString);
    res[1] = NULL;

    //printf("sortie end replaceAsterisk \n");
    //print_char_double_ptr(res);
    return res;

}


char ** doubleAsteriskParcours(char * path, char * suffixe){
    //printf("------------------DoubleAsteriskParcours------------------\n");

    char **res = malloc(sizeof(char *) * 1);
    res[0] = NULL;

    DIR *dir = NULL;
    struct dirent *entry;
    if (strcmp(path, "") == 0) dir = opendir(".");
    else dir = opendir(path);
    //printf("opendir\n");

    // on parcourt le repertoire courant
    while ((entry = readdir(dir)) != NULL) {
        //printf("entry = %s\n",entry->d_name);


        // cas où l'entrée est '.', '..' ou cachée
        if (entry->d_name[0] == '.') continue;

        // cas où l'entrée est un repertoire
        if(entry->d_type == DT_DIR){
            //printf("entry = repertory\n");
            // on appelle replaceAsterisk avec path/completeSuffixe
            // si le chemin n'existe pas, cet appel renverra un char ** vide
            // sinon, il renverra les chemins possibles
            char * newString = malloc(sizeof(char) * (strlen(path) + strlen(entry->d_name) + strlen(suffixe) + 3));
            testMalloc(newString);
            sprintf(newString, "%s/%s/%s", path, entry->d_name, suffixe);
            char ** tmp = copyStringArray(res);
            char ** replace = replaceAsterisk(newString);
            freeArray(res);
            res = combine_char_array(tmp, replace);
            freeArray(replace);
            freeArray(tmp);
            free(newString);
            //printf("sortie appel replaceAsterisk\n");

            // maintenant on fait l'appel récursif,
            // pour tester si un autre chemin n'existe pas plus loin dans l'arborescence
            char * newPath;
            if(strcmp(path, "") == 0){
                newPath = malloc(sizeof(char) * (strlen(entry->d_name) + 1));
                sprintf(newPath,"%s",entry->d_name);
            }else{
                newPath = malloc(sizeof(char) * (strlen(path) + strlen(entry->d_name) + 2));
                sprintf(newPath,"%s/%s",path,entry->d_name);
            }
            tmp = copyStringArray(res);
            replace = doubleAsteriskParcours(newPath, suffixe);
            freeArray(res);
            res = combine_char_array(tmp, replace);
            freeArray(replace);
            freeArray(tmp);
            free(newPath);
            //printf("sortie appel res doubleAsteriskParcours\n");


        }
    }


    //print_char_double_ptr(res);
    return res;
}

/*
char ** doubleAsteriskParcours(char * path,char * suffixe, char * shortSuffixe){
    printf("------------------DoubleAsteriskParcours------------------\n");

    char ** res = malloc(sizeof(char *) * 1);
    res[0] = NULL;

    DIR * dir = NULL;
    struct dirent *entry;
    if(strcmp(path, "") == 0) dir = opendir(".");
    else dir = opendir(path);
    printf("opendir\n");
    while((entry = readdir(dir)) != NULL){
        printf("entry = %s\n",entry->d_name);
        // cas où l'entrée est '.', '..' ou cachée
        if (entry->d_name[0] == '.') continue;
        // cas où l'entrée n'est pas un répertoire et qu'on est deja rentré dans un repertoire dans l'appel précédent
        if (entry->d_type != DT_DIR) {
            printf("fichier\n");
            if(strcmp(path,"") != 0) {
                printf("path not \"\"\n");
                // cas etoile ?
                if (suffixe[0] == '*') {
                    // on commence par isoler le suffixe du suffixe (apres *)
                    char *endSuffixe = malloc(sizeof(char) * strlen(suffixe));
                    endSuffixe = getSuffixe(1, suffixe);
                    endSuffixe[strlen(suffixe) - 1] = '\0';

                    //on va comparer les entrées (fichiers) avec endSuffixe
                    if (strstrSuffixe(entry->d_name, endSuffixe) == 1) {
                        printf("correspondance fic / suffixe (avec *)\n");

                        char **add = malloc(sizeof(char *) * 2);
                        add[0] = malloc(sizeof(char *) * (strlen(path) + strlen(entry->d_name) + 2));
                        sprintf(add[0], "%s/%s", path, entry->d_name);
                        add[1] = NULL;
                        char **tmp = copyStringArray(res);
                        freeArray(res);
                        res = combine_char_array(tmp, add);
                        freeArray(add);
                        freeArray(tmp);
                    }
                    free(endSuffixe);
                } else {
                    if (strcmp(entry->d_name, suffixe) == 0) {
                        printf("correspondance fic / suffixe\n");

                        char **add = malloc(sizeof(char *) * 2);
                        add[0] = malloc(sizeof(char *) * (strlen(path) + strlen(suffixe) + 2));
                        sprintf(add[0], "%s/%s", path, suffixe);
                        add[1] = NULL;
                        char **tmp = copyStringArray(res);
                        freeArray(res);
                        res = combine_char_array(tmp, add);
                        freeArray(add);
                        freeArray(tmp);
                    } else {
                        printf("pas de correspondance fic / suffixe\n");
                    }
                }
            }
        }else{         // on fait l'appel récursif sur tous les repertoires
            printf("repertoire\n");
            char * newPath;
            if(strcmp(path, "") == 0){
                newPath = malloc(sizeof(char) * (strlen(entry->d_name) + 1));
                sprintf(newPath,"%s",entry->d_name);
            }else{
                newPath = malloc(sizeof(char) * (strlen(path) + strlen(entry->d_name) + 2));
                sprintf(newPath,"%s/%s",path,entry->d_name);
            }

            char** tmp = copyStringArray(res);
            printf("appel rec DAP avec newPath = %s, suffixe = %s, shortSuffixe = %s\n",newPath,suffixe, shortSuffixe);
            char ** replace = doubleAsteriskParcours(newPath,suffixe,shortSuffixe);
            freeArray(res);
            res = combine_char_array(tmp, replace);
            freeArray(replace);
            freeArray(tmp);
        }

    }
    printf("fin readdir\n");
    closedir(dir);
    return res;

}
*/

char ** replaceDoubleAsterisk(char * asteriskString){
    //printf("--------------------------------------- replaceDoubleAsterisk ------------------------------------------------\n");
    char ** res = malloc(sizeof(char *) * 1);
    res[0] = NULL;
    char * suffixe = getSuffixe(3, asteriskString);
    //printf("suffixe = %s\n",suffixe);
    char** tmp = copyStringArray(res);
    freeArray(res);
    char** replace = doubleAsteriskParcours("", suffixe);
    res = combine_char_array(tmp, replace);
    freeArray(tmp);
    freeArray(replace);
    return res;
}


char* supprimer_occurences_slash(const char *s){
    char *result = malloc(strlen(s) + 1);
    int isPreviousSlash = 0;
    int j = 0;
    for(int i = 0; s[i] != '\0'; i++) {
        if(s[i] == '/'){
            if(isPreviousSlash == 0){
                isPreviousSlash = 1;
                result[j] = s[i];
                j++;
            }
        }else{
            isPreviousSlash = 0;
            result[j] = s[i];
            j++;
        }


    }
    result[j] = '\0';
    return result;
}

/***
 * Interprets the commands to call the corresponding functions.
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
    else{
        process_external_command(liste);
    }
}


void joker_solo_asterisk(cmds_struct liste){
    //printf("joker_solo_asterisk \n");
    //on commence par ajouter la commande dans le tableau args
    char ** args = malloc(sizeof(char *));
    *(args) = NULL;
    // on combine args avec le nouveau char ** représentant les chaines obtenues en remplaçant * dans un argument de la liste.
    for(int i = 0; i < liste.taille_array; i++){
        // la fonction combine_char_array renvoie un nouveau pointeur char ** (malloc à l'intérieur)
        // la fonction replaceAsterisk renvoie un char ** (malloc à l'intérieur).
        char * suppressed_slash = supprimer_occurences_slash(*(liste.cmds_array+i));
        char** replace;
        // si suppressed_slash commence par **/ on appelle replaceDoubleAsterisk
        if(strlen(suppressed_slash) > 2 && suppressed_slash[0] == '*' && suppressed_slash[1] == '*' && suppressed_slash[2] == '/'){
            replace = replaceDoubleAsterisk(suppressed_slash);
        }else{ // sinon on utilise replaceAsterisk
            replace = replaceAsterisk(suppressed_slash);
        }

        //si n'a aucun string, alors donner liste.cmds_array[i]
        if(replace[0] == NULL){
            freeArray(replace);
            replace = malloc(sizeof(char *) * 2);
            replace[0] = malloc(sizeof(char) * (strlen(suppressed_slash) + 1));
            strcpy(replace[0],suppressed_slash);
            replace[1] = NULL;
        }
        char** tmp = copyStringArray(args);
        freeArray(args);
        args = combine_char_array(tmp,replace);
        freeArray(replace);
        freeArray(tmp);
        free(suppressed_slash);
    }
    cmds_struct new_liste;
    size_t tailleArray = 0;
    while(args[tailleArray] != NULL) tailleArray ++;
    new_liste.taille_array = tailleArray;
    new_liste.cmds_array = args;
    interpreter(new_liste);
    freeCmdsArray(new_liste);
}

void joker_duo_asterisk(){

}


/***
 * Turns a line into a command structure.
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

    // take each string separated by a space and copy it into a list of String
    token=strtok(ligne," ");
    do{
        taille_token=strlen(token);
        //todo test taille token
        if(taille_token>=MAX_ARGS_NUMBER){
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