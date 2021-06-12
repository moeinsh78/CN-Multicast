CC := g++ -std=c++11
all: network.out router.out client.out group_server.out



network.out: main.o manager.o 
	$(CC) -o network.out main.o manager.o

router.out: router.o
	$(CC) -o router.out router.o

group_server.out: group_server.o
	$(CC) -o group_server.out group_server.o

client.out: client.o
	$(CC) -o client.out client.o




manager.o: manager.cpp manager.hpp settings.h
	$(CC) -c manager.cpp -o manager.o

client.o: client.cpp 
	$(CC) -c client.cpp -o client.o

router.o: router.cpp settings.h
	$(CC) -c router.cpp -o router.o

group_server.o: group_server.cpp 
	$(CC) -c group_server.cpp -o group_server.o

main.o: main.cpp 
	$(CC) -c main.cpp -o main.o



.PHONY: clean
clean:
	rm *.o
	rm *.out
	rm *.pipe
