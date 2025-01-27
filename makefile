all: compile link clean

compile:
	g++ -Isrc/include -c main.cpp

link:
	g++ -std=c++11 -pthread -o main main.o
clean:
	del main.o
