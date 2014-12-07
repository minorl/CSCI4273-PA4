CC=g++
FLAGS=-g -std=gnu++11 -Wall -Wextra
LIBS=-lpthread

all: hostPP timePP

timePP: timePP.cc
	$(CC) $(FLAGS) -o $@ $^ 	

hostPP: hostPP.cc message.o ppProtocol.o
	$(CC) $(FLAGS) -o $@ $^ $(LIBS)

driver1: driver1.cc threadpool.o
	$(CC) $(FLAGS) -o $@ $^ $(LIBS)

driver2: driver2.cc eventscheduler.o threadpool.o
	$(CC) $(FLAGS) -o $@ $^ $(LIBS)

driver3: driver3.cc message.o
	$(CC) $(FLAGS) -o $@ $^ $(LIBS)

threadpool.o: threadpool.cc
	$(CC) $(FLAGS) -o $@ -c $^

eventscheduler.o: eventscheduler.cc
	$(CC) $(FLAGS) -o $@ -c $^

message.o: message.cc
	$(CC) $(FLAGS) -o $@ -c $^

ppProtocol.o: ppProtocol.cc
	$(CC) $(FLAGS) -o $@ -c $^

clean:
	rm -f threadpool.o eventscheduler.o message.o ppProtocol.o timePP hostPP driver1 driver2 driver3
