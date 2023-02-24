/* librairie standard ... */
#include <stdlib.h>
/* pour getopt */
#include <unistd.h>
/* déclaration des types de base */
#include <sys/types.h>
/* constantes relatives aux domaines, types et protocoles */
#include <sys/socket.h>
/* constantes et structures propres au domaine UNIX */
#include <sys/un.h>
/* constantes et structures propres au domaine INTERNET */
#include <netinet/in.h>
/* structures retournées par les fonctions de gestion de la base de
données du réseau */
#include <netdb.h>
/* pour les entrées/sorties */
#include <stdio.h>
/* pour la gestion des erreurs */
#include <errno.h>
#define LONGSTRING 5


void afficher_infos(int src ,int lg_msg, int port, int nb, int TP, char* dest_hostname){
    printf("%s", src?"SOURCE: ":"PUITS: ");

    char* str_nb = malloc(10*sizeof(char));
    if (nb!=-1){
        snprintf(str_nb,9,"%d",nb);
    } else {
        memcpy(str_nb,"infiny",7);
    }

    printf("%s=%d, port=%d, %s=%s, TP=%s",
    src? "lg_mesg_emis":"lg_mesg_recus", lg_msg,
    port,
    src? "nb_envois":"nb_receptions", str_nb ,
    TP?"TCP":"UDP");
    if (src) printf(", dest=%s", dest_hostname);
    printf("\n");
}

char* construire_numero_message(int numero_entree){
    int numero = numero_entree%99999;
    int i = 0;
    char* numero_formate = malloc(5*sizeof(char));

    if (numero<=9999){
        numero_formate[i++] = '-';
    } 
    if (numero <= 999){
        numero_formate[i++] = '-';
    }
    if (numero <= 99){
        numero_formate[i++] = '-';
    } 
    if (numero <= 9){
        numero_formate[i++] = '-';
    }

    sprintf(&numero_formate[i],"%d", numero);
    return numero_formate;
}

char* construire_message(int numero, int lg) {
	char * message = malloc(lg*sizeof(char));
	int i=5;
    char motif = (char)(((numero-1)%26) + 97); 

    memcpy(message, construire_numero_message(numero),i);

	for (;i<lg;i++) message[i] = motif;
	return message;
	}

void afficher_message(int src, char *message, int lg, int numero) {
	int i;
    if (src){
        printf("SOURCE: Envoi n°%d (%d) [",numero,lg);
    }else {
        printf("PUITS: Reception n°%d (%d) [",numero,lg);
    }
    
	for (i=0;i<lg;i++) printf("%c", message[i]); 
	printf("]\n");
	}

int getPort(int argc, char **argv){
		int port = atoi(argv[argc-1]);
		port = htons(port);
		return port;
	}

int createSocket(int Type){
	int le_socket;
	if (Type==SOCK_DGRAM){
		if ((le_socket = socket(AF_INET, Type, 0)) == -1){ 
			printf("échec de création du socket steam\n") ;
			exit(1) ; 
		}
	}
	else if (Type==SOCK_STREAM){
		if ((le_socket = socket(AF_INET, Type, 0)) == -1){ 
			printf("échec de création du socket datagram\n") ;
			exit(1) ; 
		}
	}
	else{
		printf("il faut définir le socket");
		exit(1);
	}
	return le_socket;
}

char * getHostname(int argc, char** argv){
	char * adresse_dest = argv[argc-2];
	return adresse_dest;
}

struct hostent * getIP(char * adresse_dest){
	struct hostent * hp;
	if ((hp = gethostbyname(adresse_dest)) == NULL){
		printf("erreur gethostbyname\n") ;
		exit(1) ; 
	}
	return hp;
}

struct sockaddr_in* createDistantAddress(struct hostent * hp, int port){
	struct sockaddr_in* adr_distant = malloc(sizeof(struct sockaddr_in));

	adr_distant->sin_family = AF_INET ; /* domaine Internet*/
	adr_distant->sin_port = port;
	memcpy((char*)&adr_distant->sin_addr.s_addr, hp->h_addr, hp->h_length);

	return adr_distant;
}

void envoyerEnUdp(int le_socket, struct sockaddr_in* adr_distant, int lg_message, int nombreMessages ){
    int envoi;
	char* message;
    for (int i =1 ; (i <= nombreMessages) ; i++ ){
		message = construire_message(i,lg_message);
        envoi = sendto(le_socket, message, lg_message, 0, (struct sockaddr *)adr_distant,sizeof(struct sockaddr_in) );
		if ( envoi == -1 ) {
			printf("envoi échoué\n");
			printf("%s\n", strerror(errno));
			exit(1);
		}
		afficher_message(1,message,envoi,i);
    }
	printf("SOURCE : fin\n");

	//printf("succes de l'envoi, envoi de %d messages de %d caracteres\n", nombreMessages, envoi);
}

void envoyerEnTCP(int le_socket, struct sockaddr_in * adr_distant, int lg_message ,int nbr_messages){
    int envoi;
	char* message;
    
    if (connect(le_socket, (struct sockaddr *)adr_distant, sizeof(struct sockaddr_in))==-1){
        printf("connection échouée\n");
        exit(1);
    }
    for (int i=1; i<=nbr_messages; i++){
        message = construire_message(i,lg_message);
        envoi = send(le_socket, message, lg_message,0);
        if (envoi==-1){
            printf("écriture échouée\n");  
            exit(1);
        }
		afficher_message(1,message,envoi,i);
    }
    shutdown(le_socket, 2); 
	printf("SOURCE : fin\n");

}


