#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<time.h>
#include<ctype.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include<pthread.h>

#include "vjesalaprotokol.h"

//za prvu funkciju
#define MAX_VELICINA_RIJECI 50
#define MAX_BROJ_RIJECI 100

//za nas niz slova
#define MAXDULJINA 100

//dretve
#define MAXDRETVI 10

int w=0; //za rang listu samo

//STRUKTURA ZA RANG LISTU----------------------------------
struct Unos {
    char string[50];
    int broj;
};
//---------------------------------------------------------

//FUNKCIJA ZA ZAMJENU OVISNO O BROJU STECENIH BODOVA-------
void zamjena(struct Unos *a, struct Unos *b) {
    struct Unos temp = *a;
    *a = *b;
    *b = temp;
}
//---------------------------------------------------------

//FUNKCIJA ZA ZAMJENU SILAZNO OVISNO O BROJU STECENIH BODOVA
void sortirajRangListu(struct Unos rangLista[], int ind_rang){
    for (int i = 0; i < ind_rang - 1; i++) {
        for (int j = 0; j < ind_rang - i - 1; j++) {
            if (rangLista[j].broj < rangLista[j + 1].broj) {
                zamjena(&rangLista[j], &rangLista[j + 1]);
            }
        }
    }
}
//---------------------------------------------------------

//Broj unosa u rang listu----------------------------------
//int brojUnosa = 100;
//---------------------------------------------------------

//Inicijalizacija rang liste-------------------------------
struct Unos rangLista[MAX_BROJ_RIJECI];
//---------------------------------------------------------

//Indeks da znamo gdje smo u rang listi--------------------
int ind_rang=0;
//---------------------------------------------------------

//struktura za dretve
typedef struct{
    int commSocket;
    int indexDretve;
} obradiKlijenta__parametar;

//sve dretve na pocetku na nulu tako da svaka moze primit klijenta
int aktivneDretve[MAXDRETVI]={0};
obradiKlijenta__parametar parametarDretve[MAXDRETVI];
pthread_mutex_t lokot_aktivneDretve=PTHREAD_MUTEX_INITIALIZER;

void posaljiRANG(int sock){
    int j;
    char brojzaposlat[100];
    sprintf(brojzaposlat,"%d",ind_rang);
    posaljiPoruku(sock,ODGOVOR,brojzaposlat);
        for(j=0;j<ind_rang;j++){
            char mjestorang[100];
            sprintf(mjestorang,"%s %d",rangLista[j].string,rangLista[j].broj);
            posaljiPoruku(sock,ODGOVOR,mjestorang);
        }
    }

void *komunicirajSaKlijentom(void *parametar); //prva promjena umjesto int sock ide void*parametar
void obradiSTAVI(int sock, char *poruka, char *unesena_slova,char *sakrivena_rijec, int *brojpokusaja);
void obradiPOGODI(int sock, char *poruka, int *brojpokusaja, char *sakrivena_rijec);

/*void smanji_broj() {
    brojpokusaja=brojpokusaja-1;; // Smanji vrijednost globalne varijable
}*/

//funkcija za generiranje random rijeci...
char* nasumicnarijec(){
    FILE *datoteka;
    char rijeci[MAX_BROJ_RIJECI][MAX_VELICINA_RIJECI];
    int brojacRijeci=0;
    char rijec[MAX_VELICINA_RIJECI];

    //otvaram datoteku
    datoteka=fopen("datoteka.txt","r");

    if (datoteka == NULL) {
        printf("Greška pri otvaranju datoteke.");
        exit(1);
    }

    while(fscanf(datoteka,"%s",rijec)==1 && brojacRijeci < MAX_BROJ_RIJECI){
        strcpy(rijeci[brojacRijeci],rijec);
        brojacRijeci++;
    }

    //mozemo zatvoriti datoteku
    fclose(datoteka);

    //za rand funkciju
    srand(time(0));

    //odabiremo nasumicnu
    int randomInd = rand() % brojacRijeci;
    char *rijecZaIgru=rijeci[randomInd];
   // printf("Nasumicna rijec: %s\n", rijecZaIgru);
    return rijecZaIgru;
}

int gdje=0; //za pocetni samo unos korisnika

//ZA USERNAME---------------------------------------------------------------
char* nasumicanuser(){
    FILE *datoteka;
    char rijeci[MAX_BROJ_RIJECI][MAX_VELICINA_RIJECI];
    int brojacRijeci=0;
    char korisnik[MAX_VELICINA_RIJECI];

    //otvaram datoteku
    datoteka=fopen("usernames.txt","r");

    if (datoteka == NULL) {
        printf("Greška pri otvaranju datoteke.");
        exit(1);
    }

    while(fscanf(datoteka,"%s",korisnik)==1 && brojacRijeci < MAX_BROJ_RIJECI){
        strcpy(rijeci[brojacRijeci],korisnik);
        brojacRijeci++;
    }

    //mozemo zatvoriti datoteku
    fclose(datoteka);

    //za rand funkciju
    srand(time(0));

    //odabiremo nasumicnu
    int randomInd = rand() % brojacRijeci;
    char *rijecZaIgru=rijeci[randomInd];
   // printf("Nasumicna rijec: %s\n", rijecZaIgru);
    return rijecZaIgru;
}
//----------------------------------------------------------------------------


