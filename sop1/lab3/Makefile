LAB=lab3
all: $(LAB)
$(LAB): $(LAB).c
	gcc -Wall -fsanitize=address,undefined -o $(LAB) $(LAB).c -lpthread -lm
.PHONY: clean all
clean:
	rm $(LAB)
