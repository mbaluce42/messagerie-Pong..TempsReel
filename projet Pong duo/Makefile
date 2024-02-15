
serveur: serveur.o GameServer.o GameClient.o bat.o ball.o
	g++ -o serveur serveur.o GameServer.o GameClient.o bat.o ball.o -lsfml-graphics -lsfml-window -lsfml-system

client: client.o GameClient.o bat.o ball.o
	g++ -o client client.o GameClient.o bat.o ball.o -lsfml-graphics -lsfml-window -lsfml-system
	
serveur.o: serveur.cpp
	g++ -c serveur.cpp

GameServer.o: GameServer.cpp GameServer.h
	g++ -c GameServer.cpp

GameClient.o: GameClient.cpp GameClient.h
	g++ -c GameClient.cpp

bat.o: bat.cpp bat.h
	g++ -c bat.cpp

ball.o: ball.cpp ball.h
	g++ -c ball.cpp

clean:
	rm *.o