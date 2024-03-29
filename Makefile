# Protocoale de comunicatii:
# Laborator 8: Multiplexare
# Makefile

CFLAGS = -Wall -g

ID_CLIENT = 

# Portul pe care asculta serverul (de completat)
PORT = 

# Adresa IP a serverului (de completat)
IP_SERVER = 

all: server subscriber

# Compileaza server.c
server: server.c

# Compileaza client.c
client: subscriber.c

.PHONY: clean run_server run_subscriber

# Ruleaza serverul
run_server:
	./server ${PORT}

# Ruleaza clientul
run_client:
	./subscriber ${ID_CLIENT} ${IP_SERVER} ${PORT}

clean:
	rm -f server subscriber
