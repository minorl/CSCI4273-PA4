CC=g++
FLAGS=-g -std=gnu++11 -Wall -Wextra
LIBS=-lpthread

all: ppmrunner ppprunner timePP timePM

ppmrunner: ppmrunner.cpp ppm.o message.o threadpool.o
	$(CC) $(FLAGS) -o $@ $^ $(LIBS)

ppprunner: ppprunner.cpp message.o ppProtocol.o
	$(CC) $(FLAGS) -o $@ $^ $(LIBS)

ppm.o: ppm.cpp
	$(CC) $(FLAGS) -o $@ -c $^

ppp.o: ppp.cpp
	$(CC) $(FLAGS) -o $@ -c $^

threadpool.o: threadpool.cpp
	$(CC) $(FLAGS) -o $@ -c $^

message.o: message.cpp
	$(CC) $(FLAGS) -o $@ -c $^

timePP: timePP.cpp
	$(CC) $(FLAGS) -o $@ $^ 

timePM: timePM.cpp
	$(CC) $(FLAGS) -o $@ $^ 
	
clean:
	rm -f threadpool.o message.o ppp.o ppm.o ppmrunner ppprunner
