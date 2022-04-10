CC=gcc
CFLAGS= -I

make: ./src/*.c
	$(CC) -lreadline -g -o flush ./src/*.c 
	./flush
clean:
	-rm -f *.o

purge:
	-rm -f ./flush
