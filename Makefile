CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

SRC_FILES = main.cpp
HEADER_FILES = ThreadPool.h

main: $(SRC_FILES) $(HEADER_FILES)
	$(CXX) $(CXXFLAGS) $(SRC_FILES) -o bin/main

.PHONY: clean

clean:
	rm -f bin/main