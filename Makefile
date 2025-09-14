#
# PCIe Simulator - Main Project Makefile
#
# Copyright (c) 2025 Karan Mamaniya <kmamaniya@gmail.com>
# Licensed under the MIT License
#

# Project configuration
PROJECT_NAME := PCIe Simulator Package
VERSION := 1.0.0

# Directories
KERNEL_DIR := kernel
LIB_DIR := lib
EXAMPLES_DIR := examples
SIM_DIR := sim
UTILS_DIR := utils

# Build targets
.PHONY: all kernel lib examples clean install uninstall load unload status help deps check-deps windows

# Default target - build everything
all:
	@echo "Building $(PROJECT_NAME) v$(VERSION)..."
	@echo ""
	@$(MAKE) lib
	@$(MAKE) examples
	@echo ""
	@echo "Build complete. To build kernel module, run: make kernel"

# Build kernel module
kernel:
	@echo "Building kernel module..."
	@$(MAKE) -C $(KERNEL_DIR)

# Build userspace library
lib:
	@echo "Building utilities..."
	@$(MAKE) -C $(UTILS_DIR)
	@echo "Building simulation backends..."
	@$(MAKE) -C $(SIM_DIR)
	@echo "Building userspace library..."
	@$(MAKE) -C $(LIB_DIR)

# Build examples
examples: lib
	@echo "Building examples..."
	@$(MAKE) -C $(EXAMPLES_DIR)

# Clean all components
clean:
	@echo "Cleaning all components..."
	@$(MAKE) -C $(KERNEL_DIR) clean
	@$(MAKE) -C $(UTILS_DIR) clean
	@$(MAKE) -C $(SIM_DIR) clean
	@$(MAKE) -C $(LIB_DIR) clean
	@$(MAKE) -C $(EXAMPLES_DIR) clean
	@echo "Removing output directory..."
	rm -rf out
	@echo "Clean complete"

# Install all components
install:
	@echo "Installing $(PROJECT_NAME)..."
	@$(MAKE) -C $(LIB_DIR) install
	@$(MAKE) -C $(KERNEL_DIR) install
	@echo "Installation complete"

# Uninstall all components
uninstall:
	@echo "Uninstalling $(PROJECT_NAME)..."
	@$(MAKE) -C $(LIB_DIR) uninstall
	@echo "Uninstallation complete"

# Load kernel module
load:
	@$(MAKE) -C $(KERNEL_DIR) load

# Unload kernel module
unload:
	@$(MAKE) -C $(KERNEL_DIR) unload

# Run examples
run-examples: examples load
	@echo "Running examples..."
	@$(MAKE) -C $(EXAMPLES_DIR) run-c
	@$(MAKE) -C $(EXAMPLES_DIR) run-cpp

# Show project status
status:
	@echo "=== $(PROJECT_NAME) v$(VERSION) Status ==="
	@echo ""
	@echo "Kernel Module:"
	@lsmod | grep pcie_sim || echo "  Not loaded"
	@echo ""
	@echo "Device Files:"
	@ls -la /dev/pcie_sim* 2>/dev/null || echo "  None found"
	@echo ""
	@echo "Library Files:"
	@ls -la out/lib/lib*.a out/lib/lib*.so* 2>/dev/null || echo "  Not built"
	@echo ""
	@echo "Example Programs:"
	@ls -la out/examples/basic_test out/examples/cpp_test 2>/dev/null || echo "  Not built"
	@echo "=================================="

# Show project structure
structure:
	@echo "=== $(PROJECT_NAME) Structure ==="
	@echo ""
	@echo "Kernel Module ($(KERNEL_DIR)/):"
	@find $(KERNEL_DIR)/ -name "*.c" -o -name "*.h" | sort | sed 's/^/  /'
	@echo ""
	@echo "Simulation Backends ($(SIM_DIR)/):"
	@find $(SIM_DIR)/ \( -name "*.c" -o -name "*.h" \) | sort | sed 's/^/  /'
	@echo ""
	@echo "Library ($(LIB_DIR)/):"
	@find $(LIB_DIR)/ \( -name "*.c" -o -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) | sort | sed 's/^/  /'
	@echo ""
	@echo "Examples ($(EXAMPLES_DIR)/):"
	@find $(EXAMPLES_DIR)/ -name "*.c" -o -name "*.cpp" | sort | sed 's/^/  /'
	@echo ""
	@echo "Build Files:"
	@find . -maxdepth 2 -name "Makefile" | sort | sed 's/^/  /'
	@echo "================================"

