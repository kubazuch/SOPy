all: server client
server: server.c
	gcc -Wall -fsanitize=address,undefined -o server server.c -lrt
client: client.c
	gcc -Wall -fsanitize=address,undefined -o client client.c -lrt

.PHONY: clean all
clean:
	rm $(LAB)
