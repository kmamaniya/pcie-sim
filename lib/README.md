# PCIe Simulator - Userspace Library

**Directory:** `lib/`
**Purpose:** Cross-platform userspace library providing C and C++ APIs with enhanced configuration integration

## Overview

The userspace library provides dual API interfaces (C and C++) for PCIe device simulation with seamless integration of shared configuration utilities. The library supports both Linux kernel module communication and cross-platform simulation backends.

## Library Components

### 📚 **Dual API Architecture**

The library provides two complementary interfaces designed for different use cases:

**C Library (`api.h`, `core.c`, `utils.c`):**
- Traditional handle-based interface
- Cross-platform compatibility
- Direct kernel module communication (Linux)
- Simulation backend integration (Windows/macOS)

**C++ Library (`device.hpp`, `device.cpp`):**
- Modern RAII-based resource management
- STL container integration
- Exception-safe operations
- Enhanced performance monitoring

### 🔧 **Core C Library**

#### Main Header (`pcie_sim.h`)
Unified header that includes all necessary C library components:

```c
#include "pcie_sim.h"

// Provides access to:
// - API function declarations
// - Type definitions
// - Error codes
// - Utility functions
```

#### API Interface (`api.h`)
Core API function declarations with enhanced configuration support:

```c
// Device management
pcie_sim_error_t pcie_sim_open(int device_id, pcie_sim_handle_t *handle);
pcie_sim_error_t pcie_sim_close(pcie_sim_handle_t handle);

// Enhanced transfer operations
pcie_sim_error_t pcie_sim_transfer(pcie_sim_handle_t handle,
                                  void *buffer, size_t size,
                                  pcie_sim_direction_t direction,
                                  uint64_t *latency_ns);

pcie_sim_error_t pcie_sim_transfer_async(pcie_sim_handle_t handle,
                                        void *buffer, size_t size,
                                        pcie_sim_direction_t direction,
                                        pcie_sim_callback_t callback,
                                        void *user_data);

// Statistics and monitoring
pcie_sim_error_t pcie_sim_get_stats(pcie_sim_handle_t handle,
                                   struct pcie_sim_stats *stats);
pcie_sim_error_t pcie_sim_reset_stats(pcie_sim_handle_t handle);

// Enhanced configuration integration
pcie_sim_error_t pcie_sim_configure_errors(pcie_sim_handle_t handle,
                                          const struct pcie_sim_error_config *config);
```

#### Type Definitions (`types.h`)
Enhanced type system with cross-platform compatibility:

```c
// Handle type (opaque)
typedef struct pcie_sim_handle* pcie_sim_handle_t;

// Error codes (enhanced set)
typedef enum {
    PCIE_SIM_SUCCESS = 0,
    PCIE_SIM_ERROR_DEVICE = -1,      // Device access error
    PCIE_SIM_ERROR_PARAM = -2,       // Invalid parameter
    PCIE_SIM_ERROR_MEMORY = -3,      // Memory allocation error
    PCIE_SIM_ERROR_TIMEOUT = -4,     // Transfer timeout
    PCIE_SIM_ERROR_SYSTEM = -5,      // System error
    PCIE_SIM_ERROR_CONFIG = -6       // Configuration error (new)
} pcie_sim_error_t;

// Transfer directions
typedef enum {
    PCIE_SIM_TO_DEVICE = 0,
    PCIE_SIM_FROM_DEVICE = 1
} pcie_sim_direction_t;

// Enhanced statistics structure
struct pcie_sim_stats {
    uint64_t total_transfers;
    uint64_t total_bytes;
    uint64_t total_errors;           // Enhanced error tracking
    uint64_t timeout_errors;         // Per-type error counts
    uint64_t corruption_errors;
    uint64_t overrun_errors;
    uint64_t avg_latency_ns;
    uint64_t min_latency_ns;
    uint64_t max_latency_ns;
    double throughput_mbps;          // Real-time throughput
};
```

#### Core Implementation (`core.c`)
Cross-platform API coordination with enhanced simulation backend integration:

