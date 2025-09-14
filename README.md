# PCIe Device Driver Simulator with Enhanced Testing Suite

**Author:** Karan Mamaniya (kmamaniya@gmail.com)
**License:** MIT (Academic/Open Source)
**Version:** 1.0.0

## Overview

A complete **PCIe device driver simulation package** with advanced testing capabilities designed for **OTPU performance analysis** and professional driver development. This project provides both kernel-space device simulation and userspace libraries with modern C++ interfaces, comprehensive testing patterns, error injection scenarios, and high-performance CSV logging.

## ✨ New Features - Enhanced Testing Suite

### 🎯 **Transfer Pattern Testing**
- **Small-Fast Pattern**: 64B-1KB transfers @ high frequency (10kHz)
- **Large-Burst Pattern**: 1-4MB transfers @ lower frequency (100Hz)
- **Mixed Workload Pattern**: 1-64KB transfers @ moderate frequency (1kHz)
- **Custom Pattern**: User-defined transfer size and rate
- **Command-line Pattern Selection**: `--pattern small-fast|large-burst|mixed|custom`

### 📊 **High-Performance CSV Logging**
- **Timestamped Logging**: Microsecond precision timestamps
- **Session Metadata**: Configuration tracking and test summaries
- **Real-time Metrics**: Transfer size, latency, throughput, error status
- **Thread Tracking**: Multi-threaded transfer identification
- **Automatic Filenames**: `otpu_test_YYYYMMDD_HHMMSS.csv`

### 🚨 **Scenario-Based Error Injection**
- **Timeout Simulation**: Transfer timeout scenarios (configurable probability)
- **Data Corruption**: Simulated data integrity issues
- **Buffer Overrun**: Memory boundary violation simulation
- **Recovery Modeling**: Realistic error recovery time delays
- **Error Rate Control**: Fine-tuned probability control (0.1% - 10%)

### ⚡ **Multi-threaded Stress Testing**
- **Concurrent Device Access**: 1-64 concurrent threads
- **Configurable Duration**: 1 second to 1 hour test runs
- **Round-robin Device Assignment**: Load balancing across multiple devices
- **Real-time Progress Monitoring**: Per-thread performance tracking
- **Thread Safety Validation**: Concurrent access testing

### 🏗️ **Modular Architecture**
- **Shared Configuration Utilities**: `utils/` directory with cross-platform config
- **Cross-Platform Compatibility**: C++11 standard compliance
- **Component Reusability**: Shared config between kernel, simulation, and applications

## Quick Start - Enhanced Testing

### 1. Build the Enhanced System
```bash
# Install dependencies and build everything
make deps
make all

# Build kernel module (Linux only)
make kernel
make load
```

### 2. Basic Performance Testing
```bash
# Default mixed workload test
out/examples/cpp_test

# Small, fast transfers (OTPU-style)
out/examples/cpp_test --pattern small-fast --num-devices 4

# Large burst transfers with logging
out/examples/cpp_test --pattern large-burst --log-csv results.csv
```

### 3. Error Injection Testing
```bash
# Test timeout scenarios
out/examples/cpp_test --error-scenario timeout --num-devices 8

# Test data corruption with verbose output
out/examples/cpp_test --error-scenario corruption --verbose

# Combined error testing with logging
out/examples/cpp_test --error-scenario overrun --log-csv error_test.csv
```

### 4. Stress Testing
```bash
# Multi-threaded stress test
out/examples/cpp_test --threads 8 --duration 30

# Full stress test with error injection
out/examples/cpp_test --threads 16 --duration 60 --error-scenario timeout

# Comprehensive test (all features)
out/examples/cpp_test --pattern large-burst --threads 4 --duration 10 \
                     --error-scenario corruption --log-csv comprehensive.csv --verbose
```

## Enhanced Command-Line Interface

The new `cpp_test` application provides comprehensive command-line options:

```bash
out/examples/cpp_test [OPTIONS]

Options:
  --num-devices, -d        Number of devices to test (1-8, default: 1)
  --pattern, -p            Transfer pattern: small-fast|large-burst|mixed|custom
  --size, -s              Custom transfer size in bytes (64-4194304)
  --rate, -r              Custom transfer rate in Hz (1-10000)
  --error-scenario, -e     Error injection: timeout|corruption|overrun|none
  --threads, -t            Concurrent threads for stress testing (1-64)
  --duration              Test duration in seconds (1-3600)
  --log-csv, -l           Log results to CSV file
  --verbose, -v           Enable verbose output
  --help, -h              Show help message
```

### Usage Examples

```bash
# OTPU performance baseline
out/examples/cpp_test --pattern small-fast --num-devices 8 --log-csv baseline.csv

# Stress test with error injection
out/examples/cpp_test --threads 16 --error-scenario timeout --duration 120

# Custom pattern testing
out/examples/cpp_test --pattern custom --size 2048 --rate 5000 --verbose

# Comprehensive validation
out/examples/cpp_test --pattern mixed --threads 8 --duration 30 \
                     --error-scenario corruption --log-csv validation.csv
```