# Install build dependencies
deps:
	@echo "Installing build dependencies..."
	@./scripts/install-deps.sh

# Check if dependencies are installed
check-deps:
	@echo "Checking build dependencies..."
	@./scripts/install-deps.sh --check

# Build Windows version (MinGW/MSYS2)
windows:
	@echo "Building Windows version..."
	@echo ""
	@$(MAKE) -C $(LIB_DIR) -f Makefile.win all
	@$(MAKE) -C $(EXAMPLES_DIR) -f Makefile.win all
	@echo ""
	@echo "Windows build complete!"
	@echo ""
	@echo "Run examples:"
	@echo "  make -C examples -f Makefile.win run-c"
	@echo "  make -C examples -f Makefile.win run-cpp"

# Package for distribution
package: clean
	@echo "Creating distribution package..."
	@tar -czf pcie-simulator-$(VERSION).tar.gz \
		--exclude='.git*' \
		--exclude='*.tar.gz' \
		--transform 's,^\.,pcie-simulator-$(VERSION),' \
		.
	@echo "Package created: pcie-simulator-$(VERSION).tar.gz"

# Help
help:
	@echo "$(PROJECT_NAME) v$(VERSION) - Modular Build System"
	@echo ""
	@echo "This project provides a complete PCIe device simulation package with:"
	@echo "- Kernel module for device simulation"
	@echo "- C/C++ userspace library"
	@echo "- Example applications"
	@echo "- Modular build system"
	@echo ""
	@echo "Quick Start:"
	@echo "  make deps        # Install build dependencies"
	@echo "  make all         # Build userspace components"
	@echo "  make kernel      # Build kernel module"
	@echo "  make load        # Load kernel module"
	@echo "  make run-examples # Run example programs"
	@echo ""
	@echo "Dependency Targets:"
	@echo "  deps             - Install build dependencies"
	@echo "  check-deps       - Check if dependencies are installed"
	@echo ""
	@echo "Main Targets:"
	@echo "  all              - Build userspace library and examples"
	@echo "  kernel           - Build kernel module (Linux only)"
	@echo "  lib              - Build userspace library only"
	@echo "  examples         - Build example programs"
	@echo "  windows          - Build Windows version (MinGW/MSYS2)"
	@echo "  clean            - Clean all build artifacts"
	@echo "  install          - Install library and kernel module"
	@echo "  uninstall        - Remove installed components"
	@echo ""
	@echo "Runtime Targets:"
	@echo "  load             - Load kernel module"
	@echo "  unload           - Unload kernel module"
	@echo "  run-examples     - Run all example programs"
	@echo ""
	@echo "Info Targets:"
	@echo "  status           - Show current status"
	@echo "  structure        - Show project structure"
	@echo "  package          - Create distribution tarball"
	@echo ""
	@echo "Component Makefiles:"
	@echo "  $(KERNEL_DIR)/Makefile     - Kernel module build (Linux)"
	@echo "  $(SIM_DIR)/Makefile        - Simulation backend build (Cross-platform)"
	@echo "  $(LIB_DIR)/Makefile        - Library build (Linux/macOS)"
	@echo "  $(LIB_DIR)/Makefile.win    - Library build (Windows)"
	@echo "  $(EXAMPLES_DIR)/Makefile   - Examples build (Linux/macOS)"
	@echo "  $(EXAMPLES_DIR)/Makefile.win - Examples build (Windows)"
	@echo ""
	@echo "For component-specific help:"
	@echo "  make -C $(KERNEL_DIR) help"
	@echo "  make -C $(SIM_DIR) help"
	@echo "  make -C $(LIB_DIR) help"
	@echo "  make -C $(EXAMPLES_DIR) help"