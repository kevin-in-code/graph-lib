CXX=g++
CXXFLAGS=-std=c++11 -W -Wextra -pedantic -O3 -I./include/
AR=ar

all: ./bin/Main_TCS_2015 ./bin/CountMaximalCliques

./bin/Main_TCS_2015: ./build/Main_TCS_2015.o ./lib/graphlib.a
    $(CXX) $(CXXFLAGS) $^ -o $@

./build/Main_TCS_2015.o: ./programs/Main_TCS_2015/main.cpp ./build
    $(CXX) $(CXXFLAGS) -c $< -o $@



./bin/CountMaximalCliques: ./build/CountMaximalCliques.o ./lib/graphlib.a
    $(CXX) $(CXXFLAGS) $^ -o $@

./build/CountMaximalCliques.o: ./programs/CountMaximalCliques/main.cpp ./build
    $(CXX) $(CXXFLAGS) -c $< -o $@



./lib/graphlib.a: ./build/MersenneTwister.o ./build/BitStructures.o ./build/Graph.o ./build/Graph_ErdosRenyi.o ./build/CliqueEnumeration.o
    mkdir -p ./lib
    $(AR) rvs ./lib/graphlib.a $^

./build/MersenneTwister.o: ./src/MersenneTwister.cpp ./build
    $(CXX) $(CXXFLAGS) -c $< -o $@

./build/BitStructures.o: ./src/BitStructures.cpp ./build
    $(CXX) $(CXXFLAGS) -c $< -o $@

./build/Graph.o: ./src/Graph.cpp ./build
    $(CXX) $(CXXFLAGS) -c $< -o $@

./build/Graph_ErdosRenyi.o: ./src/Graph_ErdosRenyi.cpp ./build
    $(CXX) $(CXXFLAGS) -c $< -o $@

./build/CliqueEnumeration.o: ./src/CliqueEnumeration.cpp ./build
    $(CXX) $(CXXFLAGS) -c $< -o $@

./build:
    mkdir -p ./build

clean:
    rm -rf ./build
    rm -rf ./lib/*.a
