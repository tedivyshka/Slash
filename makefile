CC=gcc
CFLAGS= -Wall -pedantic -g
LIBS= -lreadline

prompt :
	$(CC) $(CFLAGS) -o slash slash.c $(LIBS)

clean :
	rm slash

