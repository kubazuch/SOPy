FILE=main
CFLAGS=-std=gnu99 -Wall -fsanitize=address,undefined
LDFLAGS=-fsanitize=address,undefined
LDLIBS=-lpthread -lm

all: ${FILE}
${FILE}: ${FILE}.c
	@gcc ${CFLAGS} ${LDFLAGS} ${LDLIBS} -o ${FILE} ${FILE}.c

.PHONY = clean all
clean:
	rm main