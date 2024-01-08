CC := g++
CFLAGS := -std=c++11 -pthread

all: build

build: server client

server: Server.cpp
	$(CC) $(CFLAGS) -o server Server.cpp

client: Langton.cpp
	$(CC) $(CFLAGS) -o client Langton.cpp

clean:
	rm -f server client