char *rijec;
char *korisnik;

int main(int argc, char **argv){

    if(argc!=2)
        error2("Upotreba: %s port\n", argv[0]);
    //iscitamo iz argv[1] port i upisemo u port
    int port;
    sscanf(argv[1],"%d", &port);

    //trebamo stvoriti uticnicu -> socket
    int listenerSocket = socket(PF_INET, SOCK_STREAM,0);
        if(listenerSocket==-1)
            myperror("socket");

    //bind i ona struktura
    struct sockaddr_in mojaAdresa;

    mojaAdresa.sin_family=AF_INET;
    mojaAdresa.sin_port=htons(port);
    mojaAdresa.sin_addr.s_addr=INADDR_ANY;
    memset(mojaAdresa.sin_zero,'\0',8);

    if(bind(listenerSocket,(struct sockaddr*)&mojaAdresa,sizeof(mojaAdresa))==-1)
        myperror("bind");

    //listen
    if(listen(listenerSocket,10)==-1)
        myperror("listen");

    //ovdje se stvari zbog dretvi pocinju mijenjati
    pthread_t dretve[10];
    rijec=nasumicnarijec();
    //sada ide accept
    while(1){
        //jer vjecno cekamo nove klijente...
        struct sockaddr_in klijentAdresa;
        int lenAddr=sizeof(klijentAdresa);

        int commSocket=accept(listenerSocket, (struct sockaddr*) &klijentAdresa,&lenAddr);

        if(commSocket==-1)
            myperror("accept");

        char *dekadskiIP=inet_ntoa(klijentAdresa.sin_addr);
        printf("Prihvatio konekciju od %s [socket=%d]\n", dekadskiIP, commSocket);

        //dio s dretvama, dodajemo tom nekom u paralelnom svemiru poso
        pthread_mutex_lock(&lokot_aktivneDretve);
        int k, indexNeaktivne=-1;
        for(k=0;k<MAXDRETVI;k++){
            if(aktivneDretve[k]==0)
                indexNeaktivne=k;
            else if(aktivneDretve[k]==2){
                pthread_join(dretve[k],NULL);
                aktivneDretve[k]=0;
                indexNeaktivne=k;
            }
        }
            if(indexNeaktivne==-1){
                close(commSocket); //nemam vise dretvi...
                printf("-->ipak odbijam konekviju jer nemam vise dretvi.\n");
            }
            else{
                aktivneDretve[indexNeaktivne]=1;
                parametarDretve[indexNeaktivne].commSocket=commSocket;
                parametarDretve[indexNeaktivne].indexDretve=indexNeaktivne;
                printf("-->koristim dretvu broj %d.\n",indexNeaktivne);

                pthread_create(&dretve[indexNeaktivne],NULL,komunicirajSaKlijentom,&parametarDretve[indexNeaktivne]);
            }
            pthread_mutex_unlock(&lokot_aktivneDretve);
        }

      // komunicirajSaKlijentom(commSocket);

      //  close(commSocket);
      //  printf("Zavrsio komunikaciju sa %s [socket=%d]\n", dekadskiIP, commSocket);

    return 0;
}

