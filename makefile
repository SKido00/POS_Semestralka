CC := g++
CFLAGS := -std=c++11 -pthread

all: build

build: server client

server: server.cpp
	$(CC) $(CFLAGS) -o server server.cpp

client: Langton.cpp
	$(CC) $(CFLAGS) -o client Langton.cpp

clean:
	rm -f server client
