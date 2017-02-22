
CFLAGS=-Wall
#CFLAGS=-g -Wall -Wextra
all: intermediario

intermediario.o: comun.h
comun.o: comun.h

intermediario: intermediario.o comun.o 
	$(CC) -o $@ intermediario.o comun.o -lpthread

clean:
	rm -f *.o intermediario
