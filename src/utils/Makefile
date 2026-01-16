CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -pedantic
CFLAGS += -I../../include

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

# Source files from subdirectories
SRCS_MEMANAGER := $(wildcard memanager/*.c)
SRCS_COMAP := $(wildcard comap/*.c)

# Transform to object files, preserving directory structure
OBJS_MEMANAGER := $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRCS_MEMANAGER))
OBJS_COMAP := $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRCS_COMAP))

# Combine all objects
OBJS := $(OBJS_MEMANAGER) $(OBJS_COMAP)

# Library output
TARGET := utils.a

# Default target
all: $(TARGET)

# Create object directory structure
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)/memanager $(OBJ_DIR)/comap

# Compile rule
$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Archive
$(TARGET): $(OBJS)
	ar rcs $@ $^

# Clean
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean