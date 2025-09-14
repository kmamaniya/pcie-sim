# PCIe Simulator - Cross-Platform Simulation Backends

**Directory:** `sim/`
**Purpose:** Platform-specific simulation backends providing realistic PCIe device behavior without kernel modules

## Overview

The simulation backends provide cross-platform PCIe device simulation for environments where kernel modules are not available or desired. These backends implement realistic timing, error injection, and transfer simulation while maintaining API compatibility with the kernel module approach.

## Simulation Backends

### 🐧 **Linux Simulation Backend (`linux_sim.c`)**

Pure userspace simulation for Linux environments, providing an alternative to the kernel module for development and testing.

**Key Features:**
- **Userspace Implementation**: No root privileges or kernel module required
- **Realistic Timing**: Uses `clock_gettime()` with nanosecond precision
- **Enhanced Error Injection**: Full support for timeout, corruption, and overrun scenarios
- **Memory Management**: Safe memory operations with bounds checking
- **Thread Safety**: Mutex-protected operations for concurrent access
- **Configuration Integration**: Direct integration with shared utils/ configuration

**Use Cases:**
- Development environments without kernel module access
- Containerized applications and CI/CD pipelines
- User-space testing and validation
- Cross-compilation testing
- Educational environments with restricted privileges

**Implementation Details:**
```c
#include "../utils/config.h"

// Enhanced device state with error injection
struct linux_sim_device {
    int device_id;
    bool is_open;
    pthread_mutex_t mutex;

    // Simulation state
    struct timespec last_access;
    uint64_t transfer_count;
    uint64_t total_bytes;

    // Enhanced error injection
    pcie_sim_error_scenario_t error_scenario;
    float error_probability;
    uint32_t error_recovery_time_ms;

    // Error statistics
    uint64_t timeout_errors;
    uint64_t corruption_errors;
    uint64_t overrun_errors;

    // Performance simulation
    double base_latency_us;
    double throughput_mbps;
};

// Enhanced transfer simulation with error injection
static pcie_sim_error_t simulate_transfer(struct linux_sim_device* dev,
                                         size_t size,
                                         pcie_sim_direction_t direction,
                                         uint64_t* latency_ns) {
    // Check error injection probability
    if (should_inject_error(dev)) {
        return simulate_error_scenario(dev, size, latency_ns);
    }

    // Normal transfer simulation
    return simulate_normal_transfer(dev, size, direction, latency_ns);
}
```

**Error Injection Implementation:**
- **Timeout Simulation**: Artificial delays with configurable recovery time
- **Corruption Detection**: Simulated data integrity checks with failure paths
- **Buffer Overrun**: Safe boundary validation with controlled failure modes
- **Recovery Modeling**: Realistic error recovery time simulation

**Performance Characteristics:**
- **Base Latency**: 80-200 μs (software overhead simulation)
- **Throughput**: 1-4 Gbps (realistic userspace limitations)
- **Precision**: Nanosecond timing resolution with `CLOCK_MONOTONIC`
- **Error Overhead**: +50-200 μs recovery delays per error scenario

### 🪟 **Windows Simulation Backend (`windows_sim.c`)**

Native Windows simulation backend providing full PCIe device simulation on Windows platforms.

**Key Features:**
- **Windows Native**: Uses Windows API for timing and synchronization
- **High-Resolution Timing**: `QueryPerformanceCounter()` for microsecond precision
- **Thread Safety**: Windows mutexes and synchronization primitives
- **Error Injection**: Complete error scenario support matching Linux behavior
- **MinGW Compatible**: Builds with MinGW-w64 and MSYS2 environments
- **Configuration Integration**: Shared configuration structures across platforms