## Project Structure (Enhanced)

```
PCIe Simulator Package/
├── utils/                     # 🆕 Shared Configuration Utilities
│   ├── Makefile              # Utility component build
│   ├── config.h/.c           # Cross-platform configuration structures
│   ├── options.hpp/.cpp      # Modern C++ command-line parsing
│   └── csv_logger.hpp/.cpp   # High-performance CSV logging
│
├── kernel/                   # Kernel Module (Enhanced)
│   ├── common.h              # Enhanced with error injection support
│   ├── driver.c              # Platform driver with config integration
│   └── [other kernel files]  # DMA, MMIO, proc, ring buffer, chardev
│
├── sim/                      # Cross-Platform Simulation Backends
│   ├── linux_sim.c           # Linux simulation with config support
│   └── windows_sim.c         # Windows simulation backend
│
├── lib/                      # Enhanced Userspace Library
│   ├── pcie_sim.h            # Main C library header
│   ├── device.hpp/.cpp       # Enhanced C++ device wrapper
│   ├── monitor.hpp           # Performance monitoring tools
│   └── [other lib files]     # API, types, core, utils
│
├── examples/                 # Enhanced Example Applications
│   ├── basic_test.c          # Simple C interface demonstration
│   ├── cpp_test.cpp          # 🆕 Advanced testing suite with all features
│   └── cpp_test_old.cpp      # Original version (preserved)
│
└── out/                      # Build Output (Generated)
    ├── utils/obj/            # 🆕 Utility object files
    ├── lib/libpcie_sim.*     # Enhanced library with utilities
    └── examples/cpp_test     # 🆕 Enhanced test application
```

## CSV Logging Format

The enhanced logging system produces detailed CSV files:

```csv
timestamp,session_time_ms,device_id,transfer_size,latency_us,throughput_mbps,direction,error_status,thread_id
# Session Start: 2025-01-15 14:30:25.123
# Configuration: pattern=mixed,devices=4,size=1024-65536,rate=1000
# Columns: timestamp, session_time_ms, device_id, transfer_size, latency_us, throughput_mbps, direction, error_status, thread_id
2025-01-15 14:30:25.124,0,0,4096,125.45,0.26,TO_DEVICE,SUCCESS,12345
2025-01-15 14:30:25.125,1,1,8192,98.32,0.67,TO_DEVICE,SUCCESS,12346
2025-01-15 14:30:25.126,2,0,2048,156.78,0.10,TO_DEVICE,TIMEOUT,12345
```

## Performance Testing Patterns

### Small-Fast Pattern (OTPU Optimized)
- **Size Range**: 64B - 1KB
- **Frequency**: 10,000 Hz
- **Use Case**: High-frequency, low-latency operations
- **Typical OTPU Workload**: Small tensor operations, control messages

### Large-Burst Pattern
- **Size Range**: 1MB - 4MB
- **Frequency**: 100 Hz
- **Use Case**: Bulk data transfers, model loading
- **Burst Configuration**: 10 transfers per burst, 100ms intervals

### Mixed Workload Pattern
- **Size Range**: 1KB - 64KB
- **Frequency**: 1,000 Hz
- **Use Case**: Realistic mixed workload simulation
- **Variability**: Random size distribution within range

### Custom Pattern
- **User-Defined**: Any size (64B - 4MB) and rate (1-10,000 Hz)
- **Use Case**: Specific benchmark scenarios
- **Configuration**: `--pattern custom --size 2048 --rate 5000`

## Error Injection Scenarios

### Timeout Simulation
- **Default Rate**: 1% of transfers
- **Recovery Time**: 100ms delay
- **Use Case**: Network timeout, device busy scenarios

### Data Corruption Simulation
- **Default Rate**: 0.5% of transfers
- **Recovery Time**: 50ms delay
- **Use Case**: Data integrity validation, ECC testing

### Buffer Overrun Simulation
- **Default Rate**: 2% of transfers
- **Recovery Time**: 200ms delay
- **Use Case**: Memory boundary testing, overflow detection

## Build Targets (Enhanced)

### Main Targets
```bash
make all         # Build userspace library, utilities, and examples
make kernel      # Build kernel module (Linux only)
make utils       # Build shared configuration utilities
make clean       # Clean all build artifacts including utils
```

### Testing Targets
```bash
make run-examples            # Run both basic and enhanced examples
make -C examples run-cpp     # Run enhanced C++ test suite
make -C examples test-patterns    # Test all transfer patterns
make -C examples test-errors      # Test all error scenarios
make -C examples test-stress      # Run stress tests
```

### Component Targets
```bash
make -C utils all           # Build configuration utilities
make -C utils clean         # Clean utility components
make -C lib static          # Build library with embedded utilities
make -C lib shared          # Build shared library with utilities
```

