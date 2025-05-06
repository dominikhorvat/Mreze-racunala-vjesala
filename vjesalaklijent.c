#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "vjesalaprotokol.h"

//ZA RANG LISTU KOJU PRIMA-----------
struct Unos {
    char ime[50];
    int rezultat;
};
//-----------------------------------

void obradiSTAVI(int sock);
void obradiPOGODI(int sock);
void obradiODUSTANI(int sock);

void primiRANG(int sock){
    char primiint[100];
    int velicinastruct;
    int vrstaPoruke;
    char *odgovor;

    primiPoruku(sock,&vrstaPoruke,&odgovor);
    sscanf(odgovor, "%d",&velicinastruct);

    struct Unos rangLista[velicinastruct];
    int i;
    printf("Rang lista:\n");
    for(i=0;i<velicinastruct;i++){
            primiPoruku(sock,&vrstaPoruke,&odgovor);
        printf("%d . %s\n", i+1, odgovor);
    }

}

int main(int argc, char **argv){
    if(argc !=3)
        error2("Upotreba: %s IP-adresa port\n", argv[0]);
    char dekadskiIP[20];
    strcpy(dekadskiIP,argv[1]);
    int port;
    sscanf(argv[2],"%d",&port);

    //uticnica sa socket
    int mojSocket=socket(PF_INET,SOCK_STREAM,0);
    if(mojSocket==-1)
        myperror("socket");

    //connect
    struct sockaddr_in adresaServera;

    adresaServera.sin_family=AF_INET;
    adresaServera.sin_port=htons(port);
    if(inet_aton(dekadskiIP, &adresaServera.sin_addr)==0)
        error2("%s nije dobra adresa!\n", dekadskiIP);
    memset(adresaServera.sin_zero,'\0',8);

    if(connect(mojSocket, (struct sockaddr *)&adresaServera, sizeof(adresaServera))==-1)
            myperror("connect");

    //sada ispisemo korisniku menu
    char *poruka;
    int vrstaPoruk;
    //trebamo primiti onu poruku posaljiPoruku(sock,ODGOVOR,za_poslati); 13.11.2023.
    primiPoruku(mojSocket,&vrstaPoruk,&poruka);
    printf("%s",poruka);

    int gotovo=0;
    while(!gotovo){
        printf("\n\nOdaberi opciju...\n"
                        "   1. pogodi slovo za rijec\n"
                        "   2. pogodi odmah rijec\n"
                        "   3. odustani(predaj) se\n"
                        "   \n: ");
    int opcija;
    scanf("%d",&opcija);

    //sad imamo onaj switch
    switch(opcija){
        case 1: obradiSTAVI(mojSocket); break;
        case 2: obradiPOGODI(mojSocket); break;
        case 3: obradiODUSTANI(mojSocket); gotovo=1; break;
        default: printf("Pogresna opcija...\n"); break;
        }
    }

    close(mojSocket);

    return 0;
}


void obradiSTAVI(int sock){
    char* slovo;
    printf("Koje slovo zelite isprobati: " );
    scanf(" %s", slovo);
   // char slovo_string[1000]; //jer zelimo slati string 2,'\0' zbog funkije posalji
   // slovo_string=slovo; //ajmo da moze unijeti bilo koliko i to poslati


    char poruka[1000];
    sprintf(poruka,"%s",slovo);
    int brojpok;
    posaljiPoruku(sock, STAVI, poruka);

    int vrstaPoruke;
    char *odgovor;

    if( primiPoruku( sock, &vrstaPoruke, &odgovor ) != OK )
		error1( "Doslo je do pogreske u komunikaciji sa serverom...\n" );

    if( vrstaPoruke != ODGOVOR )
		error1( "Doslo je do pogreske u komunikaciji sa serverom (nije poslao ODGOVOR)...\n" );

	//if( strcmp( odgovor, "OK" ) != 0 )
	//	printf( "Greska: %s\n", odgovor ); //!?
		printf( "%s", odgovor );
        sscanf(odgovor,"Preostali broj pokusaja: %d", &brojpok);

	    //rang lista i sve to ili pogodili ste rijec!

        if(brojpok==0){
                primiRANG(sock);
        }

	free( odgovor );
    }

void obradiPOGODI(int sock){
    char rjesenje[50];
    int brojpok;
    printf("Upisite rijec za koju mislite da je konacan odgovor: ");
    scanf(" %s", rjesenje);

    char poruka[100];
    sprintf(poruka,"%s",rjesenje);

    posaljiPoruku(sock, POGODI, poruka);

    int vrstaPoruke;
    char *odgovor;

    if( primiPoruku( sock, &vrstaPoruke, &odgovor ) != OK )
		error1( "Doslo je do pogreske u komunikaciji sa serverom...\n" );

	if( vrstaPoruke != ODGOVOR )
		error1( "Doslo je do pogreske u komunikaciji sa serverom (nije poslao ODGOVOR)...\n" );

	if( strcmp( odgovor, "OK" ) != 0 ){
		printf( "Greska: %s\n", odgovor );
		free( odgovor );
		return;
	}
	free(odgovor);

	if( primiPoruku( sock, &vrstaPoruke, &odgovor ) != OK )
		error1( "Doslo je do pogreske u komunikaciji sa serverom...\n" );

		sscanf( odgovor, "%s", poruka );
		printf( "%s", poruka );
        sscanf(odgovor,"Preostali broj pokusaja: %d", &brojpok);
        //rang lista i sve to ili pogodili ste rijec!
        if(brojpok==0){
        primiRANG(sock);
        }


	free( odgovor );
}
void obradiODUSTANI(int sock){
    posaljiPoruku(sock, ODUSTANI, "");
    int vrstaPoruke;
    char *odgovor;

    if( primiPoruku( sock, &vrstaPoruke, &odgovor ) != OK )
		error1( "Doslo je do pogreske u komunikaciji sa serverom...\n" );

	if( vrstaPoruke != ODGOVOR )
		error1( "Doslo je do pogreske u komunikaciji sa serverom (nije poslao ODGOVOR)...\n" );

	if( strcmp( odgovor, "OK" ) != 0 )
		printf( "Greska: %s\n", odgovor ); //!?
	else{
	   // printf( "OK\n" );
	   printf("Vasa igra je gotova! Odustali ste, dodjeljeno Vam je 0 bodova!");
       return;
	}
}