**Key Features:**
- **Automatic Backend Selection**: Kernel module (Linux) vs simulation (Windows/macOS)
- **Enhanced Error Handling**: Comprehensive error code mapping
- **Configuration Integration**: Seamless utils/ integration
- **Statistics Aggregation**: Cross-backend statistics normalization

**Backend Selection Logic:**
```c
static pcie_sim_error_t select_backend(void) {
    // Linux: Try kernel module first, fallback to simulation
    if (access("/dev/pcie_sim0", R_OK | W_OK) == 0) {
        return use_kernel_backend();
    }

    // Windows/macOS: Use simulation backend
    return use_simulation_backend();
}
```

#### Utility Functions (`utils.c`)
Enhanced utility functions with better error reporting:

```c
// Enhanced error string conversion
const char* pcie_sim_error_string(pcie_sim_error_t error);

// Configuration validation (integrated with utils/config.h)
pcie_sim_error_t pcie_sim_validate_config(const struct pcie_sim_test_config *config);

// Platform detection
const char* pcie_sim_get_platform_info(void);

// Backend information
const char* pcie_sim_get_backend_name(void);
```

### 🎯 **Enhanced C++ Library**

#### Device Interface (`device.hpp`)
Modern C++ wrapper with RAII and STL integration:

```cpp
namespace PCIeSimulator {
    // RAII device wrapper
    class Device {
    public:
        Device(int device_id);
        ~Device();

        // Move semantics (C++11 compatible)
        Device(Device&& other) noexcept;
        Device& operator=(Device&& other) noexcept;

        // STL container support
        template<typename T>
        uint64_t write(const std::vector<T>& data);

        template<typename T>
        uint64_t read(std::vector<T>& data, size_t size);

        // Enhanced operations
        uint64_t write(const void* buffer, size_t size);
        uint64_t read(void* buffer, size_t size);

        // Statistics and monitoring
        Statistics get_statistics() const;
        void reset_statistics();

        // Configuration integration
        void configure_errors(const pcie_sim_error_config& config);

        // Device information
        int get_device_id() const;
        std::string get_backend_name() const;

    private:
        class Impl;  // PIMPL idiom
        std::unique_ptr<Impl> pimpl_;
    };

    // Device manager for multiple devices
    class DeviceManager {
    public:
        static std::unique_ptr<Device> open_device(int device_id);
        static std::vector<int> list_devices();
        static bool is_device_available(int device_id);
    };

    // Enhanced statistics class
    class Statistics {
    public:
        uint64_t total_transfers() const;
        uint64_t total_bytes() const;
        uint64_t total_errors() const;

        // Enhanced error breakdown
        uint64_t timeout_errors() const;
        uint64_t corruption_errors() const;
        uint64_t overrun_errors() const;

        double avg_latency_ns() const;
        double throughput_mbps() const;

        // Statistical analysis
        std::pair<uint64_t, uint64_t> latency_range() const;  // min, max
        double error_rate() const;

        void print(std::ostream& os = std::cout) const;
    };
}
```

#### Performance Monitoring (`monitor.hpp`)
Advanced monitoring and benchmarking tools integrated with shared utilities:

```cpp
namespace PCIeSimulator {
    // Real-time performance monitor
    class PerformanceMonitor {
    public:
        PerformanceMonitor(Device& device);

        void start_monitoring();
        void stop_monitoring();

        // Callback-based monitoring
        void set_callback(std::function<void(const Statistics&)> callback);
        void set_interval(std::chrono::milliseconds interval);

        // Current statistics
        Statistics get_current_stats() const;

    private:
        Device& device_;
        std::atomic<bool> monitoring_;
        std::thread monitor_thread_;
        std::function<void(const Statistics&)> callback_;
        std::chrono::milliseconds interval_{1000};
    };

    // Benchmark runner with pattern integration
    class BenchmarkRunner {
    public:
        BenchmarkRunner(Device& device);

        // Pattern-based benchmarks (integrated with utils/config.h)
        BenchmarkResults run_pattern_benchmark(pcie_sim_pattern_t pattern);
        BenchmarkResults run_custom_benchmark(size_t transfer_size,
                                            size_t num_transfers);

        // Error injection benchmarks
        BenchmarkResults run_error_benchmark(pcie_sim_error_scenario_t scenario,
                                           float error_rate);

        // Comprehensive benchmark suite
        BenchmarkResults run_comprehensive_benchmark();

    private:
        Device& device_;
    };

    // Benchmark results with analysis
    class BenchmarkResults {
    public:
        double avg_latency_us() const;
        double peak_throughput_mbps() const;
        double sustained_throughput_mbps() const;

        // Statistical analysis
        std::vector<double> latency_percentiles(
            const std::vector<double>& percentiles) const;

        // Error analysis
        double error_rate() const;
        std::map<std::string, uint64_t> error_breakdown() const;

        // Export results
        void export_csv(const std::string& filename) const;
        void print(std::ostream& os = std::cout) const;
    };
}
```

