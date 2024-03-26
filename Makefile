CC = clang
CFLAGS = -Wall -Werror -Wextra -Wpedantic -gdwarf-4

all: encode decode

encode: encode.o io.o trie.o word.o
	$(CC) -o $@ $^

decode: decode.o io.o trie.o word.o
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f encode decode *.o

format:
	clang-format -i -style=file *.[ch]
