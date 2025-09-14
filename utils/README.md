# PCIe Simulator - Shared Configuration Utilities

**Directory:** `utils/`
**Purpose:** Cross-platform configuration and utility components shared across all project modules

## Overview

The `utils/` directory contains shared configuration structures, command-line parsing, and CSV logging utilities that are used by kernel module, simulation backends, and application components. This modular design ensures consistent configuration management across all platforms.

## Components

### 🔧 **Configuration Management (`config.h/.c`)**

Cross-platform C structures for configuration management with predefined patterns and validation.

**Key Features:**
- Cross-platform compatibility (C99 standard)
- Predefined transfer patterns (small-fast, large-burst, mixed, custom)
- Error injection scenario definitions
- Configuration validation and parsing functions
- String conversion utilities

**Core Structures:**
```c
typedef struct pcie_sim_test_config {
    uint32_t num_devices;                    // 1-8 devices
    struct pcie_sim_transfer_config transfer; // Pattern configuration
    struct pcie_sim_error_config error;      // Error injection settings
    struct pcie_sim_stress_config stress;    // Stress test parameters
    struct pcie_sim_log_config logging;      // CSV logging settings
    uint32_t flags;                          // Feature enable flags
} pcie_sim_test_config;
```

**Transfer Patterns:**
- `PCIE_SIM_PATTERN_SMALL_FAST`: 64B-1KB @ 10kHz (OTPU-optimized)
- `PCIE_SIM_PATTERN_LARGE_BURST`: 1-4MB @ 100Hz (bulk transfers)
- `PCIE_SIM_PATTERN_MIXED`: 1-64KB @ 1kHz (realistic workload)
- `PCIE_SIM_PATTERN_CUSTOM`: User-defined size and rate

**Error Scenarios:**
- `PCIE_SIM_ERROR_SCENARIO_NONE`: No error injection
- `PCIE_SIM_ERROR_SCENARIO_TIMEOUT`: Transfer timeout simulation (1%)
- `PCIE_SIM_ERROR_SCENARIO_CORRUPTION`: Data corruption simulation (0.5%)
- `PCIE_SIM_ERROR_SCENARIO_OVERRUN`: Buffer overrun simulation (2%)

### 🎛️ **Command-Line Parsing (`options.hpp/.cpp`)**

Modern C++ command-line argument parser with STL container integration and comprehensive validation.

**Key Features:**
- STL-based design using std::map and std::function
- Type-safe parameter validation with lambda functions
- Short and long option support (`-d` and `--num-devices`)
- Automatic help generation with examples
- Configuration structure generation

**Usage Example:**
```cpp
auto options = ProgramOptions::create_otpu_options();
if (!options->parse(argc, argv)) {
    return options->has_option("help") ? 0 : 1;
}
auto config = options->to_config();
```

**Supported Options:**
- `--num-devices, -d`: Number of devices to test (1-8)
- `--pattern, -p`: Transfer pattern selection
- `--size, -s`: Custom transfer size (64-4194304 bytes)
- `--rate, -r`: Custom transfer rate (1-10000 Hz)
- `--error-scenario, -e`: Error injection type
- `--threads, -t`: Concurrent threads (1-64)
- `--duration`: Test duration (1-3600 seconds)
- `--log-csv, -l`: CSV logging filename
- `--verbose, -v`: Enable verbose output

### 📊 **CSV Logging (`csv_logger.hpp/.cpp`)**

High-performance CSV logging system with session management and thread-safe operations.

**Key Features:**
- Thread-safe logging with std::mutex protection
- Microsecond precision timestamps
- Session metadata and configuration tracking
- Batch logging support for high-throughput scenarios
- Automatic file management with RAII

**CSV Format:**
```csv
timestamp,session_time_ms,device_id,transfer_size,latency_us,throughput_mbps,direction,error_status,thread_id
# Session Start: 2025-01-15 14:30:25.123
# Configuration: pattern=mixed,devices=4,size=1024-65536,rate=1000
2025-01-15 14:30:25.124,0,0,4096,125.45,0.26,TO_DEVICE,SUCCESS,12345
```

