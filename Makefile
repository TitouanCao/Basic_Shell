CFLAGS= -g -Wall -Werror
CC=gcc

all: shell

shell: shell.o path.o parser.o command.o command_management.o utils.o
	$(CC) $(CFLAGS) $^ -o $@

shell.o: shell.c path.h
	$(CC) $(CFLAGS) -o shell.o -c $<
path.o: path.c path.h
	$(CC) $(CFLAGS) -o path.o -c $<
parser.o: parser.c parser.h
	$(CC) $(CFLAGS) -o parser.o -c $<
command.o: command.c command.h
	$(CC) $(CFLAGS) -o command.o -c $<
command_management.o: command_management.c command_management.h
	$(CC) $(CFLAGS) -o command_management.o -c $<
utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -o utils.o -c $<

test:
	gdb shell

clean:
	rm -f *.o
