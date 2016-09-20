#
# Makefile
#
# Operating Systems, Exercise 4
#

CC = gcc
#CFLAGS = -Wall -g
CFLAGS = -Wall -O2 -g

all: scheduler-shell shell prog 

scheduler-shell: scheduler-shell.o proc-common.o
	$(CC) -o scheduler-shell scheduler-shell.o proc-common.o

shell: shell.o proc-common.o
	$(CC) -o shell shell.o proc-common.o

prog: prog.o proc-common.o
	$(CC) -o prog prog.o proc-common.o

proc-common.o: proc-common.c proc-common.h
	$(CC) $(CFLAGS) -o proc-common.o -c proc-common.c

shell.o: shell.c proc-common.h request.h
	$(CC) $(CFLAGS) -o shell.o -c shell.c

scheduler-shell.o: scheduler-shell.c proc-common.h request.h
	$(CC) $(CFLAGS) -o scheduler-shell.o -c scheduler-shell.c

prog.o: prog.c
	$(CC) $(CFLAGS) -o prog.o -c prog.c

clean:
	rm -f scheduler-shell shell prog *.o
