# Makefile
PROGRAM = test
PROGRAM_ADM = tenken
OBJS = net.o gpio.o cap.o sw.o inspect.o mysql_client.o main.o
CC = g++
CFLAGS = -Wall -O2

all: $(PROGRAM_ADM) $(PROGRAM)
$(PROGRAM_ADM): $(OBJS)
	$(CC) -o $(PROGRAM_ADM) $^ -lwiringPi -lmysqlclient -lpthread
$(PROGRAM): $(OBJS)
	$(CC) -o $(PROGRAM) $^ -lwiringPi -lmysqlclient -lpthread
.c.o:
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	$(RM) $(PROGRAM) $(OBJS)
	$(RM) $(PROGRAM_ADM) $(OBJS)
	
net.o : net.cpp net.h debug.h
gpio.o : common.h gpio.h
cap.o : common.h cap.h debug.h
sw.o : common.h gpio.h sw.h debug.h
inspect.o : common.h inspect.h gpio.h cap.h debug.h
mysql_client.o: common.h mysql_client.h main.h gpio.h debug.h
main.o	: common.h mysql_client.h inspect.h gpio.h sw.h cap.h main.h debug.h net.h