### 🔗 **Enhanced Integration Features**

#### Configuration Integration
Seamless integration with the shared `utils/` configuration system:

```cpp
// Direct configuration object support
#include "../utils/config.h"

// Apply configuration from command-line parsing
auto config = options->to_config();
device->configure_errors(config->error);

// Pattern-based testing integration
BenchmarkRunner benchmark(*device);
auto results = benchmark.run_pattern_benchmark(config->transfer.pattern);
```

#### Simulation Backend Integration
Enhanced simulation backend support with cross-platform compatibility:

**Linux Simulation (`../sim/linux_sim.c`):**
- Uses shared configuration structures
- Realistic timing simulation
- Enhanced error injection scenarios
- Memory-mapped I/O simulation

**Windows Simulation (`../sim/windows_sim.c`):**
- Windows-specific timing using QueryPerformanceCounter
- Thread-safe operations with Windows mutexes
- Compatible error injection scenarios
- Cross-platform configuration support

#### CSV Logging Integration
Direct integration with the enhanced CSV logging system:

```cpp
#include "../utils/csv_logger.hpp"

// Automatic logging integration
class Device {
    void enable_logging(const std::string& filename);
    void disable_logging();

private:
    std::unique_ptr<CSVLogger> logger_;
};

// Usage
device->enable_logging("performance_test.csv");
// All transfers automatically logged
```

## Build System Integration

### Enhanced Library Build

The library now includes shared utilities and supports both static and shared builds:

```bash
# Build enhanced library with utilities
make -C lib all

# Generated files:
# - out/lib/libpcie_sim.a     (static library with embedded utilities)
# - out/lib/libpcie_sim.so    (shared library with utilities)

# Build components:
# - lib/core.c, lib/utils.c, lib/device.cpp  (library code)
# - utils/config.c, utils/options.cpp, utils/csv_logger.cpp  (utilities)
# - sim/linux_sim.c or sim/windows_sim.c  (simulation backend)
```

### Cross-Platform Build Support

**Linux/macOS:**
```bash
make -C lib static      # Static library
make -C lib shared      # Shared library
make -C lib install     # System-wide installation
```

**Windows (MinGW):**
```bash
make -C lib -f Makefile.win all
# Builds Windows-compatible library with simulation backend
# Output: libpcie_sim.a, libpcie_sim.dll
```

### Library Linking

**Static Linking (Recommended):**
```bash
gcc -o myapp myapp.c out/lib/libpcie_sim.a -lpthread
g++ -o myapp myapp.cpp out/lib/libpcie_sim.a -lpthread
```

**Shared Linking:**
```bash
gcc -o myapp myapp.c -L out/lib -lpcie_sim -lpthread
# Requires: export LD_LIBRARY_PATH=out/lib:$LD_LIBRARY_PATH
```

## API Usage Examples

### Enhanced C API Usage