void *komunicirajSaKlijentom(void *parametar){

    obradiKlijenta__parametar *param=(obradiKlijenta__parametar *)parametar;
    int commSocket=param->commSocket;
    int indexDretve=param->indexDretve;

    int vrstaPoruke;
    int gotovo=0;
    char *poruka;
    char unesena_slova[MAXDULJINA];
    strcpy(unesena_slova,"");
    int treba=0;

    int brojpokusaja=7;

    //kako imamo nasu rijec moramo popuniti upitnicima tu rijec to jest
    //zamijeniti slova upitnicima u nekoj drugoj da znamo za kasnije
    char *sakrivena_rijec;
    int duljina_za_popuniti;
    duljina_za_popuniti=strlen(rijec);
    int i;
    sakrivena_rijec=(char*)malloc((duljina_za_popuniti+1)*sizeof(char));
    for(i=0;i<duljina_za_popuniti;i++)
        sakrivena_rijec[i]='?'; //sakrivena_rijec je popunjena upitnicima

        //generirati kodno ime...
        int l;
        int nema=0;
        while(1){
        korisnik=nasumicanuser();
            if(w==0){
       strcpy(rangLista[gdje].string, korisnik);
       w++;
       break;}
       for(l=0;l<gdje;l++){
        if(strcmp(rangLista[l].string,korisnik)==0){
            break;
            nema=1;
            }
       }
       if(nema==1)
        continue;
       else{
        strcpy(rangLista[gdje].string, korisnik);
        gdje++;
        break;
       }
    }

        //i onda ispod poslati...

    if(treba==0){
        char za_poslati[200];

        // sprintf(povratna,"Rijec ne sadrzi uneseno slovo, preostali broj pokusaja: %d | %s\n",brojpokusaja,sakrivena_rijec);
        sprintf(za_poslati,"Vase korisnicko ime: %s |Broj pokusaja: %d | Rijec: %s\n", korisnik,brojpokusaja,sakrivena_rijec);
        printf("SALJEM: %s",rijec);
        posaljiPoruku(commSocket,ODGOVOR,za_poslati); //prije je bio sock, prije dretvi
        treba++;
        }

    while(!gotovo){
        if(primiPoruku(commSocket,&vrstaPoruke,&poruka)!=OK){
            printf("Greska u komunikaciji sa klijentom [socket=%d]...\n",commSocket);
            gotovo=1;
            continue;
        }

        switch(vrstaPoruke){
            case STAVI: obradiSTAVI(commSocket,poruka,unesena_slova,sakrivena_rijec,&brojpokusaja); break;
            case POGODI: obradiPOGODI(commSocket,poruka,&brojpokusaja,sakrivena_rijec); break;
            case ODUSTANI: posaljiPoruku(commSocket, ODGOVOR, "OK"); gotovo=1; break; //ovdje ipak nest drugo
            default: posaljiPoruku(commSocket, ODGOVOR, "Nepostojeci kod poruke!\n");
        }
        free(poruka);
    }

    pthread_mutex_lock( &lokot_aktivneDretve );
	aktivneDretve[ indexDretve ] = 2;
	pthread_mutex_unlock( &lokot_aktivneDretve );

    return NULL;
}

