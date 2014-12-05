CC = g++
CFLAGS = -c -g -Wall -Wextra
LFLAGS = -Wall -Wextra -pthread
.PHONY: all clean
all: test
test: test-thread test-message test-scheduler
test-thread: driver1.o threadpool.o
	$(CC) $(LFLAGS) $^ -o $@
threadpool.o: threadpool.cc
	$(CC) $(CFLAGS) $<
driver1.o: driver1.cc
	$(CC) $(CFLAGS) $<
test-message: driver3.o message.o
	$(CC) $(LFLAGS) $^ -o $@
message.o: message.cc
	$(CC) $(CFLAGS) $<
driver3.o: driver3.cc
	$(CC) $(CFLAGS) $<
test-scheduler: driver2.o eventscheduler.o threadpool.o
	$(CC) $(LFLAGS) $^ -o $@
eventscheduler.o: eventscheduler.cc
	$(CC) $(CFLAGS) $<
driver2.o: driver2.cc
	$(CC) $(CFLAGS) $<	

clean:
	rm -f *.o
	rm -f test-thread
	rm -f test-message
	rm -f test-scheduler
tar:
	tar -cf minorlpa3.tar threadpool.cc threadpool.h message.cc message.h eventscheduler.cc eventscheduler.h driver1.cc driver2.cc driver3.cc README Makefile