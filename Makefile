CC=g++
FLAGS=-g -std=gnu++11 -Wall -Wextra
LIBS=-lpthread

all: host1

host1: host1.cc message.o ppProtocol.o
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
	rm -f threadpool.o eventscheduler.o message.o ppProtocol.o host1 driver1 driver2 driver3
