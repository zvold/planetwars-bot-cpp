all: release

OPTS := -O2 -Wall
objects := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

MyBot.o : MyBot.cc
	g++ $(OPTS) -c $<
	
%.o : %.cpp
	g++ $(OPTS) -c $<

release : $(objects) MyBot.o
	g++ -o bot-cpp $(objects) MyBot.o

clean :
	rm -f bot-cpp
	rm -f *.o