## API Documentation (Enhanced)

### Enhanced C++ Interface

```cpp
#include "device.hpp"
#include "monitor.hpp"
#include "../utils/options.hpp"
#include "../utils/csv_logger.hpp"

using namespace PCIeSimulator;

// Enhanced configuration management
auto options = ProgramOptions::create_otpu_options();
options->parse(argc, argv);
auto config = options->to_config();

// CSV logging with session management
SessionLogger logger("test_results.csv", "pattern=mixed,devices=4");

// Enhanced error injection
ErrorInjector injector(PCIE_SIM_ERROR_SCENARIO_TIMEOUT, 0.01f);

// Multi-device management
for (uint32_t device_id = 0; device_id < config->num_devices; ++device_id) {
    auto device = DeviceManager::open_device(device_id);

    // Pattern-based transfers
    pattern_based_transfer_test(device_id, config->transfer, &injector);
}

// Multi-threaded stress testing
std::vector<std::future<void>> stress_threads;
for (uint32_t i = 0; i < config->stress.num_threads; ++i) {
    stress_threads.push_back(std::async(std::launch::async,
        stress_test_worker, device_id, i, config->stress.duration_seconds));
}
```

### Configuration Structure

```c
// Cross-platform configuration (utils/config.h)
typedef struct {
    uint32_t num_devices;                    // 1-8 devices
    struct pcie_sim_transfer_config transfer; // Pattern configuration
    struct pcie_sim_error_config error;      // Error injection settings
    struct pcie_sim_stress_config stress;    // Stress test parameters
    struct pcie_sim_log_config logging;      // CSV logging settings
    uint32_t flags;                          // Feature flags
} pcie_sim_test_config;

// Transfer pattern configuration
typedef struct {
    pcie_sim_pattern_t pattern;              // SMALL_FAST, LARGE_BURST, etc.
    uint32_t min_size, max_size;            // Size range in bytes
    uint32_t rate_hz;                       // Transfer frequency
    uint32_t burst_count;                   // Transfers per burst
    uint32_t burst_interval_ms;             // Time between bursts
} pcie_sim_transfer_config;

// Error injection configuration
typedef struct {
    pcie_sim_error_scenario_t scenario;     // Error type
    float probability;                      // Error rate (0.0-1.0)
    uint32_t recovery_time_ms;             // Recovery delay
    uint32_t flags;                        // Error flags
} pcie_sim_error_config;
```

## Compatibility (Enhanced)

### Enhanced Features Support
- ✅ **Linux**: Full kernel module + userspace with all features
- ✅ **Windows**: Userspace simulation with all testing features
- ✅ **macOS**: Userspace simulation with cross-platform utilities
- ✅ **Cross-platform**: C++11 compliant utilities and enhanced API

### Compiler Requirements
- **C++11 Standard**: Required for enhanced features
- **GCC 4.8+** or **Clang 3.8+**: Full compatibility
- **C99 Standard**: For kernel module and C utilities

## Educational Value (Enhanced)

### Advanced Testing Methodologies
- **Performance Pattern Analysis**: Understanding different workload characteristics
- **Error Injection Techniques**: Systematic fault tolerance testing
- **Stress Testing Design**: Multi-threaded concurrent access patterns
- **Data Collection and Analysis**: CSV-based performance metrics

### Modern C++ Practices
- **STL Container Integration**: Modern command-line parsing with std::map
- **RAII Resource Management**: Exception-safe device handling
- **Template-based Design**: Type-safe configuration management
- **Functional Programming**: Lambda-based validation and callbacks

### Cross-platform Development
- **Modular Configuration System**: Shared utilities across platforms
- **Build System Design**: Component-based Makefile architecture
- **Platform Abstraction**: Unified API with platform-specific backends

## Learning Path (Enhanced)

### For OTPU Development
1. **Start with Pattern Testing**: `--pattern small-fast` for OTPU-style workloads
2. **Error Resilience**: Test with `--error-scenario timeout` for network issues
3. **Concurrency Validation**: Use `--threads` for multi-device scenarios
4. **Performance Analysis**: CSV logging for throughput optimization

### For Driver Development
1. **Basic API**: Start with `basic_test.c` for simple operations
2. **Advanced Features**: Study `cpp_test.cpp` for comprehensive testing
3. **Kernel Integration**: Examine kernel module for real driver patterns
4. **Cross-platform**: Understand simulation backends for portability

## Support (Enhanced)

- **Enhanced Documentation**: Component-specific help with `make -C <component> help`
- **Comprehensive Examples**: Multiple testing scenarios in `examples/`
- **CSV Analysis**: Import logs into spreadsheet tools for analysis
- **Build Troubleshooting**: `make check-deps` for dependency validation
- **Real-time Monitoring**: `/proc/pcie_sim*/stats` on Linux

---

**Professional PCIe device simulation with advanced testing capabilities for OTPU performance analysis and driver development education.**