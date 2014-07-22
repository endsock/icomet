include ../../build.mk

OBJS = subscriber.o channel.o server.o
CFLAGS += -g -I ../

all: $(OBJS)
	$(CC) -o ../../comet-server $(CFLAGS) \
		comet-server.cpp $(OBJS) \
		../util/*.o \
		$(LIBEVENT_PATH)/.libs/libevent.a  \
		$(CLIBS) -l boost_system-mt -l boost_thread-mt -l boost_filesystem-mt -l boost_program_options-mt -l mongoclient
		
subscriber.o: subscriber.h subscriber.cpp
	$(CC) -c $(CFLAGS) subscriber.cpp
channel.o: channel.h channel.cpp
	$(CC) -c $(CFLAGS) channel.cpp
server.o: server.h server.cpp
	$(CC) -c $(CFLAGS) server.cpp

clean:
	rm -f *.o ../../comet-server

