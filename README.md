Remake of tsock app.

The general usage of the tsock application is as follows:

- `tsock -p [-options] port` implements an information well waiting on port `port`
    
- `tsock -s [-options] host port` implements an information source transmiting to a well running on the host station waiting on port `port`.
    
The common options for the well and the source are:

- `-u`: use the service provided by UDP; by default, the service provided by TCP is used 
    
- -`l ##`: length of data to be transmitted / maximum length of data to be read by the tsock transmitting / receiving application; by default, this length is 30 bytes. 

The specific options for the source (-s) are: 
 
- `-n ##`: sets the number of data transmissions (default: 10) 

The specific options for the well (-p) are: 

 - `-n ##`: sets the number of receptions (default: infinite)
--- 
Recréation de l'application tsock.

L'usage général de l'application tsock est le suivant :

-   `tsock -p [-options] port` met en œuvre un puits d'information en attente sur le port `port` 
    
-   `tsock -s [-options] host port` met en œuvre en œuvre une source d'information vers un puits s'exécutant sur la station host en attente sur le port `port`.
    
Les options communes au puits et à la source sont :

-   `-u` : utilise le service fourni par UDP ; par défaut, le service fourni par TCP est utilisé
    
-   `-l ##` : longueur des données à émettre / longueur maximale des données à lire par l’application tsock émettrice / réceptrice ; par défaut, cette longueur est de 30 octets
  

Les options spécifiques à la source (-s) sont :

-   `-n ##` : définit le nombre d’émissions de données (par défaut : 10)
    
Les options spécifiques au puits (-p) sont:

-   `-n ##` : définit le nombre de réceptions (par défaut : infini)
