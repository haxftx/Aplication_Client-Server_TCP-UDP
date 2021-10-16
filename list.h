#ifndef LIST_H
#define LIST_H
#include "helpers.h"

typedef struct cell {
  void *element;
  struct cell *next;
}*list;

typedef struct client_tcp {
	char id[11];
	int sockfd, activ;
	list abonari;
}*Tclient_tcp;// date client tcp

typedef struct client_udp {
	char topic[51];
	char SF;
	queue mesage;
}*Tclient_udp;// date client udp

extern list cons(void *element, list l);
extern list cdr_and_free(list l);
extern Tclient_tcp aloc_client_tcp(char *id, int sockfd);
extern Tclient_udp aloc_client_udp(char *topic, int SF);
extern void free_abonari(list abonari);
extern void free_clienti(list clienti);

#endif
