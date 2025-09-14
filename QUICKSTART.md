# Quick Start Guide

Get up and running with PCIe Simulator in 5 minutes!

## 1. Install Dependencies (30 seconds)

**Automatic installation (recommended):**
```bash
make deps
```

**Manual check:**
```bash
make check-deps
```

**Supported systems:** Ubuntu, Debian, RHEL, CentOS, Fedora, Arch Linux, Alpine Linux, macOS

## 2. Build Everything (1 minute)

```bash
# Build userspace (library + examples)
make all

# Build kernel module (Linux only)
make kernel
```

## 3. Run Examples (30 seconds)

```bash
# Load kernel module and run tests
make load
make run-examples

# Or run individually
sudo ./examples/basic_test    # C interface
sudo ./examples/cpp_test      # C++ interface
```

## 4. Check Status (10 seconds)

```bash
# View system status
make status

# View real-time statistics
cat /proc/pcie_sim0/stats
```

## 5. Clean Up (10 seconds)

```bash
# Unload module
make unload

# Clean build files
make clean
```

## Troubleshooting

**Q: Dependencies missing?**
```bash
make deps  # Auto-install for your system
```

**Q: Kernel headers not found?**
```bash
# Ubuntu/Debian:
sudo apt install linux-headers-$(uname -r)

# RHEL/CentOS:
sudo yum install kernel-devel kernel-headers

# Fedora:
sudo dnf install kernel-devel kernel-headers
```

**Q: Can't load module?**
```bash
# Check if already loaded
lsmod | grep pcie_sim

# Unload if needed
make unload

# Try loading again
make load
```

**Q: Permission denied?**
```bash
# Examples need sudo (device access)
sudo ./examples/basic_test
sudo make run-examples
```

## What's Next?

- **API Documentation**: See [README.md](README.md#api-documentation)
- **Component Help**: `make -C lib help`, `make -C kernel help`
- **Project Structure**: `make structure`
- **Advanced Features**: Explore C++ monitoring classes

## One-Liner Setup

```bash
git clone <repo> && cd pcie-simulator && make deps && make all kernel && make load && make run-examples
```

**That's it! Happy coding! 🚀**