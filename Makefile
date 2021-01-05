# make
CXX = g++
CC = gcc
CFLAGS = -O3
EXE = Centralip.out Client.out User_interface.out

OBJS_CENTRALIP = sock.o centralip.o
OBJS_CLIENT = client.o sock.o
OBJS_USERINTERFACE = sock.o user_interface.o 

OBJS = client.o user_interface.o sock.o centralip.o

vpath %.cpp src

all: Client.out Centralip.out User_interface.out ${TARGET}

Centralip.out: $(OBJS_CENTRALIP)
		$(CXX) $^ -o $@ 
Client.out: $(OBJS_CLIENT)
		$(CXX) $^ -o $@
User_interface.out: $(OBJS_USERINTERFACE)
			$(CXX) $^ -o $@

%.o: %.cpp
	$(CXX) $< -c $(FLAGS)
clean:
	rm $(EXE) $(OBJS)
