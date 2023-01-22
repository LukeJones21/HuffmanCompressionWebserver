CXXFLAGS = -g -Wall -Werror -std=c++17

all: rest_server2

rest_server: rest_server.cc huffman.h pqueue.h bstream.h
	g++ $(CXXFLAGS) -o rest_server rest_server.cc -lpistache

rest_server2: rest_server2.cc huffman.h pqueue.h bstream.h
	g++ $(CXXFLAGS) -o rest_server2 rest_server2.cc -lpistache -lssl -lcrypto

clean:
	rm -f rest_server2
	rm -f *.zap *.unzap
