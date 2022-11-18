# Projet : programmation d'un interpréteur de commandes

**L3 Informatique - Système**

## Liste des ajouts ou modifications effectués sur le sujet par rapport à sa version initiale

#### Changement de comportement par défaut de `exit`

En l'absence de paramètre, termine le processus `slash` avec comme valeur
de retour celle de ***la dernière commande exécutée***, et non 0 comme demandé
initialement.

#### Comportement de `slash` à la fin de l'entrée standard 

Lorsqu'il atteint la fin de son entrée standard (ie la fin du
fichier si l'entrée standard a été redirigée, ou si l'utilisateur saisit
`ctrl-D` en mode interactif), `slash` se comporte comme si la commande
interne `exit` (sans paramètre) avait été saisie.

#### Valeur de retour des commandes erronées

La valeur de retour d'une commande introuvable (ni comme commande interne, ni
comme commande externe) est 127.

#### Échec d'une redirection

En cas d'échec d'une redirection, la ligne de commande saisie n'est pas
exécutée, et la valeur de retour est 1.

#### Interruption par un signal

Lorsque l'exécution d'une commande est interrompue par la réception d'un
signal, le prompt commence par la chaîne `"[SIG]"` en lieu et place de la 
valeur de retour, qui est considérée comme valant 255 au cas où un appel
à `exit` sans paramètre (ou un `ctrl-D`) suit.