**Windows-Specific Implementation:**
```c
#include <windows.h>
#include "../utils/config.h"

// Windows-specific device structure
struct windows_sim_device {
    int device_id;
    BOOL is_open;
    CRITICAL_SECTION mutex;

    // High-resolution timing
    LARGE_INTEGER frequency;
    LARGE_INTEGER last_access;

    // Simulation state
    UINT64 transfer_count;
    UINT64 total_bytes;

    // Error injection (matching Linux)
    pcie_sim_error_scenario_t error_scenario;
    float error_probability;
    DWORD error_recovery_time_ms;

    // Windows-specific error tracking
    UINT64 timeout_errors;
    UINT64 corruption_errors;
    UINT64 overrun_errors;
};

// High-precision timing implementation
static UINT64 get_current_time_us(void) {
    LARGE_INTEGER counter, frequency;
    QueryPerformanceCounter(&counter);
    QueryPerformanceFrequency(&frequency);
    return (counter.QuadPart * 1000000) / frequency.QuadPart;
}

// Windows-specific error simulation
static pcie_sim_error_t simulate_windows_error(struct windows_sim_device* dev,
                                              pcie_sim_error_scenario_t scenario,
                                              UINT64* latency_us) {
    switch (scenario) {
    case PCIE_SIM_ERROR_SCENARIO_TIMEOUT:
        Sleep(dev->error_recovery_time_ms);
        dev->timeout_errors++;
        return PCIE_SIM_ERROR_TIMEOUT;

    case PCIE_SIM_ERROR_SCENARIO_CORRUPTION:
        Sleep(dev->error_recovery_time_ms / 2);
        dev->corruption_errors++;
        return PCIE_SIM_ERROR_SYSTEM;

    case PCIE_SIM_ERROR_SCENARIO_OVERRUN:
        Sleep(dev->error_recovery_time_ms * 2);
        dev->overrun_errors++;
        return PCIE_SIM_ERROR_MEMORY;
    }

    return PCIE_SIM_SUCCESS;
}
```

**Windows-Specific Features:**
- **COM Integration**: Optional COM object interface for advanced Windows applications
- **Windows Events**: Event-based notification system for asynchronous operations
- **Registry Configuration**: Optional Windows registry-based configuration storage
- **Windows Services**: Support for Windows service deployment

**Performance Characteristics:**
- **Base Latency**: 100-250 μs (Windows API overhead)
- **Throughput**: 1-3 Gbps (Windows userspace simulation)
- **Timing Precision**: Microsecond resolution with QueryPerformanceCounter
- **Thread Safety**: Windows critical sections for synchronization

## Backend Selection and Integration

### Automatic Backend Selection

The library automatically selects the appropriate backend based on the platform and available resources:

```c
// Backend selection logic (lib/core.c)
static pcie_sim_error_t initialize_backend(void) {
#ifdef __linux__
    // Linux: Try kernel module first, fallback to simulation
    if (access("/dev/pcie_sim0", R_OK | W_OK) == 0) {
        return init_kernel_backend();
    } else {
        return init_linux_sim_backend();
    }
#elif defined(_WIN32)
    // Windows: Always use simulation backend
    return init_windows_sim_backend();
#elif defined(__APPLE__)
    // macOS: Use Linux simulation backend
    return init_linux_sim_backend();
#else
    #error "Unsupported platform"
#endif
}
```

### Configuration Integration

Both simulation backends integrate seamlessly with the shared configuration system:

```c
// Shared configuration application
static pcie_sim_error_t apply_config(struct sim_device* dev,
                                     const struct pcie_sim_test_config* config) {
    // Apply error injection settings
    dev->error_scenario = config->error.scenario;
    dev->error_probability = config->error.probability;
    dev->error_recovery_time_ms = config->error.recovery_time_ms;

    // Apply transfer pattern settings
    switch (config->transfer.pattern) {
    case PCIE_SIM_PATTERN_SMALL_FAST:
        dev->base_latency_us = 50.0;   // Optimized for small transfers
        dev->throughput_mbps = 8000.0;
        break;
    case PCIE_SIM_PATTERN_LARGE_BURST:
        dev->base_latency_us = 200.0;  // Higher latency, better throughput
        dev->throughput_mbps = 12000.0;
        break;
    case PCIE_SIM_PATTERN_MIXED:
        dev->base_latency_us = 125.0;  // Balanced performance
        dev->throughput_mbps = 4000.0;
        break;
    }

    return PCIE_SIM_SUCCESS;
}
```

