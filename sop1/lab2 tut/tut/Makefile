LAB=prog16
all: $(LAB)
$(LAB): $(LAB).c
	gcc -Wall -fsanitize=address,undefined -o $(LAB) $(LAB).c
.PHONY: clean all
clean:
	rm $(LAB)
