CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -pedantic
CFLAGS += -I../../../include

# Build mode
BUILD ?= release
ifeq ($(BUILD),debug)
    CFLAGS += -g -O0 -DDEBUG
else
    CFLAGS += -O2 -DNDEBUG
endif

# Local directories
SRC_DIR := .
OBJ_DIR := obj

# Sources and objects
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRCS))

# Library output
TARGET := comap.a

# Default target
all: $(TARGET)

# Object directory
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Compile
$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Archive
$(TARGET): $(OBJS)
	ar rcs $@ $^

# Clean
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean