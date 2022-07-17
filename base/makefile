# Makefile for Final Project

# VARIABLES #

FLAGS = -g -std=c++11 -Wall -O2
OFILES = bigNumber.o sha.o message.o node.o linkedList.o credentials.o user.o randomizer.o rsaKey.o fileSystem.o
# for extra flags for e.g. valgrind or debug
eflags ?= 

# APP #

app: main.o $(OFILES)
	g++ main.o $(OFILES) -lcrypto -lsqlite3 -o app

# MAIN #

main.o: main.cpp mydefs.h sha.h bigNumber.h message.h node.h linkedList.h credentials.h user.h randomizer.h rsaKey.h fileSystem.h
	g++ $(FLAGS) -c main.cpp

# OTHER FILES #

fileSystem.o: fileSystem.cpp fileSystem.h user.h linkedList.h randomizer.h message.h
	g++ $(FLAGS) -c fileSystem.cpp

sqlSystem.o: sqlSystem.cpp sqlSystem.h user.h randomizer.h
	g++ $(FLAGS) -c sqlSystem.cpp

rsaKey.o: rsaKey.cpp rsaKey.h bigNumber.h randomizer.h
	g++ $(FLAGS) -c rsaKey.cpp

randomizer.o: randomizer.cpp randomizer.h sha.h
	g++ $(FLAGS) -c randomizer.cpp

user.o: user.cpp user.h credentials.h message.h linkedList.h
	g++ $(FLAGS) -c user.cpp

credentials.o: credentials.cpp credentials.h
	g++ $(FLAGS) -c credentials.cpp

# linkedList.cpp is just an include and a comment
linkedList.o: linkedList.h
	g++ $(FLAGS) -c linkedList.cpp

# node.cpp is just an include and a comment
node.o: node.h
	g++ $(FLAGS) -c node.cpp

message.o: message.cpp message.h
	g++ $(FLAGS) -c message.cpp

sha.o: sha.cpp sha.h
	g++ $(FLAGS) -c sha.cpp

bigNumber.o: bigNumber.cpp bigNumber.h
	g++ $(FLAGS) -c bigNumber.cpp

# UTILITY #

run: app
	./app

debug: app
	gdb $(eflags) ./app

valgrind: app
	valgrind $(eflags) ./app

valgrind-full: app
	valgrind -v --leak-check=full ./app

valgrind-track-origins: app
	valgrind -v --leak-check=full --track-origins=yes ./app

clean:
	rm -f *.o
	rm -f app

