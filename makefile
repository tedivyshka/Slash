CC=gcc
CFLAGS= -Wall -pedantic -g
LIBS= -lreadline

slash : commands.o signal.o lineTreatment.o slash.o utilities.o
	$(CC) $(CFLAGS) utilities.o commands.o signal.o lineTreatment.o slash.o -o slash $(LIBS)

commands.o : commands.c commands.h
	$(CC) $(CFLAGS) -c commands.c -o commands.o

lineTreatment.o : lineTreatment.c lineTreatment.h
	$(CC) $(CFLAGS) -c lineTreatment.c -o lineTreatment.o

slash.o : slash.c slash.h
	$(CC) $(CFLAGS) -c slash.c -o slash.o

utilities.o : utilities.c utilities.h
	$(CC) $(CFLAGS) -c utilities.c -o utilities.o

signal.o : signal.c signal.h
	$(CC) $(CFLAGS) -c signal.c -o signal.o

clean :
	rm *.o
	rm slash