## Build System Integration

### Cross-Platform Build Support

The simulation backends are automatically built and integrated based on the target platform:

```bash
# Linux build (includes both kernel and simulation backends)
make -C sim

# Generated objects:
# - out/sim/obj/linux_sim.o    (Linux simulation backend)
# - out/sim/obj/windows_sim.o  (Windows simulation backend, if cross-compiling)

# Windows build (MSYS2/MinGW)
make -C sim -f Makefile.win

# Generated objects:
# - out/sim/obj/windows_sim.o  (Windows simulation backend)
```

### Library Integration

The simulation backends are automatically linked into the main library:

```bash
# Library build includes appropriate simulation backend
make lib

# Static library includes:
# - lib/core.c, lib/utils.c, lib/device.cpp
# - utils/config.c, utils/options.cpp, utils/csv_logger.cpp
# - sim/linux_sim.c (Linux) or sim/windows_sim.c (Windows)
```

### Conditional Compilation

Platform-specific features are handled through conditional compilation:

```c
// Cross-platform timing abstraction
#ifdef __linux__
    #include <time.h>
    static uint64_t get_time_ns(void) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
    }
#elif defined(_WIN32)
    #include <windows.h>
    static uint64_t get_time_ns(void) {
        LARGE_INTEGER counter, frequency;
        QueryPerformanceCounter(&counter);
        QueryPerformanceFrequency(&frequency);
        return (counter.QuadPart * 1000000000ULL) / frequency.QuadPart;
    }
#endif

// Cross-platform synchronization
#ifdef __linux__
    #include <pthread.h>
    typedef pthread_mutex_t sim_mutex_t;
    #define SIM_MUTEX_INIT(m) pthread_mutex_init(m, NULL)
    #define SIM_MUTEX_LOCK(m) pthread_mutex_lock(m)
    #define SIM_MUTEX_UNLOCK(m) pthread_mutex_unlock(m)
#elif defined(_WIN32)
    #include <windows.h>
    typedef CRITICAL_SECTION sim_mutex_t;
    #define SIM_MUTEX_INIT(m) InitializeCriticalSection(m)
    #define SIM_MUTEX_LOCK(m) EnterCriticalSection(m)
    #define SIM_MUTEX_UNLOCK(m) LeaveCriticalSection(m)
#endif
```

## Enhanced Features

### Realistic Transfer Simulation

Both backends implement sophisticated transfer simulation that models real PCIe behavior:

**Size-Dependent Latency:**
```c
static uint64_t calculate_transfer_latency(size_t size, double base_latency_us) {
    // Base latency + size-dependent scaling
    double size_factor = sqrt((double)size / 4096.0);  // Square root scaling
    double total_latency_us = base_latency_us * (1.0 + size_factor * 0.1);

    // Add realistic jitter (±10%)
    double jitter = (rand() / (double)RAND_MAX - 0.5) * 0.2;
    total_latency_us *= (1.0 + jitter);

    return (uint64_t)(total_latency_us * 1000.0);  // Convert to nanoseconds
}
```

**Throughput Modeling:**
```c
static double calculate_throughput_mbps(size_t size, uint64_t latency_ns) {
    // Throughput = (size * 8 bits) / (latency in seconds) / 1e6 for Mbps
    double bits = size * 8.0;
    double seconds = latency_ns / 1e9;
    double mbps = bits / (seconds * 1e6);

    // Apply realistic efficiency limits based on transfer size
    double efficiency = (size < 1024) ? 0.3 :      // Small transfers: 30% efficiency
                       (size < 64*1024) ? 0.7 :    // Medium transfers: 70% efficiency
                       0.9;                        // Large transfers: 90% efficiency

    return mbps * efficiency;
}
```

