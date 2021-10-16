CFLAGS = -Wall -g
# Portul pe care asculta serverul
PORT = 8888
# Adresa IP a serverului
IP_SERVER = 127.0.0.1

all: server subscriber

# Compileaza server.c
server: server.c list.c queue.c -lm

# Compileaza client.c
subscriber: subscriber.c

.PHONY: clean run_server run_subscriber

# Ruleaza serverul
run_server: server
	./server ${PORT}

# Ruleaza clientul
run_subscriber: subscriber
	./subscriber ${ID} ${IP_SERVER} ${PORT}

clean:
	rm -f server subscriber
