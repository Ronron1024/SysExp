# SysExp

#compilation
Recuperer le repertoire.zip source sur github
Le dezziper sur la machine qui fera office de server
Ce placer dans le repertoire et executer le script de compilation
./compil.sh

#Configuation du server afin de recevoir plusieurs connexions clients
Par exemple avec openSSH

#Lancement du server
Depuis le répertoire, lancer le server
./server

#Connexion au server via ssh
#Par exemple
1-Se connecter au reseau sans fil
ssh -X isen@ip

#execution du client
Depuis le répertoire, lancer le client
./client
