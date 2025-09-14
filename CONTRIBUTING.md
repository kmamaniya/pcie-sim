# Contributing to PCIe Device Driver Simulator

Thank you for your interest in contributing to this educational project! This guide will help you get started with contributing to the PCIe Device Driver Simulator.

## Project Purpose

This is an educational project designed to teach Linux kernel driver development concepts, specifically PCIe device simulation. The code prioritizes:

- **Educational Value** - Clear, well-commented code for learning
- **Simplicity** - Focused implementation without unnecessary complexity
- **Correctness** - Proper kernel programming patterns and safety
- **Academic Use** - Suitable for coursework and research
- **Cross-Platform** - Works on Linux, WSL2, and Windows

## How to Contribute

### 1. Fork and Clone
```bash
# Fork the repository on GitHub, then:
git clone https://github.com/your-username/pcie-simulator.git
cd pcie-simulator
```

### 2. Create a Feature Branch
```bash
git checkout -b feature/your-improvement-name
```

### 3. Set Up Development Environment
```bash
# Install dependencies automatically
make deps

# Or check what's needed
make check-deps
```

### 4. Build and Test
```bash
# Build userspace components (works everywhere)
make all

# Build kernel module (Linux only, not WSL2)
make kernel

# Test userspace functionality
make run-cpp                    # Single device test
make run-multi                  # 8-device test

# Windows testing (if on Windows with MSYS2)
make windows
make -C examples -f Makefile.win run-c
```

### 5. Make Your Changes
- Follow existing code style and patterns
- Add comments for educational clarity
- Ensure your changes build without warnings (`make clean && make`)
- Test your changes on multiple scenarios

### 6. Test Your Changes

#### Userspace Testing (Works on all platforms):
```bash
# Basic functionality
./examples/basic_test

# Advanced testing with different device counts
./examples/cpp_test --num-devices 1    # Single device
./examples/cpp_test --num-devices 4    # Multi-device
./examples/cpp_test --num-devices 8    # All devices
./examples/cpp_test --help             # Help system

# Performance testing
./examples/cpp_test -d 8               # Short form
```

#### Kernel Module Testing (Native Linux only):
```bash
# Build and load kernel module
make kernel
make load

# Check kernel messages
dmesg | tail -20

# Test with kernel module loaded
./examples/basic_test

# Check device files and proc interface
ls -la /dev/pcie_sim*
cat /proc/pcie_sim*/stats

# Unload when done
make unload
```

#### Cross-Platform Testing:
```bash
# Test on different platforms
make status                     # Check current status
make structure                  # Show project structure

# Windows testing (MSYS2)
make -f lib/Makefile.win all
make -f examples/Makefile.win all
```

### 7. Code Quality Checks
```bash
# Ensure no warnings
make clean
make all 2>&1 | grep -i warning    # Should be empty

# Check build on different components
make -C lib clean && make -C lib
make -C examples clean && make -C examples

# Validate help systems
make help
make -C lib help
make -C examples help
```

### 8. Submit a Pull Request
- Write a clear description of your changes
- Explain the educational benefit or improvement
- Include test results from different platforms if possible
- Reference any issues your PR addresses

## Types of Contributions

### Code Improvements
- **Bug fixes** - Correct any issues found in simulation or kernel code
- **Performance improvements** - Optimize while maintaining educational clarity
- **Additional features** - Extend functionality that enhances learning
- **Cross-platform compatibility** - Improve support for different systems

### Documentation
- **Code comments** - Improve educational explanations
- **README improvements** - Better setup instructions or examples
- **Learning resources** - Add relevant links or tutorials
- **API documentation** - Clarify interfaces and usage patterns

### Testing
- **Test cases** - Add scenarios that exercise different code paths
- **Platform testing** - Verify functionality on different Linux distributions
- **Performance benchmarks** - Add meaningful performance measurements
- **Edge case testing** - Test boundary conditions and error paths

### Educational Enhancements
- **Examples** - Add new example programs demonstrating concepts
- **Tutorials** - Create step-by-step learning materials
- **Code walkthroughs** - Detailed explanations of complex implementations
- **Reference materials** - Links to relevant specifications and documentation

## Coding Standards

### C Code (Kernel and Library)
- **Linux kernel style** for kernel modules (`scripts/checkpatch.pl`)
- **GNU C99** standard for userspace library
- **Consistent naming** using `pcie_sim_` prefix for public APIs
- **Comprehensive error handling** with meaningful error codes
- **Thread safety** using appropriate locking mechanisms

### C++ Code (Wrappers and Examples)
- **Modern C++11** standard minimum
- **RAII patterns** for automatic resource management
- **STL integration** where appropriate
- **Exception safety** with proper cleanup
- **Template usage** for type safety and flexibility

