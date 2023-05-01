
server: server.o
	g++ -g -Wall -pedantic server.cpp libastyle-2.06d.so -o server

server.o: server.cpp
	g++ -c -o server.o server.cpp

run:
	LD_LIBRARY_PATH=$(pwd): ./server 

clean: 
	rm -f *.o server	
