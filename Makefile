CC=gcc
CFLAGS= -I

make: ./src/*.c
	$(CC) -pthread -g -o flush ./src/*.c 

clean:
	-rm -f *.o

purge:
	-rm -f ./flush
