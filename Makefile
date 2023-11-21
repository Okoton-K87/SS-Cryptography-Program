SOURCES  = $(wildcard *.c)
OBJECTS  = numtheory.o ss.o randstate.o

CC       = clang
CFLAGS   = -Wall -Wpedantic -Werror -Wextra -gdwarf-4
LIBFLAGS = `pkg-config --libs gmp`

.PHONY: all clean format 

all: keygen encrypt decrypt

keygen: $(OBJECTS) keygen.o
	$(CC) -o $@ $^ $(LIBFLAGS)

encrypt: $(OBJECTS) encrypt.o
	$(CC) -o $@ $^ $(LIBFLAGS)

decrypt: $(OBJECTS) decrypt.o
	$(CC) -o $@ $^ $(LIBFLAGS)

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJECTS) keygen encrypt decrypt $(SOURCES:%.c=%.o)

format:
	clang-format -i -style=file *.[ch]
