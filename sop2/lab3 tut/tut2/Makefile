\all: server client
server: prog24s.c
	gcc -Wall -fsanitize=address,undefined -o prog24s prog24s.c -lrt
client: prog24c.c
	gcc -Wall -fsanitize=address,undefined -o prog24c prog24c.c -lrt

.PHONY: clean all
clean:
	rm $(LAB)
