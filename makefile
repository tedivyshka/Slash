CC=gcc
CFLAGS= -Wall -pedantic -g
LIBS= -lreadline

DEBUG = -fsanitize=address

prompt :
	$(CC) $(CFLAGS) -o slash slash.c $(LIBS)
promptDebug :
	$(CC) $(CFLAGS) $(DEBUG) -o slash slash.c $(LIBS)
clean :
	rm slash

