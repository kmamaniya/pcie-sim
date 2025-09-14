# Makefile for PCIe Simulator Utilities
# Copyright (c) 2025 Karan Mamaniya <kmamaniya@gmail.com>
# MIT License

# Compiler settings
CC = gcc
CXX = g++
CFLAGS = -Wall -Wextra -std=c99 -fPIC -O2 -g
CXXFLAGS = -Wall -Wextra -std=c++11 -fPIC -O2 -g

# Output directories
OUT_DIR = ../out
OBJ_DIR = $(OUT_DIR)/utils/obj

# Source files
C_SOURCES = config.c
CXX_SOURCES = options.cpp csv_logger.cpp
C_OBJECTS = $(C_SOURCES:.c=.o)
CXX_OBJECTS = $(CXX_SOURCES:.cpp=.o)
C_OBJECTS_FULL = $(addprefix $(OBJ_DIR)/, $(C_OBJECTS))
CXX_OBJECTS_FULL = $(addprefix $(OBJ_DIR)/, $(CXX_OBJECTS))

ALL_OBJECTS = $(C_OBJECTS_FULL) $(CXX_OBJECTS_FULL)

# Headers
HEADERS = config.h options.hpp csv_logger.hpp

# Targets
.PHONY: all clean help dirs

all: dirs $(ALL_OBJECTS)
	@echo "Built utility objects"

dirs:
	@mkdir -p $(OBJ_DIR)

# Compile C source files
$(OBJ_DIR)/%.o: %.c $(HEADERS)
	@echo "Compiling C utility: $<"
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C++ source files
$(OBJ_DIR)/%.o: %.cpp $(HEADERS)
	@echo "Compiling C++ utility: $<"
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OUT_DIR)/utils
	@echo "Cleaned utilities"

help:
	@echo "PCIe Simulator Utilities Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  all     - Build all utility objects"
	@echo "  clean   - Remove all object files"
	@echo "  help    - Show this help message"
	@echo ""
	@echo "This directory contains shared utilities:"
	@echo "  config.c/.h       - Configuration structures and parsing"
	@echo "  options.cpp/.hpp  - C++ command-line options parser"
	@echo "  csv_logger.cpp/.hpp - High-performance CSV logging"
	@echo ""
	@echo "These utilities are used by:"
	@echo "  - Kernel modules (config.h)"
	@echo "  - Simulation backends (config.h)"
	@echo "  - Example applications (all utilities)"

# Dependency tracking
-include $(ALL_OBJECTS:.o=.d)

$(OBJ_DIR)/%.d: %.c
	@$(CC) $(CFLAGS) -MM $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(OBJ_DIR)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(OBJ_DIR)/%.d: %.cpp
	@$(CXX) $(CXXFLAGS) -MM $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(OBJ_DIR)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$