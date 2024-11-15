# 伪命令
.PHONY: clean test

BUILD_DIR := build
SRC_DIR := src
TEST_DIR := test
BIN_DIR := bin

CC := /usr/bin/gcc
CXX := /usr/bin/g++
AR := /usr/bin/ar

LIB_NAME := sparse_kmeans_tree
LIB_TARGET := $(BUILD_DIR)/$(LIB_NAME).lib

INC_DIRS := /usr/include ./_3rdparty/nlohmann_json
INC_DIRS := $(addprefix -I, $(INC_DIRS))

TEST_INC_DIRS := /usr/include ./_3rdparty/doctest ./_3rdparty/nlohmann_json src
TEST_INC_DIRS := $(addprefix -I, $(TEST_INC_DIRS))

SRCS_C := $(shell find $(SRC_DIR) -name '*.c')
SRCS_CXX := $(shell find $(SRC_DIR) -name '*.cpp')

NOT_DIR_SRCS_C := $(notdir $(SRCS_C))
NOT_DIR_SRCS_CXX := $(notdir $(SRCS_CXX))

OBJS_C := $(patsubst %.c, $(BUILD_DIR)/%.o, $(NOT_DIR_SRCS_C))
OBJS_CXX := $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(NOT_DIR_SRCS_CXX))
OBJS := $(OBJS_C) $(OBJS_CXX)

TEST_SRCS_C := $(shell find $(TEST_DIR) -name '*.c')
TEST_SRCS_CXX := $(shell find $(TEST_DIR) -name '*.cpp')

TEST_NOT_DIR_SRCS_C := $(notdir $(TEST_SRCS_C))
TEST_NOT_DIR_SRCS_CXX := $(notdir $(TEST_SRCS_CXX))

TEST_OBJS_C := $(patsubst %.c, $(BUILD_DIR)/%.o, $(TEST_NOT_DIR_SRCS_C))
TEST_OBJS_CXX := $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(TEST_NOT_DIR_SRCS_CXX))
TEST_OBJS := $(TEST_OBJS_C) $(TEST_OBJS_CXX)

CFLAGS := -lpthread -fopenmp -lstdc++ -std=c++11 -lm -g
TEST_FLAGS := 

$(LIB_TARGET) : $(OBJS)
	$(AR) rcs -o $(LIB_TARGET) $^
	mkdir -p $(BIN_DIR)
	mv $@ $(BIN_DIR)/

$(BUILD_DIR)/unit_test: $(OBJS) $(TEST_OBJS) $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^
	mkdir -p $(BIN_DIR)
	mv $@ $(BIN_DIR)/
	
$(BUILD_DIR): 
	mkdir -p $(BUILD_DIR)

$(OBJS_C): $(BUILD_DIR)/%.o : $(SRC_DIR)/%.c $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INC_DIRS) -c $< -o $@

$(OBJS_CXX): $(BUILD_DIR)/%.o : $(SRC_DIR)/%.cpp $(BUILD_DIR)
	$(CXX) $(CFLAGS) $(INC_DIRS) -c $< -o $@

$(TEST_OBJS_C): $(BUILD_DIR)/%.o : $(TEST_DIR)/%.c $(BUILD_DIR)
	$(CC) $(TEST_FLAGS) $(CFLAGS) $(TEST_INC_DIRS) -c $< -o $@

$(TEST_OBJS_CXX): $(BUILD_DIR)/%.o : $(TEST_DIR)/%.cpp $(BUILD_DIR)
	$(CXX) $(TEST_FLAGS) $(CFLAGS) $(TEST_INC_DIRS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)/*

test: $(BUILD_DIR)/unit_test