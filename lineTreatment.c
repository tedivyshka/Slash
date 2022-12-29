#include "lineTreatment.h"
#include "pipeline.h"
#include "redirection.h"


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
    test_Arg_Len(res);
    return res;

}

char ** doubleAsteriskParcours(char * path, char * suffixe){
    //printf("------------------DoubleAsteriskParcours------------------\n");

    char **res = malloc(sizeof(char *) * 1);
    res[0] = NULL;


    //cas ou ** ne représente rien
    if(strcmp(path,"") == 0){
        char * s = malloc(sizeof(char) * (strlen(suffixe) + 1));
        strcpy(s,suffixe);

        char ** temp = copyStringArray(res);
        char ** replaceFirst = replaceAsterisk(s);
        freeArray(res);
        res = combine_char_array(temp, replaceFirst);
        freeArray(replaceFirst);
        freeArray(temp);
        free(s);

    }

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
        // cas où l'entrée est un lien symbolique
        if (entry->d_type == DT_LNK) continue;

        // cas où général

        //printf("entry = repertory\n");
        // on appelle replaceAsterisk avec path/completeSuffixe
        // si le chemin n'existe pas, cet appel renverra un char ** vide
        // sinon, il renverra les chemins possibles
        char * newString;

        //si il y a un chemin avant
        if(strcmp(path,"") != 0){
            // si il y a un suffixe
            if(strcmp(suffixe,"") != 0){
                newString = malloc(sizeof(char) * (strlen(path) + strlen(entry->d_name) + strlen(suffixe) + 3));
                testMalloc(newString);
                sprintf(newString, "%s/%s/%s", path, entry->d_name, suffixe);
            }else{
                //pas de suffixe
                newString = malloc(sizeof(char) * (strlen(path) + strlen(entry->d_name) + 2));
                testMalloc(newString);
                sprintf(newString, "%s/%s", path, entry->d_name);
            }
        }else{
            // pas de chemin avant

            // si il y a un suffixe
            if(strcmp(suffixe,"") != 0){
                newString = malloc(sizeof(char) * (strlen(entry->d_name) + strlen(suffixe) + 2));
                testMalloc(newString);
                sprintf(newString, "%s/%s", entry->d_name, suffixe);
            }else{
                //pas de suffixe
                newString = malloc(sizeof(char) * (strlen(entry->d_name) + 1));
                testMalloc(newString);
                sprintf(newString, "%s", entry->d_name);
            }
        }

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
        if(entry->d_type == DT_DIR) {

            char *newPath;
            if (strcmp(path, "") == 0) {
                newPath = malloc(sizeof(char) * (strlen(entry->d_name) + 1));
                sprintf(newPath, "%s", entry->d_name);
            } else {
                newPath = malloc(sizeof(char) * (strlen(path) + strlen(entry->d_name) + 2));
                sprintf(newPath, "%s/%s", path, entry->d_name);
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
    closedir(dir);


    //print_char_double_ptr(res);
    test_Arg_Len(res);
    return res;
}

char ** replaceDoubleAsterisk(char * asteriskString){
    //printf("--------------------------------------- replaceDoubleAsterisk ------------------------------------------------\n");
    char ** res = malloc(sizeof(char *) * 1);
    res[0] = NULL;
    char * suffixe;
    if(strlen(asteriskString) > 3) suffixe = getSuffixe(3, asteriskString);
    else {
        suffixe = malloc(sizeof(char) * 1);
        suffixe[0] = '\0';
    }
    //printf("suffixe = %s\n",suffixe);
    char** tmp = copyStringArray(res);
    freeArray(res);
    char** replace = doubleAsteriskParcours("", suffixe);
    res = combine_char_array(tmp, replace);
    freeArray(tmp);
    freeArray(replace);
    free(suffixe);

    test_Arg_Len(res);
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

/**
 * Compare a string to multiple redirection signs (<,>,>|,>>,2>,2>|,2>>)
 * @param str the string to compare to
 * @return boolean: 1=true 0=false
 */
int strcmp_redirections(char* str) {
    char *list[] = {"<", ">", ">|", ">>", "2>", "2>|", "2>>"};

    for (int i = 0; i < 7; ++i) {
        if (strcmp(list[i], str) == 0) {
            return 1;
        }
    }
    return 0;
}

size_t pipes_quantity(cmd_struct cmd){
    size_t pipes=0;
    for(int i=0; i<cmd.taille_array; ++i){
        if(strcmp(*(cmd.cmd_array+i),"|")==0){
            // handle the error where a pipe is the last argument
            if(i==cmd.taille_array-1){
                syntax_error=1;
                errorCode=2;
            }
            pipes++;
        }
    }
    return pipes;
}

/**
 * Creates a cmds_struct with the commands separated by the redirection symbols.
 * example : [0] "ls -l | wc -l" -> [0] "ls -l" [0] "|" [0] "wc -l"
 * with each [] a new array
 * @param cmd the struct with an array of strings of the command line
 * @return cmds_struct with the commands separated
 */
cmds_struct separate_pipes(cmd_struct cmd){
    cmds_struct res;
    res.taille_array=0;
    size_t pipes=pipes_quantity(cmd);
    cmd_struct* cmds_array=malloc(sizeof(cmd_struct)*(pipes+1));

    // count of the strings in cmd
    int string_count=0;
    // count of the strings on the last array of res
    int string_count_cmds=0;
    // index of the last copied string
    int size_tmp=0;
    while(string_count<cmd.taille_array){
        // when the current string is a redirection symbol
        if(strcmp(*(cmd.cmd_array+string_count),"|")==0){
            // if there's two pipes next to each other, put an empty string
            if(string_count==size_tmp){
                (cmds_array+res.taille_array)->cmd_array=malloc(sizeof(char*));
                *(cmds_array+res.taille_array)->cmd_array=malloc(sizeof(char));
                cmds_array[res.taille_array].cmd_array[0][0]='\0';
                (cmds_array+res.taille_array)->taille_array=1;
                syntax_error=1;
                errorCode=2;
            }
            // copy all strings before the symbol in one array
            else{
                (cmds_array+res.taille_array)->cmd_array=copyNStringArray((cmd.cmd_array+size_tmp),string_count_cmds);
                size_tmp+=string_count_cmds;
                (cmds_array+res.taille_array)->taille_array=string_count_cmds;
            }

            res.taille_array++;
            size_tmp+=1;
            string_count_cmds=0;
        }
        else{
            string_count_cmds++;
        }
        string_count++;
    }

    // copy either the last part of the string or the string without redirections
    (cmds_array+res.taille_array)->cmd_array=copyNStringArray((cmd.cmd_array+size_tmp),string_count_cmds);
    (cmds_array+res.taille_array)->taille_array=string_count_cmds;
    res.taille_array++;
    res.cmds_array=cmds_array;
    return res;
}

/**
 * Interprets the commands to call the corresponding functions.
 * @param liste struct for the command
 */
void interpreter(char* first_command,cmds_struct separated_list) {
    if(syntax_error==1){
        dprintf(STDERR_FILENO,"slash: syntax error\n");
    }
    else{
        if(separated_list.taille_array>1){
            handle_pipe(separated_list);
        }
        else if(strcmp(first_command,"cd")==0 || strcmp(first_command,"pwd")==0 || strcmp(first_command,"exit")==0){
            handle_redirection_intern(*separated_list.cmds_array);
        }
        else{
            handle_redirection_extern(*separated_list.cmds_array);
        }
    }
}

/**
 * Verify syntax for the standard input and output
 * @param cmds the list of commands to verify
 */
void verify_syntax(cmds_struct cmds){
    for(int i=0; i<cmds.taille_array; ++i){
        for(int j=0; j<cmds.cmds_array[i].taille_array; ++j){
            if((strcmp(cmds.cmds_array[i].cmd_array[j],">")==0
            || strcmp(cmds.cmds_array[i].cmd_array[j],">>")==0
            || strcmp(cmds.cmds_array[i].cmd_array[j],">|")==0)
            && i!=(cmds.taille_array-1)){
                syntax_error=1;
                errorCode=2;
                return;
            }
            if((strcmp(cmds.cmds_array[i].cmd_array[j],">")==0
                || strcmp(cmds.cmds_array[i].cmd_array[j],">>")==0
                || strcmp(cmds.cmds_array[i].cmd_array[j],">|")==0)
                && j==(cmds.cmds_array[i].taille_array-1)){
                syntax_error=1;
                errorCode=2;
                return;
            }
            if(strcmp(cmds.cmds_array[i].cmd_array[j],"<")==0 && i!=0){
                syntax_error=1;
                errorCode=2;
                return;
            }
        }
    }
}

void joker_expansion(cmd_struct liste){
    //printf("joker_expansion \n");
    //on commence par ajouter la commande dans le tableau args
    char ** args = malloc(sizeof(char *));
    *(args) = NULL;
    // on combine args avec le nouveau char ** représentant les chaines obtenues en remplaçant * dans un argument de la liste.
    for(int i = 0; i < liste.taille_array; i++){
        // la fonction combine_char_array renvoie un nouveau pointeur char ** (malloc à l'intérieur)
        // la fonction replaceAsterisk renvoie un char ** (malloc à l'intérieur).
        char * suppressed_slash = supprimer_occurences_slash(*(liste.cmd_array+i));
        char** replace;
        // si suppressed_slash commence par ** on appelle replaceDoubleAsterisk
        if(strlen(suppressed_slash) > 1 && suppressed_slash[0] == '*' && suppressed_slash[1] == '*'){
            replace = replaceDoubleAsterisk(suppressed_slash);
        }else{ // sinon on utilise replaceAsterisk
            replace = replaceAsterisk(suppressed_slash);
        }

        //si n'a aucun string, alors donner liste.cmd_array[i]
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
    cmd_struct new_liste;
    size_t tailleArray = 0;
    while(args[tailleArray] != NULL) tailleArray ++;
    new_liste.taille_array = tailleArray;
    new_liste.cmd_array = args;

    test_Arg_Len(new_liste.cmd_array);

    cmds_struct separated_list=separate_pipes(new_liste);
    verify_syntax(separated_list);
    interpreter(*new_liste.cmd_array,separated_list);
    syntax_error=0;
    freeCmdsArray(separated_list);
    freeCmdArray(new_liste);
}


/***
 * Turns a line into a command structure.
 * @param ligne line from the prompt
 * @return struct cmd_struct
 */
cmd_struct lexer(char* ligne){
    char** cmd_array=malloc(sizeof(char*));
    testMalloc(cmd_array);
    char* token;
    size_t* taille_array_init= malloc(sizeof(size_t));
    testMalloc(taille_array_init);
    size_t taille_token;
    size_t taille_array=0;
    *taille_array_init=1;

    // take each string separated by a space and copy it into a list of strings
    token=strtok(ligne," ");
    do{
        taille_token=strlen(token);
        if(taille_token>=MAX_ARGS_NUMBER){
            perror("MAX_ARGS_NUMBER REACHED");
            exit(1);
        }

        cmd_array=checkArraySize(cmd_array,taille_array,taille_array_init);
        *(cmd_array+taille_array)=malloc(sizeof(char)*(taille_token+1));
        *(cmd_array+taille_array)=memcpy(*(cmd_array+taille_array),token,taille_token+1);
        taille_array++;
    }
    while((token = strtok(NULL, " ")));


    free(token);
    free(taille_array_init);
    cmd_struct cmdStruct = {.cmd_array=cmd_array, .taille_array=taille_array};
    return cmdStruct;
}
