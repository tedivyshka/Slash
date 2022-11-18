CC=gcc
CFLAGS= -Wall -pedantic -g
LIBS= -lreadline

prompt :
	$(CC) $(CFLAGS) $(LIBS) -o slash slash.c

clean :
	rm slash