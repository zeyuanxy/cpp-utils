#
# $File: Makefile
# $Date: Fri Sep 12 22:51:24 2014 +0800
#
# A single output portable Makefile for
# simple c++ project

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
LIB_DIR = lib
TARGET = sift2 #THIS IS BINARY NAME

CXX = g++
#CXX = clang++

BIN_TARGET = $(BIN_DIR)/$(TARGET)
#PROF_FILE = $(BIN_TARGET).prof

INCLUDE_DIR = -I $(SRC_DIR) -I $(SRC_DIR)/include


 CXXFLAGS += -O2 -fopenmp
# CXXFLAGS = -g
# CXXFLAGS += -pg


#CXXFLAGS += $(DEFINES)
CXXFLAGS += -std=c++11
#CXXFLAGS += -ansi
CXXFLAGS += -Wall -Wextra
CXXFLAGS += $(INCLUDE_DIR)
#CXXFLAGS += $(LDFLAGS)
#CXXFLAGS += $(shell pkg-config --libs --cflags opencv)
#CXXFLAGS += -pthread -lpthread
CXXFLAGS += -lopencv_core
CXXFLAGS += -lopencv_highgui
CXXFLAGS += -lopencv_imgproc
CXXFLAGS += -lopencv_calib3d

# CXXFLAGS += -fPIC


#CC = /usr/share/clang/scan-build/ccc-analyzer
#CXX = /usr/share/clang/scan-build/c++-analyzer
CXXSOURCES = $(shell find $(SRC_DIR)/ -name "*.cpp")
OBJS = $(addprefix $(OBJ_DIR)/,$(CXXSOURCES:.cpp=.o))
DEPFILES = $(OBJS:.o=.d)

.PHONY: all clean run rebuild gdb

all: $(BIN_TARGET)

$(LIB_DIR):
	mkdir $(LIB_DIR)

$(OBJ_DIR)/%.o: %.cpp
	@echo "[CC] $< ..."
	@$(CXX) -c $< $(CXXFLAGS) -o $@

$(OBJ_DIR)/%.d: %.cpp
	@mkdir -pv $(dir $@)
	@echo "[dep] $< ..."
	@$(CXX) $(INCLUDE_DIR) $(CXXFLAGS) -MM -MT "$(OBJ_DIR)/$(<:.cpp=.o) $(OBJ_DIR)/$(<:.cpp=.d)" "$<" > "$@"

sinclude $(DEPFILES)

$(BIN_TARGET): $(OBJS)
	@echo "[link] $< ..."
	@mkdir -p $(BIN_DIR)
	@$(CXX) $(OBJS) -o $@ $(CXXFLAGS)
	@echo have a nice day!

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(LIB_DIR)

run: $(BIN_TARGET)
	./$(BIN_TARGET)

rebuild:
	+@make clean
	+@make

gdb: $(BIN_TARGET)
	gdb ./$(BIN_TARGET)