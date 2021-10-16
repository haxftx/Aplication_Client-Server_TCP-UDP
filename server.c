#include "client.h"

void usage(char *file) {
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}

void prelucrate_msg_udp(list clienti, char *msg) {
	//timit mesajul la toti si le salvezi la cei inactivi;
	char buffer[strlen(msg)];
	memcpy(buffer, msg, strlen(msg));
	char *token = strtok(buffer, " ");
	token = strtok(NULL, " ");
	token = strtok(NULL, " ");//topicul
	Tclient_tcp element;
	while (clienti != NULL) {// pentru fiecare client
		element = (Tclient_tcp)clienti->element;
		add_or_send_msg(element, token, msg);
		clienti = clienti->next;
	}
}

int main(int argc, char **argv) {
	if (argc < 2) {
		usage(argv[0]);
	}

	int sockfd_udp, sockfd_tcp, newsockfd, portno;// socketi si port
	char buffer[BUFLEN];
	struct sockaddr_in serv_addr, cli_addr;
	int n, i, ret;
	socklen_t clilen;
	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar
	int fdmax;			// valoare maxima fd din multimea read_fds
	list clienti = NULL; // lista de clienti din baza de date
	char *msg, *id;

	// se goleste multimea de descriptori de citire si cea temporara
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);
	//deschid un scoket tcp
	sockfd_tcp = socket(PF_INET, SOCK_STREAM, 0);
	DIE(sockfd_tcp < 0, "socket");
	int flag = 1;// dezactivez Neagle
	setsockopt(sockfd_tcp, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

	portno = atoi(argv[1]);// portul
	DIE(portno == 0, "atoi");
	//setez datele adresei
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	//atasez adresa socketului
	ret = bind(sockfd_tcp, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "bind");
	ret = listen(sockfd_tcp, MAX_CLIENTS);
	DIE(ret < 0, "listen");
	
	//deschid un socket udp
	sockfd_udp = socket(PF_INET, SOCK_DGRAM, 0);
	DIE(sockfd_udp < 0, "socket");// dezactivez Neagle
	setsockopt(sockfd_udp, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
	//atasez adresa socketului
	ret = bind(sockfd_udp, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "bind");

	// se adauga stdint, socket_tcp, socket_udp descriptor in multimea read_fds
	FD_SET(0, &read_fds);
	FD_SET(sockfd_tcp, &read_fds);
	FD_SET(sockfd_udp, &read_fds);
	fdmax = sockfd_udp;
	while (1) {
		tmp_fds = read_fds; 
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");
		memset(buffer, 0, BUFLEN);	
		if (FD_ISSET(0, &tmp_fds)) {// citeste de la tastatura
			fgets(buffer, BUFLEN - 1, stdin);
			if (!strncmp(buffer, "exit", 4)) {
				break;
			} else {
				write(1, "De la stdin poate primi doar exit!\n", 35);
				continue;
			}
		}
		for (i = 3; i <= fdmax; i++) {// pentru fiecare fd
			if (FD_ISSET(i, &tmp_fds)) {// daca este setat
				if (i == sockfd_udp) {// un mesaj udp
					clilen = sizeof(cli_addr);
					n = recvfrom(i, buffer, sizeof(buffer), 0,
									(struct sockaddr *) &cli_addr, &clilen);
					DIE(n < 0, "recvfrom");
					msg = mesage_to_str(buffer, inet_ntoa(cli_addr.sin_addr),
													ntohs(cli_addr.sin_port));
					prelucrate_msg_udp(clienti, msg);// trimit sau salvez
					free(msg);
					break;
				}
				if (i == sockfd_tcp) { // o noua conexiune tcp
					setsockopt(i, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
					clilen = sizeof(cli_addr);
					newsockfd = accept(sockfd_tcp, (struct sockaddr *)
														&cli_addr, &clilen);
					DIE(newsockfd < 0, "accept");
					n = recv(newsockfd, buffer, BUFLEN, 0);
					DIE(n < 0, "recv");
					n = search_client(clienti, newsockfd, buffer);
					if (n == 1) {//daca este activ
						send(newsockfd, "Exista deja un client cu acest id\n", 35, 0);
						break;
					}
					if (n == 0 && !add_client(&clienti, newsockfd, buffer))
						break;//daca nu exista il adaug in lista
					// se adauga noul socket la multimea descriptorilor de citire
					FD_SET(newsockfd, &read_fds);
					if (newsockfd > fdmax) { 
						fdmax = newsockfd;
					}
					// feedback connect
					printf("New client %s connected from %s:%d.\n", buffer,
							inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
					break;
				
				} else {// mesaj de la client
					n = recv(i, buffer, BUFLEN, 0);
					DIE(n < 0, "recv");
					if (n == 0) {// a inchis conexiunea
						//caut socketul si fac inactiv clientul
						id = dezactivate_client(clienti, i);
						if (id) {// feedback disconect
							printf("Client %s disconnected.\n", id);
						}// se scoate din multimea de citire socketul inchis
						close(i);
						FD_CLR(i, &read_fds);
					} else {// am primit un mesaj de la un client
						subscribe_unsubribe(clienti, buffer, i);
					}
					break;
				}
			}
		}
	}
	free_clienti(clienti);// sterg lista de clienti
	close(sockfd_udp);
	close(sockfd_tcp);


	return 0;
}
