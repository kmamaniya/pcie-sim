# Makefile for PCIe Simulator Backend
# Copyright (c) 2025 Karan Mamaniya <kmamaniya@gmail.com>
# MIT License

# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -fPIC -O2 -g
INCLUDES = -I../lib

# Output directories
OUT_DIR = ../out
OBJ_DIR = $(OUT_DIR)/sim/obj

# Platform detection
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    PLATFORM_CFLAGS = -D_GNU_SOURCE
    PLATFORM_LIBS = -lpthread -lrt
    SIM_SOURCES = linux_sim.c
endif

# Windows detection
ifeq ($(findstring CYGWIN,$(UNAME_S)),CYGWIN)
    PLATFORM_CFLAGS = -D_WIN32
    PLATFORM_LIBS = -lkernel32
    SIM_SOURCES = windows_sim.c
endif
ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
    PLATFORM_CFLAGS = -D_WIN32
    PLATFORM_LIBS = -lkernel32
    SIM_SOURCES = windows_sim.c
endif
ifeq ($(findstring MSYS,$(UNAME_S)),MSYS)
    PLATFORM_CFLAGS = -D_WIN32
    PLATFORM_LIBS = -lkernel32
    SIM_SOURCES = windows_sim.c
endif

# Source files
SOURCES = $(SIM_SOURCES)
OBJECTS = $(SOURCES:.c=.o)
OBJECTS_FULL = $(addprefix $(OBJ_DIR)/, $(OBJECTS))

# Targets
.PHONY: all clean help dirs

all: dirs $(OBJECTS_FULL)
	@echo "Built simulation backend objects for $(UNAME_S)"

dirs:
	@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: %.c
	@echo "Compiling simulation backend: $<"
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(PLATFORM_CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OUT_DIR)/sim
	@echo "Cleaned simulation backend"

help:
	@echo "PCIe Simulator Backend Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  all     - Build platform-specific simulation backend"
	@echo "  clean   - Remove all object files"
	@echo "  help    - Show this help message"
	@echo ""
	@echo "Platform detected: $(UNAME_S)"
	@echo "Sources to build: $(SIM_SOURCES)"
	@echo ""
	@echo "This directory contains cross-platform simulation backends:"
	@echo "  linux_sim.c   - Linux implementation with pthread synchronization"
	@echo "  windows_sim.c - Windows implementation with CRITICAL_SECTION"

# Dependency tracking
-include $(OBJECTS:.o=.d)

%.d: %.c
	@$(CC) $(CFLAGS) $(PLATFORM_CFLAGS) $(INCLUDES) -MM $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$