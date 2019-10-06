#gcc -Wall main.c hello_fn.c -o newhello
#default: main.c hello_fn.c
#	gcc -Wall main.c hello_fn.c -o newhello -I.
#gcc -o fram_test -L/usr/local/lib -I/usr/local/include/buspirate fram_test.c fram.c -lbuspirate
#Estos comandos si funcionan
#g++ -c fram_test.c -I/usr/local/include/buspirate
#g++ -c fram.c -I/usr/local/include/buspirate
#g++ -o fram fram_test.o fram.o -L/usr/local/lib -lbuspirate
CC=g++
CFLAGS= -c -std=c++11 -Wall
#IFLAGS= -I/usr/local/include
#LFLAGS= -L/usr/local/lib
OBJECTS= Logger.o medicion_laser.o
LIBS = -lboost_filesystem -lboost_system -lwiringPi
PROG_NAME = medicion_laser

all: prog

prog: $(OBJECTS)
	$(CC) $(OBJECTS) -o $(PROG_NAME) $(LFLAGS) $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) $(IFLAGS) $<
	
clean:
	rm -rf *.o

clean_all:
	rm -rf *.o $(PROG_NAME)
