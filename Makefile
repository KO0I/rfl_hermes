CC ?= gcc
PKG_CONFIG ?= pkg-config
CFLAGS ?= -std=c99 -Wall -Wextra -Wpedantic -O2 -g
RAYLIB_CFLAGS := $(shell $(PKG_CONFIG) --cflags raylib)
RAYLIB_LIBS := $(shell $(PKG_CONFIG) --libs raylib)
LDLIBS := $(RAYLIB_LIBS) -lm

BUILD_DIR := build
TARGET := $(BUILD_DIR)/rfl_hermes
SOURCES := src/main.c src/character_creator.c src/character_renderer.c
TEST_CHARACTER_CREATOR := $(BUILD_DIR)/test_character_creator

.PHONY: all run clean test

all: $(TARGET)

$(TARGET): $(SOURCES) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(RAYLIB_CFLAGS) -Isrc -o $@ $(SOURCES) $(LDLIBS)

test: $(TEST_CHARACTER_CREATOR)
	./$(TEST_CHARACTER_CREATOR)

$(TEST_CHARACTER_CREATOR): tests/test_character_creator.c src/character_creator.c src/character_creator.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(RAYLIB_CFLAGS) -Isrc -o $@ tests/test_character_creator.c src/character_creator.c $(LDLIBS)

$(BUILD_DIR):
	mkdir -p $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
