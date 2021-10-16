Aplicatia consta in 2 parti server si subscriber.

Subscriber-ul stie doar a se conecta la server si sa poata citi de la tastatura
3 comenzi(subscribe, unsubscribe, exit), pe care le verifica daca sunt sau nu
valide si le trimite catre server, pentru a-i fi mai usor clientului el va
primi de la server direct sirul cu caractere care este un mesaj de la
topicurile care este abonat, si il va afisa direct.
Creeaza o conexiune catre server, trimite id-l cu care se va identifica si apoi
asculta socketul si stdinul, pentru stdin verifica valabilitatea comenzei
introduse si o trimite sau nu la server, la mesajele venite pe socket doar le
afiseaza deoarece deja sunt prelucrate.

Server-ul este un broken pentru clientii tcp si udp, primeste abonari de la
clientii tcp si de la clientii udp primeste mesaje adresate clientilor tcp 
abonati.
Asculta stdin si toti clientii conectati tcp si udp, daca primeste ceva de la
tastatura reactioneaza doar la comanda exit, iar la mesajele udp le creeaza in
humman-readable, apoi verifica in baza sa de date toti clientii ce sunt abonati
la acest topic si daca sunt activi le trimite mesajul, iar daca nu este activ,
insa este abonat S&F(nu vrea sa piarda nici un mesaj) salvez mesajele intr-o
coada de mesaje. Pentru mesajele primite de la clientii tcp, cand e o noua
conexiune verifica daca nu exista deja acest id (trimite un mesaj ca exista id)
daca clientul este inactiv, il activeaza si-i trimite toate mesajele care s-au
adunat in coada S&F(mesaje salvate), iar daca nu exista il adauga in baza de 
clienti, iar daca trimite un mesaj(de subscribe, unsubscribe) il trateaza 
caatare.

Pentru a salva clientii am folost o lista care ca informatie are structura
Tclient_tcp, unde in ea la randul ei are o lista de abonari care e structura
Tclient_udp, unde in ea la randul ei este coada cu mesajele salvate.

helpers.h - contrine incluziunile si DIE, cu care tratez erorile de la apelul
            funtiilor de sistem.

list.h - contine structurile folosite pentru liste si semnatura funtiilor din
            list.c

list.c - contine funtiile de prelucrare a listelor:
        cons() - creeaza o lista;
        cdr_and_free() - elibereaza capul listei;
        aloc_client_udp() - aloca un element de tip Tclient_udp cu parametrii
                            de topic si flagul pentru S&F
        free_abonari() - elibereaza o lista ce contine elemente Tclient_udp
        aloc_client_tcp() - aloca un element de tip Tclient_tcp cu parametrii
                            de id client si socket file descriptor
        free_clienti() - elibereaza o lista ce contine elemente Tclient_tcp
client.h - contine funtii de prelucare a mesajelor de la clienti
        mesage_to_str() - creeaza un string in format humman-readable din
                        mesajul primit de la clientul udp, dupa formatul din
                        enunt pentru cele 4 tipuri de date ale mesajului INT,
                        SHORT_REAL, FLOAT si STRING
        send_msg_SF() - trimite unui client tcp, toate mesajele pierdute in
                        timpul de deconectare, de la toate topicurile abonat
        dezactivate_client() - dezactiveaza un client din baza de date obtinand
                                id-l clientului
        add_client() - adauga un client tcp in baza de date a server-lui
        search_client() - verifica daca exista sau nu un client si daca e activ
                          ignor, daca e inactiv il activez
        subscribe_unsubribe() - aboneaza sau dezaboneaza un client de la un
                                topic in dependenta de comanda primita de la
                                clientul tcp
        add_or_send_msg() - ia mesajul primit de la clientul udp si il salveaza
                            sau trimite catre toti clientii abonati la acest
                            topic
subscriber.c - contine rularea subscriber-lui
            usage() - afiseaza mesajul de rulare corespunzator in caz de eroare
            nr_space() - calculeaza numarul de spatii al unui string.
            main() - rularea proprie zisa unde se conecteaza si comunica prin
                     socketul conectat cu server-ul
server.c - contine rularea server-ului
         usage() - afiseaza mesajul de rulare corespunzator in caz de eroare
         main() - rularea proprie zisa unde se conecteaza si comunica prin
                  toti socketii cu care sa conectat, cu stdin, clientii udp
                  si cu clientii tcp

Pentru rulare in Makefile trebuie de indicat IP-l server-ului si portul, apoi
comanda: make run_server - porneste server-ul
comanda: make run_subscriber ID=<ID_Client> - porneste subscriber-ul