CXX := clang++
FORMATTER := clang-format
CXXFLAGS += -g -Wall -Wextra -std=c++17 -fsanitize=address

test: test.cpp test.h
	$(CXX) $< -o $@ $(CXXFLAGS)

clean:
	rm -rf test test.dSYM

format:
	$(FORMATTER) -i -style="Google" $(wildcard *.cpp) $(wildcard *.h)

.PHONY: clean format

