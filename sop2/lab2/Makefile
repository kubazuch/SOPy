all: server client
server: kuchnia.c
	gcc -Wall -fsanitize=address,undefined -o kuchnia kuchnia.c -lrt
client: klient.c
	gcc -Wall -fsanitize=address,undefined -o klient klient.c -lrt

.PHONY: clean all
clean:
	rm $(LAB)
