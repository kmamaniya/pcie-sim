#
# PCIe Simulator - Library Makefile
#
# Copyright (c) 2025 Karan Mamaniya <kmamaniya@gmail.com>
# Licensed under the MIT License
#

# Compiler configuration
CC := gcc
CXX := g++
CFLAGS := -Wall -Wextra -O2 -std=gnu99 -fPIC
CXXFLAGS := -Wall -Wextra -O2 -std=c++11 -fPIC
AR := ar

# Output directories
OUT_DIR := ../out
OBJ_DIR := $(OUT_DIR)/lib/obj
LIB_DIR := $(OUT_DIR)/lib

# Library configuration
LIB_NAME := pcie_sim
STATIC_LIB := $(LIB_DIR)/lib$(LIB_NAME).a
SHARED_LIB := $(LIB_DIR)/lib$(LIB_NAME).so.1.0
SHARED_LIB_SONAME := $(LIB_DIR)/lib$(LIB_NAME).so.1
SHARED_LIB_LINK := $(LIB_DIR)/lib$(LIB_NAME).so

# Source files
C_SOURCES := core.c utils.c
CXX_SOURCES := device.cpp
C_OBJECTS := $(C_SOURCES:.c=.o)
CXX_OBJECTS := $(CXX_SOURCES:.cpp=.o)
C_OBJECTS_FULL := $(addprefix $(OBJ_DIR)/, $(C_OBJECTS))
CXX_OBJECTS_FULL := $(addprefix $(OBJ_DIR)/, $(CXX_OBJECTS))

# Simulation backend objects (built by sim/ Makefile)
SIM_OUT_DIR := $(OUT_DIR)/sim/obj
# Only include objects that exist for current platform
SIM_OBJECTS := $(wildcard $(SIM_OUT_DIR)/*.o)

# Utility objects (built by utils/ Makefile)
UTILS_OUT_DIR := $(OUT_DIR)/utils/obj
UTILS_OBJECTS := $(wildcard $(UTILS_OUT_DIR)/*.o)

ALL_OBJECTS := $(C_OBJECTS_FULL) $(CXX_OBJECTS_FULL)

# Headers
C_HEADERS := api.h types.h pcie_sim.h
CXX_HEADERS := device.hpp monitor.hpp

# Build targets
.PHONY: all static shared clean install help dirs

all: dirs static shared

dirs:
	@mkdir -p $(OBJ_DIR) $(LIB_DIR)

static: $(STATIC_LIB)

shared: $(SHARED_LIB)

$(STATIC_LIB): $(ALL_OBJECTS) $(SIM_OBJECTS) $(UTILS_OBJECTS)
	@echo "Building static library..."
	@mkdir -p $(LIB_DIR)
	$(AR) rcs $@ $^
	@echo "Static library created: $@"

$(SHARED_LIB): $(ALL_OBJECTS) $(SIM_OBJECTS) $(UTILS_OBJECTS)
	@echo "Building shared library..."
	@mkdir -p $(LIB_DIR)
	$(CXX) -shared -Wl,-soname,$(SHARED_LIB_SONAME) -o $@ $^
	cd $(LIB_DIR) && ln -sf $(notdir $(SHARED_LIB)) $(notdir $(SHARED_LIB_SONAME))
	cd $(LIB_DIR) && ln -sf $(notdir $(SHARED_LIB)) $(notdir $(SHARED_LIB_LINK))
	@echo "Shared library created: $@"

# Compile C source files
$(OBJ_DIR)/%.o: %.c $(C_HEADERS)
	@echo "Compiling C: $<"
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C++ source files
$(OBJ_DIR)/%.o: %.cpp $(CXX_HEADERS) $(C_HEADERS)
	@echo "Compiling C++: $<"
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "Cleaning library..."
	rm -rf $(OUT_DIR)/lib

install: all
	@echo "Installing library..."
	sudo mkdir -p /usr/local/lib /usr/local/include/pcie_sim
	sudo cp $(LIB_DIR)/* /usr/local/lib/
	sudo cp $(C_HEADERS) $(CXX_HEADERS) /usr/local/include/pcie_sim/
	sudo ldconfig
	@echo "Library installed"

uninstall:
	@echo "Uninstalling library..."
	sudo rm -f /usr/local/lib/lib$(LIB_NAME)*
	sudo rm -rf /usr/local/include/pcie_sim
	sudo ldconfig
	@echo "Library uninstalled"

help:
	@echo "PCIe Simulator Library"
	@echo ""
	@echo "Targets:"
	@echo "  all       - Build static and shared libraries"
	@echo "  static    - Build static library (.a)"
	@echo "  shared    - Build shared library (.so)"
	@echo "  clean     - Clean build artifacts"
	@echo "  install   - Install library system-wide"
	@echo "  uninstall - Remove installed library"
	@echo ""
	@echo "C Library Components:"
	@echo "  core.c    - Core API implementation"
	@echo "  utils.c   - Utility functions"
	@echo "  api.h     - API declarations"
	@echo "  types.h   - Type definitions"
	@echo ""
	@echo "C++ Library Components:"
	@echo "  device.cpp - C++ wrapper implementation"
	@echo "  device.hpp - Device interface"
	@echo "  monitor.hpp - Monitoring tools"
	@echo ""
	@echo "Usage:"
	@echo "  make static          # Build static library"
	@echo "  make shared          # Build shared library"
	@echo "  sudo make install    # Install system-wide"