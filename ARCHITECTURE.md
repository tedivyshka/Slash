Le programme Slash se compose de 7 fichiers en langage C et un fichier makefile pour compiler.
L'exécution de make à la racine du dépôt crée (dans ce même répertoire) l'exécutable slash qui peut être lancé avec ./slash ,
make clean efface tous les fichiers compilés.

Entre les 7 fichiers C, slash.c est le fichier qui initialise les variables et lance la boucle principale du programme.

Le fichier utilities.c contient des fonctions utilisés pour gérer les erreurs d'allocation de mémoire et aussi les tableaux des strings utilisés.
Les structures de données que le programme utilise se trouvent aussi dans ce fichier:
struct cmd_struct représente une commande, cette structure contient une liste (char**) des parties de la commande et la taille (size_t) de cette liste ;
struct cmds_struct représente une liste de cmd_struct, cette structure contient une liste (cmd_struct*) des commandes et la taille (size_t) de cette liste.
La structure cmds_struct est utilisé dans les pipeline.

Les autres fichiers sont utilisés par slash pour répondre au sujet, réspectivement:
l'étoile et double étoile dans lineTreatment.c ;
les commandes cd, pwd, exit et commandes externes dans commands.c ;
les signaux dans signal.c ;
les pipeline dans pipeline.c ;
les redirection dans redirection.c.