### Advanced Error Injection

**Probabilistic Error Generation:**
```c
static bool should_inject_error(struct sim_device* dev) {
    if (dev->error_scenario == PCIE_SIM_ERROR_SCENARIO_NONE) {
        return false;
    }

    // Generate random number [0.0, 1.0)
    float random = (float)rand() / (float)RAND_MAX;
    return random < dev->error_probability;
}

static pcie_sim_error_t simulate_error_scenario(struct sim_device* dev,
                                               size_t size,
                                               uint64_t* latency_ns) {
    // Apply error-specific recovery delay
    uint64_t recovery_delay_ns = dev->error_recovery_time_ms * 1000000ULL;

    switch (dev->error_scenario) {
    case PCIE_SIM_ERROR_SCENARIO_TIMEOUT:
        // Simulate timeout with extended delay
        *latency_ns += recovery_delay_ns;
        dev->timeout_errors++;
        return PCIE_SIM_ERROR_TIMEOUT;

    case PCIE_SIM_ERROR_SCENARIO_CORRUPTION:
        // Simulate corruption detection with moderate delay
        *latency_ns += recovery_delay_ns / 2;
        dev->corruption_errors++;
        return PCIE_SIM_ERROR_SYSTEM;  // Map to system error

    case PCIE_SIM_ERROR_SCENARIO_OVERRUN:
        // Simulate buffer overrun with significant delay
        *latency_ns += recovery_delay_ns * 2;
        dev->overrun_errors++;
        return PCIE_SIM_ERROR_MEMORY;  // Map to memory error
    }

    return PCIE_SIM_SUCCESS;
}
```

### Performance Optimization

**Memory Management:**
```c
// Efficient memory pool for transfer simulation
struct memory_pool {
    void* pool_memory;
    size_t pool_size;
    size_t used_size;
    sim_mutex_t pool_mutex;
};

static void* sim_alloc(struct memory_pool* pool, size_t size) {
    SIM_MUTEX_LOCK(&pool->pool_mutex);

    if (pool->used_size + size > pool->pool_size) {
        SIM_MUTEX_UNLOCK(&pool->pool_mutex);
        return NULL;  // Pool exhausted
    }

    void* ptr = (char*)pool->pool_memory + pool->used_size;
    pool->used_size += (size + 7) & ~7;  // 8-byte alignment

    SIM_MUTEX_UNLOCK(&pool->pool_mutex);
    return ptr;
}
```

**Cache-Friendly Data Structures:**
```c
// Optimize for cache performance
struct sim_device {
    // Hot data (frequently accessed) - first cache line
    int device_id;
    bool is_open;
    pcie_sim_error_scenario_t error_scenario;
    float error_probability;

    // Performance data - second cache line
    uint64_t transfer_count;
    uint64_t total_bytes;
    double base_latency_us;
    double throughput_mbps;

    // Cold data (infrequently accessed)
    sim_mutex_t mutex;
    char padding[64];  // Ensure next device starts on new cache line
} __attribute__((aligned(64)));
```

## Testing and Validation

### Backend Validation Suite

```bash
# Test Linux simulation backend
out/examples/cpp_test --help  # Should show "Backend: LinuxSim"

# Test Windows simulation backend (on Windows)
out/examples/cpp_test.exe --help  # Should show "Backend: WindowsSim"

# Cross-backend compatibility test
out/examples/cpp_test --pattern small-fast --log-csv linux_sim.csv
# CSV format should be identical across backends
```

### Performance Benchmarking