```c
#include "pcie_sim.h"
#include "../utils/config.h"

int main() {
    pcie_sim_handle_t handle;
    pcie_sim_error_t ret;

    // Open device
    ret = pcie_sim_open(0, &handle);
    if (ret != PCIE_SIM_SUCCESS) {
        fprintf(stderr, "Failed to open device: %s\n",
                pcie_sim_error_string(ret));
        return 1;
    }

    // Configure error injection
    struct pcie_sim_error_config error_config = {
        .scenario = PCIE_SIM_ERROR_SCENARIO_TIMEOUT,
        .probability = 0.01f,  // 1%
        .recovery_time_ms = 100,
        .flags = 0
    };
    ret = pcie_sim_configure_errors(handle, &error_config);

    // Perform transfers with enhanced error handling
    uint8_t data[4096];
    uint64_t latency_ns;

    for (int i = 0; i < 100; ++i) {
        ret = pcie_sim_transfer(handle, data, sizeof(data),
                               PCIE_SIM_TO_DEVICE, &latency_ns);
        if (ret != PCIE_SIM_SUCCESS) {
            printf("Transfer %d failed: %s (latency: %lu ns)\n",
                   i, pcie_sim_error_string(ret), latency_ns);
        } else {
            printf("Transfer %d success: %lu ns\n", i, latency_ns);
        }
    }

    // Get enhanced statistics
    struct pcie_sim_stats stats;
    ret = pcie_sim_get_stats(handle, &stats);
    if (ret == PCIE_SIM_SUCCESS) {
        printf("Statistics:\n");
        printf("  Total transfers: %lu\n", stats.total_transfers);
        printf("  Total errors: %lu\n", stats.total_errors);
        printf("  Timeout errors: %lu\n", stats.timeout_errors);
        printf("  Average latency: %lu ns\n", stats.avg_latency_ns);
        printf("  Throughput: %.2f Mbps\n", stats.throughput_mbps);
        printf("  Error rate: %.2f%%\n",
               (stats.total_errors * 100.0) / stats.total_transfers);
    }

    pcie_sim_close(handle);
    return 0;
}
```

### Enhanced C++ API Usage

```cpp
#include "device.hpp"
#include "monitor.hpp"
#include "../utils/options.hpp"
#include "../utils/csv_logger.hpp"

using namespace PCIeSimulator;

int main(int argc, char* argv[]) {
    try {
        // Parse command-line options (enhanced integration)
        auto options = ProgramOptions::create_otpu_options();
        if (!options->parse(argc, argv)) {
            return options->has_option("help") ? 0 : 1;
        }
        auto config = options->to_config();

        // Open device with RAII
        auto device = DeviceManager::open_device(0);

        // Configure error injection from parsed config
        device->configure_errors(config->error);

        // Enable CSV logging if requested
        std::unique_ptr<SessionLogger> session_logger;
        if (config->flags & PCIE_SIM_CONFIG_ENABLE_LOGGING) {
            session_logger = std::unique_ptr<SessionLogger>(
                new SessionLogger(config->logging.csv_filename,
                                "pattern=" + std::string(pcie_sim_pattern_to_string(config->transfer.pattern))));
            device->enable_logging(config->logging.csv_filename);
        }

        // Pattern-based testing
        BenchmarkRunner benchmark(*device);
        auto results = benchmark.run_pattern_benchmark(config->transfer.pattern);

        // Real-time monitoring
        PerformanceMonitor monitor(*device);
        monitor.set_callback([](const Statistics& stats) {
            std::cout << "Real-time: " << stats.throughput_mbps()
                      << " Mbps, errors: " << stats.total_errors() << std::endl;
        });
        monitor.start_monitoring();

        // STL container integration
        std::vector<uint32_t> test_data(1024);
        std::iota(test_data.begin(), test_data.end(), 0);

        for (int i = 0; i < 100; ++i) {
            try {
                uint64_t latency = device->write(test_data);
                if (config->flags & PCIE_SIM_CONFIG_VERBOSE) {
                    std::cout << "Transfer " << i << ": " << latency << " ns" << std::endl;
                }
            } catch (const DeviceError& e) {
                std::cerr << "Transfer " << i << " failed: " << e.what() << std::endl;
            }
        }

        monitor.stop_monitoring();

        // Enhanced statistics
        auto final_stats = device->get_statistics();
        final_stats.print();

        // Export results
        results.export_csv("benchmark_results.csv");
        results.print();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

## Performance Characteristics

### Latency Performance

**Kernel Module (Linux):**
- Base latency: 50-150 μs (realistic PCIe simulation)
- Error injection overhead: +50-200 μs (scenario dependent)
- Variance: ±5-10% (realistic jitter simulation)

**Simulation Backend (Cross-platform):**
- Base latency: 80-200 μs (software simulation)
- Error injection overhead: +50-200 μs (consistent with kernel)
- Variance: ±10-15% (software timing variance)

### Throughput Performance

**Optimized Configurations:**
- **Small-Fast Pattern**: 2-8 Gbps (high frequency, small transfers)
- **Large-Burst Pattern**: 4-12 Gbps (bulk transfers, low frequency)
- **Mixed Pattern**: 1-4 Gbps (realistic mixed workload)

**Error Impact:**
- **Timeout Errors**: 10-30% throughput reduction (recovery delays)
- **Corruption Errors**: 5-15% throughput reduction (retransmission simulation)
- **Overrun Errors**: 15-40% throughput reduction (buffer management overhead)

### Memory Efficiency

**C Library:**
- Handle overhead: ~256 bytes per device
- Transfer overhead: ~64 bytes per operation
- Statistics storage: ~128 bytes per device

**C++ Library:**
- Object overhead: ~512 bytes per Device instance (RAII management)
- STL integration: Template-based, zero-copy where possible
- PIMPL overhead: Additional indirection for ABI stability

## Integration Testing

### Comprehensive Test Suite

```bash
# Basic functionality tests
out/examples/basic_test          # C API validation
out/examples/cpp_test           # C++ API validation