**Usage Example:**
```cpp
// Session logger with automatic session management
SessionLogger session_logger("results.csv", "pattern=mixed,devices=4");

// Individual transfer logging
session_logger.log_transfer(device_id, transfer_size, latency_us,
                           throughput_mbps, "TO_DEVICE", "SUCCESS", thread_id);
```

## Build System

### Building Utilities

```bash
# Build all utilities
make -C utils all

# Build specific components
make -C utils config.o      # Configuration management
make -C utils options.o     # Command-line parsing
make -C utils csv_logger.o  # CSV logging

# Clean utilities
make -C utils clean
```

### Integration

The utilities are automatically built and linked into the main library:

```bash
# Utilities are included in library build
make lib

# Library contains all utility objects:
# - out/utils/obj/config.o
# - out/utils/obj/options.o
# - out/utils/obj/csv_logger.o
```

### Dependencies

- **C Utilities**: Standard C99 library (string.h, stdlib.h)
- **C++ Utilities**: C++11 standard library (STL containers, chrono, mutex)
- **Cross-platform**: Works on Linux, Windows (MinGW), and macOS

## API Reference

### Configuration Functions

```c
// Initialize configuration with defaults
int pcie_sim_config_init(struct pcie_sim_test_config *config);

// Set predefined transfer pattern
int pcie_sim_config_set_pattern(struct pcie_sim_test_config *config,
                               pcie_sim_pattern_t pattern);

// Set custom transfer parameters
int pcie_sim_config_set_custom_pattern(struct pcie_sim_test_config *config,
                                      uint32_t size, uint32_t rate);

// Set error injection scenario
int pcie_sim_config_set_error_scenario(struct pcie_sim_test_config *config,
                                      pcie_sim_error_scenario_t scenario);

// Validate configuration
int pcie_sim_config_validate(const struct pcie_sim_test_config *config);

// String parsing and conversion
pcie_sim_pattern_t pcie_sim_parse_pattern(const char *pattern_str);
pcie_sim_error_scenario_t pcie_sim_parse_error_scenario(const char *error_str);
const char *pcie_sim_pattern_to_string(pcie_sim_pattern_t pattern);
const char *pcie_sim_error_scenario_to_string(pcie_sim_error_scenario_t scenario);
```

### C++ Options Interface

```cpp
namespace PCIeSimulator {
    class ProgramOptions {
    public:
        // Create OTPU-optimized option set
        static std::unique_ptr<ProgramOptions> create_otpu_options();

        // Parse command-line arguments
        bool parse(int argc, char* argv[]);

        // Convert to configuration structure
        std::unique_ptr<pcie_sim_test_config> to_config() const;

        // Check if option was provided
        bool has_option(const std::string& name) const;

        // Get typed option value
        template<typename T>
        T get(const std::string& name) const;

        // Print help message
        void print_help() const;
    };
}
```

### C++ CSV Logging Interface

```cpp
namespace PCIeSimulator {
    // Low-level CSV logger
    class CSVLogger {
    public:
        CSVLogger(const std::string& filename);

        void log_transfer(uint32_t device_id, uint32_t transfer_size,
                         double latency_us, double throughput_mbps,
                         const std::string& direction,
                         const std::string& error_status,
                         uint32_t thread_id);

        void flush();
        size_t get_record_count() const;
        const std::string& get_filename() const;
    };

    // High-level session logger with automatic management
    class SessionLogger {
    public:
        SessionLogger(const std::string& filename, const std::string& config);
        ~SessionLogger();  // Automatic session end logging

        void log_transfer(uint32_t device_id, uint32_t transfer_size,
                         double latency_us, double throughput_mbps,
                         const std::string& direction,
                         const std::string& error_status,
                         uint32_t thread_id);

        CSVLogger* get_logger() const;
    };
}
```

