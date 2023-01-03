all: zap unzap rest_server

zap: zap.cc huffman.h pqueue.h bstream.h
	g++ -Wall -Werror -std=c++17 -o zap zap.cc

unzap: unzap.cc huffman.h
	g++ -Wall -Werror -std=c++17 -o unzap unzap.cc

rest_server: rest_server.cc
	g++ -Wall -Werror -std=c++17 -o rest_server rest_server.cc -lpistache

clean:
	rm -f unzap zap rest_server
	rm -f *.zap *.unzap
