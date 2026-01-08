CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -g
CFLAGS += -I../../../include -I./unity
LDFLAGS =

OBJ_DIR := obj
BIN_DIR := bin
UNITY_DIR := unity

MEMANAGER_DIR := ../../../src/utils/memanager
MEMANAGER_LIB := $(MEMANAGER_DIR)/libmemanager.a

UNITY_SRC := $(UNITY_DIR)/unity.c
UNITY_OBJ := $(OBJ_DIR)/unity.o

TEST_SRCS := $(wildcard test_*.c)
TEST_EXES := $(patsubst %.c,$(BIN_DIR)/%,$(TEST_SRCS))

all: run-tests

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(MEMANAGER_LIB):
	$(MAKE) -C $(MEMANAGER_DIR) DEBUG=1

$(UNITY_OBJ): $(UNITY_SRC) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/test_%: test_%.c $(UNITY_OBJ) $(MEMANAGER_LIB) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

build: $(TEST_EXES)

run-tests: build
	@for test in $(TEST_EXES); do \
		echo "▶ Running $$test"; \
		$$test || exit 1; \
	done

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all build run-tests clean
