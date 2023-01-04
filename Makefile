all: rest_server

rest_server: rest_server.cc huffman.h pqueue.h bstream.h
	g++ -Wall -Werror -std=c++17 -o rest_server rest_server.cc -lpistache

clean:
	rm -f rest_server
	rm -f *.zap *.unzap
