CC=g++
FLAGS=-g -std=gnu++11 -Wall -Wextra
LIBS=-lpthread

all: ppmrunner ppprunner time

ppmrunner: ppmrunner.cpp ppm.o message.o threadpool.o
	$(CC) $(FLAGS) -o $@ $^ $(LIBS)

ppprunner: ppprunner.cpp message.o
	$(CC) $(FLAGS) -o $@ $^ $(LIBS)

ppm.o: ppm.cpp
	$(CC) $(FLAGS) -o $@ -c $^

ppp.o: ppp.cpp
	$(CC) $(FLAGS) -o $@ -c $^

threadpool.o: threadpool.cpp
	$(CC) $(FLAGS) -o $@ -c $^

message.o: message.cpp
	$(CC) $(FLAGS) -o $@ -c $^

time: time.cpp
	$(CC) $(FLAGS) -o $@ $^ 

clean:
	rm -f threadpool.o message.o ppp.o ppm.o ppmrunner ppprunner time
