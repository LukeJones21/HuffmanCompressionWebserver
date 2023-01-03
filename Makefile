all: zap unzap

zap: zap.cc huffman.h pqueue.h bstream.h
	g++ -Wall -Werror -std=c++11 -o zap zap.cc

unzap: unzap.cc huffman.h
	g++ -Wall -Werror -std=c++11 -o unzap unzap.cc

clean:
	rm -f unzap zap
	rm -f *.zap *.unzap
