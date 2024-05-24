CC = gcc
CFLAGS = -Wall -Wextra -pedantic
OBJ1 = functions.o
OBJ2 = basic_functions.o

all: mysh

functions.o: functions.c functions.h
	$(CC) $(CFLAGS) -c functions.c

basic_functions.o: basic_functions.c basic_functions.h
	$(CC) $(CFLAGS) -c basic_functions.c

main.o: main.c functions.h basic_functions.h
	$(CC) $(CFLAGS) -c main.c

mysh: main.o $(OBJ1) $(OBJ2)
	$(CC) $(CFLAGS) -o mysh main.o $(OBJ1) $(OBJ2)

clean:
	rm -f mysh *.o