struct sockaddr_in* createLocalAddress(int port){
	struct sockaddr_in* adr_local = malloc(sizeof(struct sockaddr_in));
	adr_local->sin_family = AF_INET;
	adr_local->sin_port = port;
	adr_local->sin_addr.s_addr = INADDR_ANY;

	return adr_local;
}

void recevoirMessageUDP(int le_socket, int lg_message, int nb_message){
	char* message_recu = malloc(lg_message*sizeof(char));
	struct sockaddr * padr_em = malloc(sizeof(struct sockaddr)) ;
	unsigned int* plg_adr_em = malloc(sizeof(int));
    int recus = 0;
	int octets_recus;

    while((octets_recus=recvfrom(le_socket,message_recu, lg_message, 0, padr_em, plg_adr_em )) && (recus<nb_message || nb_message==-1 )){
        if (octets_recus==-1){
			printf("erreur lors de la lecture des messages reçus\n");
			exit(1);
		}
		afficher_message(0,message_recu,octets_recus,(recus+1));
        printf("\n");
        recus++;
    }
	printf("PUITS : fin\n");
	//printf("%d Messages a/ont été(s) reçus \n", recus);
}

void recevoirMessageTCP(int le_socket, int taille_buffer, int tailleMessage, int nb_messages){
    struct sockaddr * padr_client = malloc(sizeof(struct sockaddr)) ;
    unsigned int* plg_adr_client = malloc(sizeof(int));
    int socket_client;
    char * message_recu = malloc(tailleMessage* sizeof(char)) ;
    int nbr_octets_lus;
    int recus = 0;


    if (listen(le_socket, taille_buffer)==-1){
        printf("échec du dimensionnement du buffer\n");
        exit(1);
    }

    socket_client = accept(le_socket, padr_client, plg_adr_client);
    if (socket_client==-1){
        printf("échec de l'accept\n");
        exit(1);
    }
 

    while ((nbr_octets_lus = recv(socket_client, message_recu, tailleMessage,0)) && (recus<nb_messages || nb_messages==-1)){
        if (nbr_octets_lus==-1){
            printf("reception échouée\n");
            perror("recv");
            exit(1);
        }
        else {
            afficher_message(0,message_recu,nbr_octets_lus,(recus+1));
        }
        recus++;
    }
	printf("SOURCE : fin\n");
	//printf("%d Messages a/ont été(s) reçus\n", recus);

}

int main (int argc, char **argv) {
	int c;
	extern char *optarg;
	extern int optind;
    int lg_message= 30; //Nb d'octets par message à envoyer et à recevoir par défaut: 30;
	int nb_message = -1; /* Nb de messages à envoyer ou à recevoir, par défaut : 10 en émission, infini en réception */
	int source = -1 ; /* 0=puits, 1=source */
    int transf_type = 1; /* 0=UDP, 1=TCP */
	
	// Variables
	int le_socket;
	int port;
	char * hostname;
	struct hostent *hp ;
    

	//En envoi
	struct sockaddr_in* adr_distant;

	//Reception
	struct sockaddr_in* adr_local;

	

	while ((c = getopt(argc, argv, "pn:l:su")) != -1) {
		switch (c) {
			case 'u':
                transf_type = 0;
				le_socket = createSocket(SOCK_DGRAM);
				break;
			case 'p':
				source = 0;
				if (source == 1) {
					printf("usage: cmd [-p|-s][-n ##]\n");
					exit(1);
				}
				break;
			case 's':
				source = 1;
				if (source == 0) {
					printf("usage: cmd [-p|-s][-n ##]\n");
					exit(1) ;
				}
				break;

			case 'n':
				nb_message = atoi(optarg);
				break;
            case 'l':
                if ((lg_message = atoi(optarg))<5){
					printf("Le message doit contenir au minimum 5 octets\n");
					exit(1);
				}
                break;
			default:

				break;
			}
		}

	if (source == -1) {
		printf("usage: cmd [-p|-s][-n ##]\n");
		exit(1) ;
	}
    if (transf_type==1){
        le_socket=createSocket(SOCK_STREAM);
    }

	if (nb_message == -1 && source == 1) {
	nb_message = 10 ;
	} 

	//On récupère le numéro de port
	port =getPort(argc, argv);


	if (source == 1) {

		//On récupère le nom de la machine destinataire
        hostname=getHostname(argc, argv);
		//On résoud son adresse
        hp=getIP(hostname);
		//On fabrique l'objet qui contient l'adresse distante
        adr_distant = createDistantAddress(hp, port);


	}
	else {

		//On construit l'objet qui contient l'adresse locale
		adr_local= createLocalAddress(port);
		//On l'associe avec la représentation interne du port
		if (bind(le_socket, (struct sockaddr *)adr_local, sizeof(struct sockaddr_in)) == -1){
            perror("bind");
			printf("échec du bind\n") ;
			exit(1) ; 
			}

	}



	afficher_infos(source, lg_message,port,nb_message,transf_type, hostname);

    if (transf_type){   //En TCP
        if (source == 1){
            envoyerEnTCP(le_socket, adr_distant,lg_message,nb_message);
        }
        else {
            recevoirMessageTCP(le_socket, nb_message*lg_message, lg_message, nb_message);
        }
        
    }
    else {  //En UDP
        if (source == 1){
            envoyerEnUdp(le_socket, adr_distant, lg_message, nb_message);
        }
        else {
            recevoirMessageUDP(le_socket, lg_message, nb_message);
        }
        
    }

    return 0;
}

