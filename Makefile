CC = gcc

OBJS = wordsearch.o btree.o

CFLAGS = -w -wall

COMPILE = $(CC) -c

BIN = wordsearch

$(BIN) : $(OBJS)
	$(CC) -lm -o $@ $(OBJS)

wordsearch.o : wordsearch.c wordsearch.h btree.h structure.h
	$(COMPILE) wordsearch.c

btree.o : btree.c btree.h structure.h
	$(COMPILE) btree.c

all:
	gcc -o wordsearch wordsearch.c btree.c -lnsl

clean:
	rm -rf ${BIN} *.o
