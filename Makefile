CC=g++
CFLAGS=-std=c++20 -lncurses -o fileviewer.out

all:
	$(CC) file_viewer.cpp $(CFLAGS)
