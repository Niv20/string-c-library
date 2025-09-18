## Generic Linked List Library - Makefile
# Usage: make [target]

# ===== Variables =====
CC      := clang
CFLAGS  := -std=c11 -Wall -Wextra -Wpedantic -O2
DEBUG_CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -g -O0
LDFLAGS := 

# Source files
SOURCES := linked_list.c demo.c
OBJECTS := $(SOURCES:.c=.o)
TARGET  := demo
DEBUG_TARGET := demo_debug

# Colored output (remove if unwanted)
GREEN := \033[1;32m
YELLOW := \033[1;33m
BLUE := \033[1;34m
RESET := \033[0m

# ===== Targets =====
.PHONY: all debug run leak clean help rebuild

all: $(TARGET)
	@echo "$(GREEN)[OK] Build complete: $(TARGET)$(RESET)"

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

$(DEBUG_TARGET): CFLAGS := $(DEBUG_CFLAGS)
$(DEBUG_TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@
	@echo "$(YELLOW)[INFO] Debug binary built: $(DEBUG_TARGET)$(RESET)"

# Run normally
run: $(TARGET)
	@echo "$(BLUE)=== Running $(TARGET) ===$(RESET)"
	./$(TARGET)

# Debug with lldb (change to gdb if you prefer)
debug: $(DEBUG_TARGET)
	@echo "$(YELLOW)Launching lldb for $(DEBUG_TARGET)... (use 'run' inside lldb)$(RESET)"
	lldb ./$(DEBUG_TARGET)

# Memory leak check (macOS leaks)
leak: $(DEBUG_TARGET)
	@echo "$(YELLOW)Running leaks (macOS only)$(RESET)"
	leaks -atExit -- ./$(DEBUG_TARGET) || true

# Clean build artifacts
clean:
	@rm -f $(OBJECTS) $(TARGET) $(DEBUG_TARGET)
	@echo "$(BLUE)Build directory cleaned$(RESET)"

# Rebuild from scratch
rebuild: clean all

help:
	@echo "Available targets:"
	@echo "  make / make all  -> Build (default target: $(TARGET))"
	@echo "  make run         -> Build and run"
	@echo "  make debug       -> Build debug binary and launch lldb"
	@echo "  make leak        -> Run macOS leaks tool on debug binary"
	@echo "  make clean       -> Remove objects and binaries"
	@echo "  make rebuild     -> Clean then build"
	@echo "  make help        -> Show this help"

# Generic rule to build object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
