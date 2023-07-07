HEADERS = $(wildcard include/*.hpp)
SOURCE = $(wildcard src/*.cpp) $(wildcard alg-test/*.cpp)

all: $(HEADERS) $(SOURCE)
	g++ -I. -g -std=c++20 $(SOURCE) -o main