### General Guidelines
- **No compiler warnings** with `-Wall -Wextra`
- **Consistent indentation** (4 spaces, no tabs)
- **Meaningful variable names** that explain purpose
- **Function documentation** explaining parameters and return values
- **Educational comments** explaining "why" not just "what"

## File Organization

### Understanding the Structure
```
pcie-simulator/
├── kernel/                 # Linux kernel module
│   ├── driver.c           # Platform device framework
│   ├── chardev.c          # Character device interface
│   ├── dma.c              # DMA simulation
│   ├── mmio.c             # Memory-mapped I/O
│   ├── ringbuffer.c       # High-performance rings
│   ├── procfs.c           # Proc filesystem interface
│   └── common.h           # Shared definitions
├── sim/                    # Cross-platform simulation backends
│   ├── linux_sim.c        # Linux simulation implementation
│   ├── windows_sim.c      # Windows simulation implementation
│   └── Makefile           # Simulation backend build
├── lib/                    # Cross-platform userspace library
│   ├── core.c             # Main implementation
│   ├── utils.c            # Utility functions
│   ├── device.hpp         # C++ wrapper classes
│   ├── device.cpp         # C++ implementation
│   ├── monitor.hpp        # Performance monitoring
│   ├── api.h              # Public C API
│   └── types.h            # Common data structures
├── examples/              # Test programs and examples
│   ├── basic_test.c       # Simple C API usage
│   ├── cpp_test.cpp       # Advanced C++ with CLI parsing
│   └── Makefile.win       # Windows build support
├── out/                   # Build output (generated)
│   ├── sim/obj/          # Simulation backend objects
│   ├── lib/              # Compiled libraries and objects
│   └── examples/         # Compiled example programs
└── scripts/               # Build and utility scripts
    └── install-deps.sh    # Cross-platform dependency installer
```

### When Adding New Files
- Place kernel code in `kernel/`
- Place simulation backends in `sim/`
- Place library code in `lib/`
- Place examples in `examples/`
- Update relevant Makefiles
- Add appropriate headers and documentation
- Build outputs go automatically to `out/` directory

## Testing Guidelines

### Unit Testing
While this is primarily an educational project, contributions should:
- Test basic functionality (device open/close, transfers)
- Verify error handling paths
- Check resource cleanup
- Validate statistics accuracy

### Integration Testing
- Test kernel module loading/unloading
- Verify userspace-kernel communication
- Check multi-device scenarios
- Test cross-platform compatibility

### Performance Testing
- Measure and report latency improvements
- Benchmark throughput under different loads
- Test memory usage and leak detection
- Verify thread safety under concurrent access

## Documentation Requirements

### Code Documentation
```c
/**
 * Brief description of function purpose
 *
 * @param dev: Device context
 * @param buffer: Data buffer for transfer
 * @param size: Size of transfer in bytes
 * @return: Error code or success
 *
 * Educational note: This function demonstrates proper DMA setup
 * including cache coherency and error handling patterns common
 * in real PCIe device drivers.
 */
static int pcie_sim_setup_dma(struct pcie_sim_device *dev,
                             void *buffer, size_t size);
```

### Commit Messages
```
component: brief description of change

Longer explanation of what this change accomplishes and why
it was necessary. Include educational benefit if applicable.

- Specific change 1
- Specific change 2

Tested on: Ubuntu 22.04, WSL2, Windows 11 (MSYS2)
```

## Educational Focus

Remember that this project serves educational purposes:

### Prioritize Learning Value
- Choose clarity over optimization when there's a trade-off
- Add comments explaining kernel concepts and patterns
- Reference official documentation where appropriate
- Explain design decisions in commit messages

### Maintain Simplicity
- Avoid overly complex abstractions
- Keep the scope focused on PCIe simulation
- Don't add features that obscure core concepts
- Balance realism with educational clarity

### Support Different Skill Levels
- Provide both simple and advanced examples
- Include references to relevant documentation
- Explain prerequisites for understanding concepts
- Make the code accessible to newcomers

## Getting Help

### Resources
- **Project documentation**: See README.md and IMPLEMENTATION_OVERVIEW.md
- **Build help**: `make help` shows all available targets
- **Component help**: Each Makefile has detailed help sections
- **Learning resources**: See README.md learning resources section

### Communication
- **Issues**: Use GitHub issues for bugs and feature requests
- **Discussions**: Use GitHub discussions for questions and ideas
- **Documentation**: Improve documentation for unclear concepts

### Before Submitting
- Read through existing code to understand patterns
- Check that your contribution aligns with educational goals
- Test on multiple platforms if possible
- Consider how your change affects learning value

---

**Thank you for contributing to making Linux kernel driver development more accessible for learning and education!**

## License

By contributing to this project, you agree that your contributions will be licensed under the same MIT License that covers the project.