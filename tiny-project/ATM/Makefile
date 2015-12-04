CXX=g++
CXXFLAGS=-std=c++11 -Wall -O3 -g -MMD -MP
LIBS=
TARGET=main

SRC=$(shell ls *.cpp)
SRC_H=$(shell ls *.h)

all: $(TARGET)

$(TARGET): $(SRC:.cpp=.o)
			$(CXX) $(CXXFLAGS) $(LIBS) $+ -o $@

clean:
			$(RM) -r *.o *.d *.dSYM *.txt $(TARGET)

.PHONY: all clean
