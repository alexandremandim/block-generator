all: gerador clean
	
gerador: main.o
	g++ main.o -o gerador

main.o: src/main.cpp
	g++ -c  src/main.cpp

clean:
	rm -rf *.o