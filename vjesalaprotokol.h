#ifndef __VJESALAPROTOCOL_H_
#define __VJESALAPROTOCOL_H_

#define STAVI 1 //za staviti slovo
#define POGODI 2 //za pogoditi rijec
#define ODUSTANI 3 //u slucaju da zelimo odustati od igre
#define ODGOVOR 4

//ovo ispod koriste i klijent i server
#define OK 1
#define NIJEOK 0

int primiPoruku(int sock, int *vrstaPoruke, char **poruka);
int posaljiPoruku(int sock, int vrstaPoruke, const char *poruka);

#define error1( s ) { printf( s ); exit( 0 ); }
#define error2( s1, s2 ) { printf( s1, s2 ); exit( 0 ); }
#define myperror( s ){ perror( s ); exit( 0 ); }

#endif // __VJESALAPROTOCOL_H_
