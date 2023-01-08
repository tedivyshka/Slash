## Architecture logicielle

Le programme Slash se compose de 7 fichiers en langage C et un fichier makefile pour la compilation.  
L'exécution de `make` à la racine du dépôt crée (dans ce même répertoire) l'exécutable slash qui peut être lancé avec `./slash`,
`make clean` efface tous les fichiers compilés.

Entre les 7 fichiers C, `slash.c` est le fichier qui initialise les variables et lance la boucle principale du programme.  
Le fichier `utilities.c` contient des fonctions utilitaires utilisées pour gérer les erreurs d'allocation de mémoire ainsi que les tableaux de strings et les structures.

Les autres fichiers sont utilisés par slash pour répondre au sujet, respectivement:  
l'étoile et double étoile dans lineTreatment.c,  
les commandes cd, pwd, exit et commandes externes dans commands.c,  
les signaux dans signal.c,  
les pipeline dans pipeline.c,  
les redirections dans redirection.c.

## Structures de données

Les structures de données que le programme utilise se trouvent dans le fichier `utilities.c` sous cette représentation :  
`struct cmd_struct` représente une commande, cette structure contient une liste (char**) des parties de la commande et la taille (size_t) de cette liste.  
`struct cmds_struct` représente une liste de cmd_struct, cette structure contient une liste (cmd_struct*) des commandes et la taille (size_t) de cette liste. La séparation des commandes entre plusieurs `cmd_struct` s'effectue sur les pipes.  

## Algorithmes implémentés

Après que la ligne de commande ait été récupérée par readline, l'expansion des jokers est effectuée puis la ligne est transformée en structure `cmds_struct` contenant chaque commande et ses arguments.  
Une fonction `interpreter()` dans `lineTreatment.c` va traiter les différents cas possibles de la ligne de commande écrite :   
    - Une ligne de commande contenant des pipes  
    - Une commande interne ou externe avec redirection, ou non  

`handle_pipe()` dans `pipeline.c` va traiter itérativement tous les pipes dans la ligne ainsi que les redirections éventuelles de l'entrée standard, sortie standard et des sorties d'erreur.  
Chaque fils va alors exécuter sa commande et le père va attendre leur fin pour récupérer les valeurs de retour ainsi que l'éventuelle interception d'un signal.  

`handle_redirection_extern()` et `handle_redirection_intern()` dans `redirection.c` sont semblables.  
Les redirections de l'entrée, sortie et erreur standard sont traitées puis la commande est executée.  
Pour les commandes externes, toute la fonction s'effectue dans le processus fils et le père agit comme dans la fonction `handle_pipe()`.


