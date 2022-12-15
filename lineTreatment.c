#include "lineTreatment.h"




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

            // on incrémente le nombre d'entrées acceptées
            free(newString);
        }
    }
    closedir(dir);

    free(prefixe);
    free(suffixe);
    free(asterisk);

    return res;

    endWithAsterisk:
    freeArray(res);
    res = malloc(sizeof(char *) * 2);
    res[0] = malloc(sizeof(char) * strlen(asteriskString) + 1);
    strcpy(res[0], asteriskString);
    res[1] = NULL;
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





void joker_solo_asterisk(cmds_struct liste){
    //on commence par ajouter la commande dans le tableau args
    char ** args = malloc(sizeof(char *));
    *(args) = NULL;
    // on combine args avec le nouveau char ** représentant les chaines obtenues en remplaçant * dans un argument de la liste.
    for(int i = 0; i < liste.taille_array; i++){
        // la fonction combine_char_array free ses deux arguments et renvoie un nouveau pointeur char ** (malloc a l'interieur)
        // la fonction replaceAsterisk free son argument et renvoie un char ** (malloc a l'interieur)
        char * suppressed_slash = supprimer_occurences_slash(*(liste.cmds_array+i));
        char** replace = replaceAsterisk(suppressed_slash);
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