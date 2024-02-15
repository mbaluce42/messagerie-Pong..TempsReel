CC=g++
CFLAGS=-I.

all: main ball.o bat.o
main: main.o ball.o bat.o
	g++ main.o ball.o bat.o -o main -lsfml-graphics -lsfml-window -lsfml-system
main.o:
	g++ -c main.cpp ball.cpp bat.cpp
ball.o: 
	g++ -c ball.cpp
bat.o:
	g++ -c bat.cpp
clean:
	rm *.o

