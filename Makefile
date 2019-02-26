all: gerador clean
	
gerador: main.o
	g++ main.o -o gerador

main.o: main.cpp
	g++ -c -g main.cpp

clean:
	rm -rf *.o