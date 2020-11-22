CFLAGS= -g -Wall -Werror
CC=gcc

all: shell

shell: shell.o environment.o parser.o command.o command_management.o utils.o
	$(CC) $(CFLAGS) $^ -o $@

%.o:%.c
	$(CC) $(CFLAGS) -c $<

shell.o: environment.h
environment.o: environment.h
parser.o: parser.h
command.o: command.h
command_management.o: command_management.h
utils.o: utils.h

test:
	gdb shell

clean:
	rm -f *.o shell
