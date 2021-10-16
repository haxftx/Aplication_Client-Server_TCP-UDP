#ifndef CLIENT_H
#define CLIENT_H
#include "list.h"

char *mesage_to_str(char *buffer, char *ip, int portno) {
	// transforma mesajul udp intr-un string
	char *str = calloc(BUFLEN + 100, sizeof(char));// rezultatul
	if (!str) {// eroare alocare
		write(2, "Nu s-a alocat memorie\n", 22);
		return NULL;
	}
	char *paylod = calloc(BUFLEN, sizeof(char));// continut
	if (!paylod) {// eroare alocare
		write(2, "Nu s-a alocat memorie\n", 22);
		free(str);
		return NULL;
	}
	char topic[51];
	memset(topic, 0, 51);
	if (buffer[49] != '0') {
		strncpy(topic, buffer, strlen(buffer));
	} else strncpy(topic, buffer, 50);
	char tip = buffer[50];// tipul de date
	char semn = buffer[51];// octetul de semn
	if (tip == 0) {// INT
		int number;// numarul
		memcpy(&number, buffer + 52, sizeof(int));
		number = ntohl(number);
		if (semn == 1)
			number *= -1;
		sprintf(paylod, "INT - %d", number);// creez continutul
	} else if (tip == 1) {// SHORT_REAL
		uint16_t number;// numarul
		memcpy(&number, buffer + 51, sizeof(uint16_t));
		number = ntohs(number);// creez continutul
		sprintf(paylod, "SHORT_REAL - %f", number * 1.0 / 100);
	} else if (tip == 2) {// FLOAT
		int number;// numarul
		memcpy(&number, buffer + 52, sizeof(int));
		number = ntohl(number);
		uint8_t mod;// puterea lui 10
		memcpy(&mod, buffer + 56, sizeof(uint8_t));
		if (semn == 1)
			number *= -1;// creez continutul
		sprintf(paylod, "FLOAT - %f", number * 1.0 / pow(10, mod));
	} else if (tip == 3) {// STRING
		memcpy(paylod, "STRING - ", 9);
		memcpy(paylod + 9, buffer + 51, 1500);// creez continutul
		paylod[1509] = '\0';
	}// creez rezultatul
	sprintf(str, "%s:%d - %s - %s\n", ip, portno, topic, paylod);
	free(paylod);
	return str;
}

void send_msg_SF(Tclient_tcp client) {
	// trimite mesajele salvate ale unui client
	list aboanari;
	aboanari = client->abonari;
	queue q;
	char *msg;
	int n;
	while (aboanari != NULL) {
		q = ((Tclient_udp)aboanari->element)->mesage;
		while (!queue_empty(q)) {// pentru toate mesajele
			msg = (char *)queue_deq(q);
			n = send(client->sockfd, msg, strlen(msg), 0);
			DIE(n < 0, "send");
			free(msg);
		}
		aboanari = aboanari->next;
	}
	
}

char *dezactivate_client(list clienti, int sockfd) {
	// intoarce id-l unui client activ si-l face inactiv
	Tclient_tcp element;
	while (clienti != NULL) {
		element = (Tclient_tcp)clienti->element;
		if (element->sockfd == sockfd) {
			element->activ = 0;
			return element->id;
		}
		clienti = clienti->next;
	}
	return NULL;
}

int add_client(list *clienti, int sockfd, char *id) {
	// adauga un client in baza de date
	Tclient_tcp element = aloc_client_tcp(id, sockfd);
	if (!element)// eroare
		return 0;
	*clienti = cons(element, *clienti);
	return 1;
}

int search_client(list clienti, int sockfd, char *id) {
	// actualizeaza datele sau aflu existenta, inexistenta unui client
	Tclient_tcp info;
	while (clienti != NULL) {
		info = (Tclient_tcp) clienti->element;
		if (!strcmp(info->id, id)) {
			if(info->activ) {// client activ
				return 1;
			} else {// client inactiv
				info->activ = 1;
				info->sockfd = sockfd;
				send_msg_SF(info);//trimit mesajele salvate
				return 2;
			}
		}
		clienti = clienti->next;
	}
	return 0;
}

void subscribe_unsubribe(list clienti, char *buffer, int sockfd) {
	Tclient_tcp element;
	while (clienti != NULL) {// caut clientul
		element = (Tclient_tcp)clienti->element;
		if (element->sockfd == sockfd)
			break;
		clienti = clienti->next;
	}
	if (clienti == NULL)// nu exista clientul
		return;

	char *token = strtok(buffer, " ");// tipul funtiei
	int f = -1;
	if (!strcmp(token, "subscribe")) {
		token = strtok(NULL, " ");
		f = 1;
	} else if (!strcmp(token, "unsubscribe")) {
		token = strtok(NULL, "\n");
		f = 0;
	}
	char *SF = strtok(NULL, " ");// flag pentru S&F
	list abonari = element->abonari, ant = NULL;
	Tclient_udp info;
	while (abonari != NULL) {// caut abonarea
		info = (Tclient_udp)abonari->element;
		if (!strcmp(info->topic, token))
			break;
		ant = abonari;
		abonari = abonari->next;
	}
	if (f == 1) {// subscribe
		if (abonari) {//daca l-am gasit actualizez S&F
			info->SF = atoi(SF);
			return;
		}// adaug o abonare
		info = aloc_client_udp(token, atoi(SF));
		if (!info)// eroare
			return;
		element->abonari = cons(info, element->abonari);
	} else if (f == 0) {// unsubscribe
		if (abonari == NULL) {//nici nu era abonat
			return;
		}
		if (ant == NULL) {// primul element
            ant = element->abonari;
			element->abonari = abonari->next;
            ant->next = NULL;
			free_abonari(ant);
		} else {// inafara de primul
			ant->next = abonari->next;
			abonari->next = NULL;
			free_abonari(abonari);
		}
	}
}

void add_or_send_msg(Tclient_tcp client, char *topic, char *msg) {
	// adauga clientilor sau le trimite un mesaj la ce sunt abonati
	list l = client->abonari;
	Tclient_udp element;
	while (l != NULL) {// pentru fiecare abonare
		element = (Tclient_udp)l->element;
		if (!strcmp(topic, element->topic)) {// daca este abonat
			if (client->activ) {// daca e activ trimit mesajul
				int n = send(client->sockfd, msg, strlen(msg), 0);
				DIE(n < 0, "send");
			} else if (!client->activ && element->SF) {// daca e inactiv salvez
				char *str = calloc(strlen(msg) + 1, sizeof(char));
				if (!str) {// eroare
					write(2, "Eroare alocare memorie\n", 24);
					return;
				}
				memset(str, 0, strlen(msg) + 1);
				memcpy(str, msg, strlen(msg));
				queue_enq(element->mesage, str);// salvez mesajul
			}
			break;
		}
		l = l->next;
	}
}

#endif