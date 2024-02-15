all: serveur client

serveur: serveur.o GameServer.o GameClient.o 
	g++ -o serveur serveur.o GameServer.o GameClient.o

client: client.o GameClient.o
	g++ -o client client.o GameClient.o 
	
serveur.o: serveur.cpp
	g++ -c serveur.cpp

GameServer.o: GameServer.cpp GameServer.h
	g++ -c GameServer.cpp

GameClient.o: GameClient.cpp GameClient.h
	g++ -c GameClient.cpp
clean:
	rm *.o