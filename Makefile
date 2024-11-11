# 伪命令
.PHONY: clean buildlib buildso

BUILD_DIR := build
SRC_DIR := src

CC := /usr/bin/gcc
CXX := /usr/bin/g++
AR := /usr/bin/ar

LIB_NAME := sparse_kmeans_tree
LIB_TARGET := $(BUILD_DIR)/$(LIB_NAME).lib

INC_DIRS := /usr/include
INC_DIRS := $(addprefix -I, $(INC_DIRS))

SRCS_C := $(shell find $(SRC_DIR) -name '*.c')
SRCS_CXX := $(shell find $(SRC_DIR) -name '*.cpp')

NOT_DIR_SRCS_C := $(notdir $(SRCS_C))
NOT_DIR_SRCS_CXX := $(notdir $(SRCS_CXX))

OBJS_C := $(patsubst %.c, $(BUILD_DIR)/%.o, $(NOT_DIR_SRCS_C))
OBJS_CXX := $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(NOT_DIR_SRCS_CXX))
OBJS := $(OBJS_C) $(OBJS_CXX)

CFLAGS := -lpthread -fopenmp

$(LIB_TARGET) : $(OBJS)
	$(AR) rcs -o $(LIB_TARGET) $^

$(OBJS_C): $(SRCS_C)
	$(CC) $(CFLAGS) $(INC_DIRS) -c $< -o $@

$(OBJS_CXX): $(SRCS_CXX)
	$(CXX) $(CFLAGS) $(INC_DIRS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)/*

default: $(LIB_TARGET)