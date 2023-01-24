LAB=lab4
all: $(LAB)
$(LAB): $(LAB).c
	gcc -Wall -fsanitize=address,undefined -o $(LAB) $(LAB).c -lrt -lm
.PHONY: clean all
clean:
	rm $(LAB)
