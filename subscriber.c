#include "helpers.h"

void usage(char *file) {
	fprintf(stderr, "Usage: %s <ID_Client> <IP_Server> <Port_Server>\n", file);
	exit(0);
}

int nr_space(char *str, int *first_space) {
	//intoarce numarul de spatii si indexul primului
	int count = 0;
	*first_space = 0;
	for (int i = 0; i < strlen(str); i++) {
		if (str[i] == ' ') {
			if (*first_space == 0)
				*first_space = i;
			count++;
		}
	}
	return count;
}

int main(int argc, char **argv) {
	if (argc < 4) {
		usage(argv[0]);
	}
	int sockfd, n, ret;
	struct sockaddr_in serv_addr;// adresa serverului
	char buffer[BUFLEN];
	fd_set read_fds;// multimea de citire folosita in select()
	fd_set tmp_fds;// multime folosita temporar
	int fdmax, ofsset;// maximul fd din multimea read_fds, ofsset la comanda

	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);// creez un scket
	DIE(sockfd < 0, "socket");
	int flag = 1;// dezactivez Neagle
	setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (void *)&flag, sizeof(flag));
	// setez datele adresei
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[3]));
	ret = inet_aton(argv[2], &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton");
	// creez conexiunea
	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "connect");
	ret = send(sockfd, argv[1], 10, 0);// trimit id-l
	DIE(ret < 0, "send");
	
	FD_SET(0, &read_fds);
	FD_SET(sockfd, &read_fds);
	fdmax = sockfd;
	while (1) {
		tmp_fds = read_fds; 
		memset(buffer, 0, BUFLEN);
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");
		if (FD_ISSET(0, &tmp_fds)) {// se citeste de la tastatura	
			fgets(buffer, BUFLEN - 1, stdin);
			if (!strncmp(buffer, "exit", 4)) {// exit
				break;
			}
			n = nr_space(buffer, &ret);// pentru verificare comenzie
			ofsset = ret;
			if (!strncmp(buffer, "subscribe", ret)) {// subscribe
				if (n != 2 || (buffer[strlen(buffer) - 2] != '0' &&
										buffer[strlen(buffer) - 2] != '1')) {
					write(1, "Comanda gresita subscribe\n", 26);
					continue;
				}// trimit comanda catre server
				n = send(sockfd, buffer, strlen(buffer), 0);
				DIE(n < 0, "send");
				// feedback subscribe
				write(1, "subscribed", 10);
				n = nr_space(buffer + ret + 1, &ret);
				write(1, buffer + ofsset, ret + 1);
				write(1, " \n", 3);
			} else if (!strncmp(buffer, "unsubscribe", ret)) {// unsubscribe
				if (n != 1) {
					write(1, "Comanda gresita unsubscribe\n", 28);
					continue;
				}// trimit comanda catre server
				n = send(sockfd, buffer, strlen(buffer), 0);
				DIE(n < 0, "send");
				// feedback unsubscribe
				write(1, "unsubscribed", 12);
				write(1, buffer + ofsset, strlen(buffer) - ofsset);
			} else {// comanda gresita
				write(1, "Comanda gresita\n", 17);
			}
		} else if(FD_ISSET(sockfd, &tmp_fds)) {// mesaj de la server
			n = recv(sockfd, buffer, BUFLEN, 0);
			DIE(n < 0, "recv");
			write(1, buffer, n);
		}
	}
	close(sockfd);// inchid conexiunea
	return 0;
}