void obradiSTAVI(int sock, char *poruka,char* unesena_slova,char* sakrivena_rijec, int *brojpokusaja){
    char primljen_znak;
    int vasibodovi;
    //u slucaju da je primljeno vise od jednog znaka
    printf(" 11 %s",poruka);
    size_t duljinakojutrebam=strlen(poruka);
    printf("%zu",duljinakojutrebam);
    if(duljinakojutrebam>1){
        posaljiPoruku(sock,ODGOVOR,"Molimo Vas da pazljivo unesete samo jedno slovo!");
        return; //obavezno!
    }

    primljen_znak=tolower(poruka[0]);
    printf("%c\n", primljen_znak);
    //pretvorimo u ascii zbog lakse provjere jesmo li primili znak izmedju a-z
    int ascii_vrijednost =(int) primljen_znak;
    printf("%d",ascii_vrijednost);
    if(ascii_vrijednost<97 || ascii_vrijednost>122 || ascii_vrijednost==119 ||ascii_vrijednost==120 || ascii_vrijednost==121 || ascii_vrijednost==113){
        posaljiPoruku(sock, ODGOVOR, "Unesite pazljivo slovo koje zelite isprobati (a-z)");
        return;
       }
    int i;
       //sve proslo imamo nas dobar znak
       //moramo ga spremiti u nas string, to jest listu
    //definirali prije accepta: char unesena_slova[MAXDULJINA];
    if(strlen(unesena_slova)==0){
        unesena_slova[0]=primljen_znak; //spremili prvi prvi znak koji smo primili u listu
        unesena_slova[1]='\0';
        printf("if sa strlen(unesena_slova)\n");
    }
    else{
        for(i=0;i<strlen(unesena_slova);i++){
            if(primljen_znak==unesena_slova[i]){//ako ima vec tog znaka u nizu
                printf("tog znaka ima vec u nizu\n");
                posaljiPoruku(sock, ODGOVOR, "Ovaj znak ste vec isprobali, unesite ponovo neki drugi!");
                return;
            }
        }
        //inace sve dobro proslo, dosli smo do kraja i dodajemo na kraj nas znak
        unesena_slova[i]=primljen_znak;
        unesena_slova[i+1]='\0'; //dodamo na kraj null znak radi iduceg poziva funkcije
    }

    //sad brojevi pokusaja, provjera je li to u rijeci i ispunjavanje onih ?
    int ima=0; //u slucaju da nema tog slova
    for(int i=0;i<strlen(rijec);i++){
        //tu cemo sad i odmah popuniti onu s upitnicima tj zamjeniti
        //ako pronadjemo odredjeno slovo u rijec[i]
        if(primljen_znak==rijec[i]){
            sakrivena_rijec[i]=primljen_znak;
            ima=1;
            printf("zamjenjujemo upitnike slovima\n");
            //zamjenili upitnike slovom
            if(strcmp(rijec,sakrivena_rijec)==0){
                //ako smo pogodili cijelu rijec sa slovima koje smo upisivali
                char povr[200];

                //ujedno trebamo vidjeti koliko je ostalo broja pokusaja i koliko je duga rijec da damo bodove
                ind_rang=gdje-1;
                vasibodovi=strlen(rijec)+(*brojpokusaja); //trebat ce za rang listu <---------------------------------------------------------------------
                rangLista[ind_rang].broj=vasibodovi;
                sortirajRangListu(rangLista, ind_rang);

                sprintf(povr,"Bravo!!! Pogodili ste rijec %s, preostao Vam je broj pokusaja: %d\n",sakrivena_rijec,*brojpokusaja);
                posaljiPoruku(sock,ODGOVOR,povr);
                return;
            }
        }
    }
    if(ima==0){
        //trebamo smanjiti broj pokusaja i posalti ?????
     //smanji_broj();
     printf("pocetni broj pokusaja %d\n",*brojpokusaja);
     (*brojpokusaja)--;
     printf("nakon smanjenja %d",*brojpokusaja);
     //brojpokusaja;
     printf("tu bi trebao biti smanjeno sad nakon ovog smanji_broj() funkcije\n");

     //slucaj kada smo pogrijesili slovo u zadnjem pokusaju i gotovo je
     if(*brojpokusaja==0){
        char gotovaigra[200];



        //ujedno gledamo koliko je preostalo upitnika u nepogodjenoj rijeci
        int pogslova=0;
        char upitnik='?';
        for(i=0;i<strlen(rijec);i++)
            //brojimo koliko je pogodjenih slova
            if(upitnik!=sakrivena_rijec[i])
                pogslova++;
                printf("--------------------------------------\n");
                printf("\n upisujem bodove %d\n", pogslova);
        ind_rang=gdje-1;
        vasibodovi=pogslova;
        printf("\n na taj rang %d", ind_rang);
        rangLista[ind_rang].broj=vasibodovi;

        printf("\n koliko je bodova na ranglista[ind_rang].broj = %d\n",rangLista[ind_rang].broj);
        sortirajRangListu(rangLista, ind_rang);
        //saljemo broj za velicinu strukture
        printf("gotov sa sortiranjem\n");

        sprintf(gotovaigra,"Preostali broj pokusaja: %d ! Zao nam je igra je gotova! Trazena rijec je bila %s",*brojpokusaja,rijec);
        posaljiPoruku(sock,ODGOVOR,gotovaigra);
        posaljiRANG(sock);
        return;
     }

     char povratna[200];
     sprintf(povratna,"Rijec ne sadrzi uneseno slovo, preostali broj pokusaja: %d | Rijec: %s\n",*brojpokusaja,sakrivena_rijec);
     posaljiPoruku(sock,ODGOVOR,povratna);
     return;
    }
    else{
        //upisali slova
        char povratna[200];
        printf("trebalo bi se sada vratiti da je pogodjeno slovo\n");
        sprintf(povratna,"Pogodili ste slovo, preostali broj pokusaja: %d | Rijec: %s\n",*brojpokusaja,sakrivena_rijec);
        posaljiPoruku(sock,ODGOVOR,povratna);
        return;
    }

}

void obradiPOGODI(int sock, char *poruka,int *brojpokusaja, char *sakrivena_rijec){
    int vasibodovi;
    char povratna[100];

    if(strcmp(rijec,poruka)==0){
            ind_rang=gdje;
            vasibodovi=(*brojpokusaja)+strlen(poruka);
            rangLista[ind_rang].broj=vasibodovi;
                sortirajRangListu(rangLista, ind_rang);
        posaljiPoruku(sock, ODGOVOR, "Bravo, pogodili ste rijec!");
        return;
    }
    (*brojpokusaja)--;
    int i;
    if(brojpokusaja==0){
     //ujedno gledamo koliko je preostalo upitnika u nepogodjenoj rijeci
       int pogslova=0;
        char upitnik='?';
        for(i=0;i<strlen(rijec);i++)
            //brojimo koliko je pogodjenih slova
            if(upitnik!=sakrivena_rijec[i])
                pogslova;
        ind_rang=gdje;
        vasibodovi=pogslova;
        rangLista[ind_rang].broj=vasibodovi;
                sortirajRangListu(rangLista, ind_rang);


        sprintf(povratna,"Niste pogodili rijec! Preostali broj pokusaja: %d\n",*brojpokusaja);
        return;
    }


    posaljiPoruku(sock,ODGOVOR,povratna);
    return;
}

