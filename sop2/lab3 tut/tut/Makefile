all: server client client2
server: prog23b_s.c
	gcc -Wall -fsanitize=address,undefined -o prog23b_s prog23b_s.c -lrt
client: prog23_local.c
	gcc -Wall -fsanitize=address,undefined -o prog23_local prog23_local.c -lrt
client2: prog23_tcp.c
	gcc -Wall -fsanitize=address,undefined -o prog23_tcp prog23_tcp.c -lrt

.PHONY: clean all
clean:
	rm $(LAB)
