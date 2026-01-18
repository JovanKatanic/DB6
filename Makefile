CC = gcc
BASE_CFLAGS = -Wall -Wextra -std=c17 -pedantic -Iinclude
LDFLAGS =

# Build mode: debug or release (default: release)
BUILD ?= release

ifeq ($(BUILD),debug)
    CFLAGS = $(BASE_CFLAGS) -g -O0 -DDEBUG
else
    CFLAGS = $(BASE_CFLAGS) -O2 -DNDEBUG
endif

BIN_DIR := bin
OBJ_DIR := obj

TARGET := $(BIN_DIR)/app

MAIN_SRC := src/main.c
MAIN_OBJ := $(OBJ_DIR)/main.o

UTILS_DIR := src/utils
UTILS_LIB := $(UTILS_DIR)/utils.a

all: $(TARGET)

# Directories
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Build utils
$(UTILS_LIB):
	$(MAKE) -C $(UTILS_DIR) BUILD=$(BUILD)

# Build memanager
# $(COHMAP_LIB):
# 	$(MAKE) -C $(COHMAP_DIR) BUILD=$(BUILD)

# Compile main.c
$(MAIN_OBJ): $(MAIN_SRC) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link final executable
$(TARGET): $(MAIN_OBJ) $(UTILS_LIB) | $(BIN_DIR)
	$(CC) $(MAIN_OBJ) $(UTILS_LIB) -o $@ $(LDFLAGS)

# $(TARGET): $(MAIN_OBJ) $(UTILS_LIB) $(UTILS_LIB2) | $(BIN_DIR)
# 	$(CC) $(MAIN_OBJ) $(UTILS_LIB) $(UTILS_LIB2) -o $@ $(LDFLAGS)


run: $(TARGET)
	./$(TARGET)

clean:
	$(MAKE) -C $(UTILS_DIR) clean
# 	$(MAKE) -C $(COHMAP_DIR) clean
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean run