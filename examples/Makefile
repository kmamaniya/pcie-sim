#
# PCIe Simulator - Examples Makefile
#
# Copyright (c) 2025 Karan Mamaniya <kmamaniya@gmail.com>
# Licensed under the MIT License
#

# Compiler configuration
CC := gcc
CXX := g++
CFLAGS := -Wall -Wextra -O2 -std=gnu99
CXXFLAGS := -Wall -Wextra -O2 -std=c++11
LDFLAGS := -lpthread

# Output directories
OUT_DIR := ../out
BIN_DIR := $(OUT_DIR)/examples
LIB_OUT_DIR := $(OUT_DIR)/lib

# Library paths
LIB_DIR := ../lib
STATIC_LIB := $(LIB_OUT_DIR)/libpcie_sim.a
SHARED_LIB := $(LIB_OUT_DIR)/libpcie_sim.so

# Example programs
C_EXAMPLE := $(BIN_DIR)/basic_test
CXX_EXAMPLE := $(BIN_DIR)/cpp_test

# Build targets
.PHONY: all static shared clean run-c run-cpp run-multi help dirs

all: dirs static

dirs:
	@mkdir -p $(BIN_DIR)

static: $(C_EXAMPLE) $(CXX_EXAMPLE)

shared: $(C_EXAMPLE)-shared $(CXX_EXAMPLE)-shared

# Build with static library
$(C_EXAMPLE): basic_test.c $(STATIC_LIB)
	@echo "Building C example (static)..."
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -I$(LIB_DIR) -o $@ $< $(STATIC_LIB) $(LDFLAGS)

$(CXX_EXAMPLE): cpp_test.cpp $(STATIC_LIB)
	@echo "Building C++ example (static)..."
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -I$(LIB_DIR) -o $@ $< $(STATIC_LIB) $(LDFLAGS)

# Build with shared library
$(C_EXAMPLE)-shared: basic_test.c $(SHARED_LIB)
	@echo "Building C example (shared)..."
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -I$(LIB_DIR) -L$(LIB_OUT_DIR) -o $@ $< -lpcie_sim $(LDFLAGS)

$(CXX_EXAMPLE)-shared: cpp_test.cpp $(SHARED_LIB)
	@echo "Building C++ example (shared)..."
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -I$(LIB_DIR) -L$(LIB_OUT_DIR) -o $@ $< -lpcie_sim $(LDFLAGS)

# Ensure libraries are built
$(STATIC_LIB):
	@echo "Building static library..."
	$(MAKE) -C $(LIB_DIR) static

$(SHARED_LIB):
	@echo "Building shared library..."
	$(MAKE) -C $(LIB_DIR) shared

clean:
	@echo "Cleaning examples..."
	rm -rf $(OUT_DIR)/examples

run-c: $(C_EXAMPLE)
	@echo "Running C example..."
	@echo "Note: Requires kernel module to be loaded"
	sudo $(C_EXAMPLE)

run-cpp: $(CXX_EXAMPLE)
	@echo "Running C++ example..."
	@echo "Note: Using simulation backend (no kernel module needed)"
	$(CXX_EXAMPLE)

run-multi: $(CXX_EXAMPLE)
	@echo "Running comprehensive 8-device test..."
	@echo "Note: Using simulation backend (no kernel module needed)"
	$(CXX_EXAMPLE) --num-devices 8

run-c-shared: $(C_EXAMPLE)-shared
	@echo "Running C example (shared)..."
	@echo "Note: Requires kernel module to be loaded"
	LD_LIBRARY_PATH=$(LIB_OUT_DIR) sudo $(C_EXAMPLE)-shared

run-cpp-shared: $(CXX_EXAMPLE)-shared
	@echo "Running C++ example (shared)..."
	@echo "Note: Requires kernel module to be loaded"
	LD_LIBRARY_PATH=$(LIB_OUT_DIR) sudo $(CXX_EXAMPLE)-shared

help:
	@echo "PCIe Simulator Examples"
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build all examples (static linking)"
	@echo "  static       - Build examples with static library"
	@echo "  shared       - Build examples with shared library"
	@echo "  clean        - Clean build artifacts"
	@echo "  run-c        - Run C example"
	@echo "  run-cpp      - Run C++ example"
	@echo "  run-c-shared - Run C example (shared library)"
	@echo "  run-cpp-shared - Run C++ example (shared library)"
	@echo ""
	@echo "Examples:"
	@echo "  basic_test.c  - C interface demonstration"
	@echo "  cpp_test.cpp  - C++ interface demonstration"
	@echo ""
	@echo "Usage:"
	@echo "  make all         # Build examples"
	@echo "  make run-c       # Run C example"
	@echo "  make run-cpp     # Run C++ example"