## Integration Guide

### Using Configuration in Kernel Module

```c
#include "../utils/config.h"

// Kernel module can use configuration constants
if (error_scenario == PCIE_SIM_ERROR_SCENARIO_TIMEOUT) {
    // Inject timeout error
    msleep(100);  // Recovery delay
}
```

### Using Configuration in Simulation Backends

```c
#include "../utils/config.h"

int linux_sim_configure_pattern(pcie_sim_pattern_t pattern) {
    switch (pattern) {
    case PCIE_SIM_PATTERN_SMALL_FAST:
        // Configure for high-frequency, small transfers
        break;
    case PCIE_SIM_PATTERN_LARGE_BURST:
        // Configure for bulk transfers
        break;
    }
}
```

### Using Enhanced Options in Applications

```cpp
#include "../utils/options.hpp"
#include "../utils/csv_logger.hpp"

int main(int argc, char* argv[]) {
    // Parse command-line options
    auto options = ProgramOptions::create_otpu_options();
    if (!options->parse(argc, argv)) {
        return options->has_option("help") ? 0 : 1;
    }

    // Convert to configuration
    auto config = options->to_config();

    // Setup CSV logging if requested
    std::unique_ptr<SessionLogger> logger;
    if (config->flags & PCIE_SIM_CONFIG_ENABLE_LOGGING) {
        logger = std::make_unique<SessionLogger>(
            config->logging.csv_filename, "test_session");
    }

    // Use configuration for testing...
}
```

## Compatibility

### C++ Standard Compliance
- **C++11 Compatible**: Uses `std::unique_ptr<T>(new T())` instead of `std::make_unique`
- **STL Containers**: Leverages std::map, std::vector, std::function
- **Thread Safety**: std::mutex and std::lock_guard
- **Time Handling**: std::chrono for high-precision timestamps

### Cross-Platform Support
- **Linux**: Full compatibility with GCC/Clang
- **Windows**: MinGW-w64 and MSYS2 support
- **macOS**: Clang with C++11 support

### Error Handling
- **C Functions**: Return 0 on success, -1 on error
- **C++ Classes**: Exception-based error handling with std::runtime_error
- **Validation**: Comprehensive parameter validation with detailed error messages

## Testing

### Unit Testing

```bash
# Test configuration parsing
out/examples/cpp_test --pattern small-fast --verbose

# Test CSV logging
out/examples/cpp_test --log-csv test.csv --pattern mixed

# Test error scenarios
out/examples/cpp_test --error-scenario timeout --num-devices 4

# Test stress configuration
out/examples/cpp_test --threads 8 --duration 10
```

### Integration Testing

The utilities are automatically tested as part of the main build:

```bash
# Build system tests integration
make all

# Application tests validate utilities
make -C examples run-cpp
```

## Performance Considerations

### CSV Logging Performance
- **Buffered I/O**: Uses std::ofstream with automatic buffering
- **Thread Safety**: Minimal lock contention with fine-grained locking
- **Memory Efficient**: Direct streaming without intermediate buffers
- **Batch Operations**: Supports batch logging for high-throughput scenarios

### Configuration Performance
- **Compile-time Constants**: Predefined patterns use compile-time initialization
- **String Operations**: Minimal string parsing with efficient comparisons
- **Memory Layout**: Structures designed for cache-friendly access patterns

## Future Enhancements

### Planned Features
- **JSON Configuration**: Support for configuration files
- **Binary Logging**: High-performance binary logging format
- **Plugin System**: Loadable configuration modules
- **Network Configuration**: Remote configuration and monitoring

### Extensibility
- **Custom Patterns**: Framework for user-defined transfer patterns
- **Plugin Architecture**: Support for third-party configuration providers
- **Monitoring Integration**: Hooks for real-time monitoring systems

---

**Shared utilities providing consistent, cross-platform configuration management and high-performance logging for the PCIe Simulator ecosystem.**