```bash
#!/bin/bash
# backend_benchmark.sh

echo "Benchmarking simulation backends..."

# Linux simulation backend
echo "Testing Linux simulation backend..."
out/examples/cpp_test --pattern small-fast --duration 10 --log-csv linux_sim.csv
out/examples/cpp_test --pattern large-burst --duration 10 --log-csv linux_burst.csv

# Error injection testing
out/examples/cpp_test --error-scenario timeout --duration 5 --log-csv linux_errors.csv

# Performance analysis
python3 -c "
import pandas as pd
import numpy as np

# Load results
sim_data = pd.read_csv('linux_sim.csv', comment='#')
print(f'Average latency: {sim_data[\"latency_us\"].mean():.2f} μs')
print(f'Throughput: {sim_data[\"throughput_mbps\"].mean():.2f} Mbps')
print(f'Error rate: {((sim_data[\"error_status\"] != \"SUCCESS\").sum() / len(sim_data) * 100):.2f}%')
"

echo "Backend benchmark complete."
```

### Cross-Platform Validation

```bash
# Windows validation (MSYS2)
pacman -S mingw-w64-x86_64-gcc
make deps
make windows
out/examples/cpp_test.exe --pattern mixed --duration 5

# macOS validation
brew install gcc
make all
out/examples/cpp_test --pattern mixed --duration 5

# Compare results across platforms
# All platforms should produce similar performance characteristics
```

## Integration Examples

### Custom Backend Development

For specialized environments, custom backends can be developed following the established interface:

```c
// custom_backend.c
#include "../utils/config.h"
#include "../lib/api.h"

// Custom backend implementation
static pcie_sim_error_t custom_open(int device_id, pcie_sim_handle_t* handle) {
    // Custom device opening logic
    return PCIE_SIM_SUCCESS;
}

static pcie_sim_error_t custom_transfer(pcie_sim_handle_t handle,
                                       void* buffer, size_t size,
                                       pcie_sim_direction_t direction,
                                       uint64_t* latency_ns) {
    // Custom transfer simulation
    return PCIE_SIM_SUCCESS;
}

// Backend registration
static const struct pcie_sim_backend custom_backend = {
    .name = "CustomBackend",
    .open = custom_open,
    .close = custom_close,
    .transfer = custom_transfer,
    .get_stats = custom_get_stats
};

void register_custom_backend(void) {
    pcie_sim_register_backend(&custom_backend);
}
```

### Container Deployment

```dockerfile
# Dockerfile for simulation backend deployment
FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc-multilib \
    g++-multilib

# Copy source code
COPY . /pcie-simulator

# Build simulation backend (no kernel module)
WORKDIR /pcie-simulator
RUN make all

# Test simulation backend
RUN out/examples/cpp_test --pattern small-fast --duration 1

# Entry point for testing
ENTRYPOINT ["out/examples/cpp_test"]
CMD ["--help"]
```

## Troubleshooting

### Common Issues

**Timing Precision Issues:**
```bash
# Check timing resolution (Linux)
cat /sys/devices/system/clocksource/clocksource0/current_clocksource
# Should show "tsc" or "hpet" for best precision

# Check performance counter frequency (Windows)
# Registry: HKEY_LOCAL_MACHINE\HARDWARE\DESCRIPTION\System\CentralProcessor\0
```

**Synchronization Problems:**
```bash
# Thread safety validation
out/examples/cpp_test --threads 8 --duration 10 --verbose
# Should show no race conditions or inconsistent statistics
```

**Performance Issues:**
- **High Latency**: Reduce system load, check for CPU throttling
- **Low Throughput**: Increase transfer size, reduce error injection rate
- **Timing Inconsistency**: Ensure stable CPU frequency, disable power management

### Debug Features

**Backend-Specific Debugging:**
```c
// Enable debug mode (compile time)
#define SIM_DEBUG 1

// Debug output (runtime)
export PCIE_SIM_DEBUG=1
out/examples/cpp_test --verbose
```

**Performance Profiling:**
```bash
# Profile simulation backend
perf record -g out/examples/cpp_test --pattern large-burst --duration 30
perf report

# Memory usage analysis
valgrind --tool=massif out/examples/cpp_test --duration 10
ms_print massif.out.*
```

---

**Cross-platform simulation backends providing realistic PCIe device behavior with enhanced error injection and configuration integration for comprehensive testing without kernel module dependencies.**