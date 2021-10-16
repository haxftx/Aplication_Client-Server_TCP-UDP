#include "list.h"

list cons(void *element, list l) {
	list temp = malloc(sizeof(struct cell));
	temp->element = element;
	temp->next = l;
	return temp;
}

list cdr_and_free(list l) {
	list temp = l->next; 
	free(l);
	return temp;
}

Tclient_udp aloc_client_udp(char *topic, int SF) {
	// aloca o structura Tclient_tcp
	Tclient_udp element = calloc(1, sizeof(struct client_udp));
	if (!element) {// eroare
		write(2, "Eroare alocare memorie\n", 24);
		return NULL;
	}
	strcpy(element->topic, topic);
	element->SF = SF;
	element->mesage = queue_create();
	return element;
}

void free_abonari(list abonari) {
	// elibereaza o lista cu elemente de tip Tclient_udp
	list p;
	queue q;
	char *msg;
	while (abonari != NULL) {
		p = abonari;
		
		q = ((Tclient_udp)abonari->element)->mesage;
		while (!queue_empty(q)) { //elibereaza coada
			msg = queue_deq(q);
			free(msg);
		}
		free(q);
		abonari = abonari->next;
		free(p->element);
		free(p);
	}
}

Tclient_tcp aloc_client_tcp(char *id, int sockfd) {
	// aloca o structura Tclient_tcp
	Tclient_tcp element = calloc(1, sizeof(struct client_tcp));
	if (!element) {// eroare
		write(2, "Eroare alocare memorie\n", 24);
		return NULL;
	}
	strcpy(element->id, id);
	element->sockfd = sockfd;
	element->activ = 1;
	element->abonari = NULL;
	return element;
}

void free_clienti(list clienti) {
	// elibereaza o lista cu elemente de tip Tclient_tcp
	list p, l;
	while (clienti != NULL) {
		p = clienti;
		clienti = clienti->next;
		l = ((Tclient_tcp)p->element)->abonari;
		free_abonari(l);
		free(p->element);
		free(p);
	}
	
}
