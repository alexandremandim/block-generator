CFLAGS = -O
CC = g++

all: Teste

Teste: main.o generator.o
	$(CC) $(CFLAGS) -o gerador main.o generator.o

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

generator.o: libs/generator/generator.cpp
	$(CC) $(CFLAGS) -c libs/generator/generator.cpp

clean:
	rm -rf *.o

