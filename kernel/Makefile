#
# PCIe Simulator - Kernel Module Makefile
#
# Copyright (c) 2025 Karan Mamaniya <kmamaniya@gmail.com>
# Licensed under the MIT License
#

# Kernel module configuration
KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# Module name and sources
MODULE_NAME := pcie_sim
obj-m := $(MODULE_NAME).o

# Kernel module object files
$(MODULE_NAME)-objs := driver.o \
                       chardev.o \
                       dma.o \
                       procfs.o \
                       mmio.o \
                       ringbuffer.o

# Build targets
.PHONY: all clean help

all:
	@echo "Building kernel module..."
	@if [ ! -d "$(KERNEL_DIR)" ]; then \
		echo "❌ Error: Kernel headers not found at $(KERNEL_DIR)"; \
		echo ""; \
		if grep -q microsoft /proc/version 2>/dev/null; then \
			echo "🔍 WSL Environment Detected"; \
			echo ""; \
			echo "WSL2 uses a Microsoft kernel that doesn't support custom kernel modules."; \
			echo ""; \
			echo "Solutions:"; \
			echo "1. Use userspace library only:"; \
			echo "   make all     # Build userspace components"; \
			echo ""; \
			echo "2. Test on a native Linux system:"; \
			echo "   - Ubuntu/Debian VM"; \
			echo "   - Native Linux installation"; \
			echo "   - Docker with --privileged mode"; \
			echo ""; \
			echo "3. Use Windows version (if on Windows):"; \
			echo "   make windows # Build Windows version"; \
		else \
			echo "Install kernel headers:"; \
			echo "  Ubuntu/Debian: sudo apt install linux-headers-$$(uname -r)"; \
			echo "  RHEL/CentOS:   sudo yum install kernel-devel"; \
			echo "  Fedora:        sudo dnf install kernel-devel"; \
			echo "  Arch:          sudo pacman -S linux-headers"; \
		fi; \
		echo ""; \
		exit 1; \
	fi
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules
	@echo "Kernel module built: $(MODULE_NAME).ko"

clean:
	@echo "Cleaning kernel module..."
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean 2>/dev/null || true
	rm -f *.o *.ko *.mod.c *.mod *.symvers *.order
	find . -name ".*.cmd" -delete

install: all
	@echo "Installing kernel module..."
	sudo mkdir -p /lib/modules/$(shell uname -r)/extra
	sudo cp $(MODULE_NAME).ko /lib/modules/$(shell uname -r)/extra/
	sudo depmod -a
	@echo "Module installed"

load: all
	@echo "Loading kernel module..."
	@if lsmod | grep -q "^$(MODULE_NAME) "; then \
		echo "Module already loaded"; \
	else \
		sudo insmod $(MODULE_NAME).ko && echo "Module loaded"; \
	fi

unload:
	@echo "Unloading kernel module..."
	@if lsmod | grep -q "^$(MODULE_NAME) "; then \
		sudo rmmod $(MODULE_NAME) && echo "Module unloaded"; \
	else \
		echo "Module not loaded"; \
	fi

help:
	@echo "PCIe Simulator Kernel Module"
	@echo ""
	@echo "Targets:"
	@echo "  all      - Build kernel module"
	@echo "  clean    - Clean build artifacts"
	@echo "  install  - Install module to system"
	@echo "  load     - Load module into kernel"
	@echo "  unload   - Unload module from kernel"
	@echo ""
	@echo "Files:"
	@echo "  driver.c      - Main platform driver"
	@echo "  chardev.c     - Character device interface"
	@echo "  dma.c         - DMA simulation"
	@echo "  procfs.c      - Proc filesystem interface"
	@echo "  mmio.c        - Memory-mapped I/O"
	@echo "  ringbuffer.c  - Ring buffer protocol"
	@echo "  common.h      - Shared definitions"