# Configuration integration tests
out/examples/cpp_test --pattern small-fast --verbose
out/examples/cpp_test --error-scenario timeout --log-csv test.csv

# Performance validation
out/examples/cpp_test --threads 4 --duration 30 --pattern mixed

# Cross-platform testing (Windows)
out/examples/cpp_test.exe --help  # Windows simulation backend
```

### Automated Validation

```bash
#!/bin/bash
# library_test_suite.sh

echo "Running library integration tests..."

# C API tests
echo "Testing C API..."
out/examples/basic_test || exit 1

# C++ API tests with configuration
echo "Testing C++ API with enhanced features..."
out/examples/cpp_test --pattern small-fast --duration 5 || exit 1
out/examples/cpp_test --error-scenario timeout --duration 5 || exit 1
out/examples/cpp_test --threads 2 --duration 3 || exit 1

# CSV logging tests
echo "Testing CSV logging integration..."
out/examples/cpp_test --log-csv test_output.csv --pattern mixed --duration 5 || exit 1
[ -f test_output.csv ] || exit 1
[ -s test_output.csv ] || exit 1  # File not empty
rm -f test_output.csv

# Backend validation
echo "Testing backend selection..."
BACKEND=$(out/examples/cpp_test --help 2>&1 | grep -o "Backend: [A-Za-z]*")
echo "Using backend: $BACKEND"

echo "All library tests passed!"
```

## Future Enhancements

### Planned Features

**Advanced API Features:**
- Asynchronous transfer completion with futures/promises
- Batch transfer operations for improved efficiency
- Custom callback integration for real-time processing

**Enhanced Monitoring:**
- Histogram-based latency analysis
- Real-time performance profiling with configurable triggers
- Integration with system monitoring tools (Prometheus, Grafana)

**Configuration Extensions:**
- JSON-based configuration file support
- Runtime configuration updates without restart
- Plugin-based configuration providers

### Compatibility Roadmap

**C++ Standard Evolution:**
- C++14/17/20 feature adoption while maintaining C++11 baseline
- Modern async/await patterns for transfer operations
- Concepts-based template constraints (C++20)

**Platform Extensions:**
- macOS native backend development
- ARM64 optimization and testing
- Container deployment optimization

---

**Professional userspace library providing comprehensive PCIe device simulation with enhanced configuration integration and cross-platform compatibility for advanced testing and development.**