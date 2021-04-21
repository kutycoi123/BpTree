CXX = g++
CXXFLAGS = -std=c++11 -g -Wall

all: main

main: main.o BpTree.o Node.o
	$(CXX) $(CXXFLAGS) -o main main.o BpTree.o Node.o

main.o: main.cpp BpTree.h Node.h
	$(CXX) $(CXXFLAGS) -c main.cpp

BpTree.o: BpTree.h

Node.o: Node.h

clean:
	rm -rf *.o main






