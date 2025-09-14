# PCIe Simulator - Example Applications

**Directory:** `examples/`
**Purpose:** Comprehensive test applications demonstrating C and C++ APIs with advanced testing capabilities

## Overview

The `examples/` directory contains demonstration applications that showcase both the C and C++ APIs of the PCIe Simulator library. The enhanced test suite includes pattern-based testing, error injection scenarios, multi-threaded stress testing, and high-performance CSV logging.

## Applications

### 🔰 **Basic C Test (`basic_test.c`)**

Simple demonstration of the C library interface with fundamental operations.

**Purpose:**
- Introduction to C API usage
- Basic device operations
- Simple transfer testing
- Error handling examples

**Features:**
- Device open/close operations
- Synchronous data transfers
- Statistics retrieval
- Error handling demonstration

**Usage:**
```bash
# Build and run basic C test
make -C examples basic_test
out/examples/basic_test

# Or use make target
make -C examples run-c
```

**Code Overview:**
```c
#include "pcie_sim.h"

pcie_sim_handle_t handle;
pcie_sim_error_t ret;
uint8_t data[4096];

// Open device
ret = pcie_sim_open(0, &handle);

// Transfer data
uint64_t latency_ns;
ret = pcie_sim_transfer(handle, data, sizeof(data),
                       PCIE_SIM_TO_DEVICE, &latency_ns);

// Get statistics
struct pcie_sim_stats stats;
ret = pcie_sim_get_stats(handle, &stats);

// Close device
ret = pcie_sim_close(handle);
```

### 🚀 **Enhanced C++ Test Suite (`cpp_test.cpp`)**

Comprehensive testing application with advanced features for OTPU performance analysis and driver validation.

**Purpose:**
- Advanced C++ API demonstration
- Pattern-based performance testing
- Error injection and fault tolerance testing
- Multi-threaded stress testing
- CSV data logging and analysis

**Key Features:**
- **Command-line Interface**: Comprehensive option parsing
- **Transfer Patterns**: Small-fast, large-burst, mixed, and custom patterns
- **Error Injection**: Timeout, corruption, and overrun scenarios
- **Stress Testing**: Multi-threaded concurrent device access
- **CSV Logging**: High-performance data collection
- **Real-time Monitoring**: Verbose progress tracking

#### Enhanced Command-Line Interface

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
  --help, -h              Show detailed help message
```

#### Transfer Pattern Testing

**Small-Fast Pattern (OTPU Optimized):**
```bash
# High-frequency, low-latency transfers
out/examples/cpp_test --pattern small-fast --num-devices 4 --verbose

# Configuration:
# - Size: 64B - 1KB
# - Rate: 10,000 Hz
# - Use Case: Small tensor operations, control messages
```

**Large-Burst Pattern:**
```bash
# Bulk data transfer simulation
out/examples/cpp_test --pattern large-burst --log-csv bulk_test.csv

# Configuration:
# - Size: 1MB - 4MB
# - Rate: 100 Hz
# - Use Case: Model loading, large dataset transfers
```

**Mixed Workload Pattern:**
```bash
# Realistic mixed workload
out/examples/cpp_test --pattern mixed --num-devices 8

# Configuration:
# - Size: 1KB - 64KB
# - Rate: 1,000 Hz
# - Use Case: General-purpose workload simulation
```

**Custom Pattern:**
```bash
# User-defined parameters
out/examples/cpp_test --pattern custom --size 2048 --rate 5000 --verbose

# Allows fine-tuning for specific benchmark scenarios
```

#### Error Injection Testing

**Timeout Scenarios:**
```bash
# Simulate transfer timeouts
out/examples/cpp_test --error-scenario timeout --num-devices 4 --verbose

# Default: 1% error rate, 100ms recovery time
# Use Case: Network timeouts, device busy conditions
```

**Data Corruption Scenarios:**
```bash
# Simulate data integrity issues
out/examples/cpp_test --error-scenario corruption --log-csv corruption_test.csv

# Default: 0.5% error rate, 50ms recovery time
# Use Case: Data integrity validation, ECC testing
```

**Buffer Overrun Scenarios:**
```bash
# Simulate memory boundary violations
out/examples/cpp_test --error-scenario overrun --verbose

# Default: 2% error rate, 200ms recovery time
# Use Case: Memory safety testing, boundary validation
```

#### Multi-threaded Stress Testing

**Basic Stress Test:**
```bash
# 8 concurrent threads for 30 seconds
out/examples/cpp_test --threads 8 --duration 30 --num-devices 4

# Features:
# - Round-robin device assignment
# - Per-thread performance tracking
# - Thread safety validation
```

**Comprehensive Stress Test:**
```bash
# Combined stress test with error injection
out/examples/cpp_test --threads 16 --duration 60 --error-scenario timeout \
                     --pattern mixed --log-csv stress_test.csv --verbose

# Validates:
# - Concurrent device access
# - Error handling under load
# - Thread safety with error conditions
```

#### CSV Logging and Analysis

**Basic Logging:**
```bash
# Log all transfers to CSV
out/examples/cpp_test --pattern mixed --log-csv results.csv --num-devices 4

# Generated CSV includes:
# - Timestamp (microsecond precision)
# - Session time and configuration
# - Transfer size, latency, throughput
# - Error status and thread ID
```

**Advanced Analysis:**
```bash
# Comprehensive test with full logging
out/examples/cpp_test --pattern large-burst --threads 4 --duration 20 \
                     --error-scenario corruption --log-csv analysis.csv --verbose

# CSV can be imported into:
# - Spreadsheet applications (Excel, LibreOffice)
# - Data analysis tools (Python pandas, R)
# - Visualization tools (Grafana, matplotlib)
```

### 📊 **Legacy Test (`cpp_test_old.cpp`)**

Preserved original C++ test application for compatibility and comparison.

**Purpose:**
- Backward compatibility reference
- Simple C++ API usage examples
- Legacy code maintenance

## Build System

### Building Examples

```bash
# Build all examples
make -C examples all

# Build specific examples
make -C examples basic_test    # C example
make -C examples cpp_test      # Enhanced C++ test suite
make -C examples cpp_test_old  # Legacy C++ example

# Clean examples
make -C examples clean
```

### Build Configurations

**Static Linking (Default):**
```bash
make -C examples static
# Links against out/lib/libpcie_sim.a
# Self-contained executables
```

**Shared Linking:**
```bash
make -C examples shared
# Links against out/lib/libpcie_sim.so
# Requires library to be in system path or LD_LIBRARY_PATH
```

**Windows Build:**
```bash
make -C examples -f Makefile.win all
# MinGW-w64 compatible build
# Uses simulation backend (no kernel module)
```

### Running Examples

**Automated Testing:**
```bash
# Run all examples with kernel module loaded
make run-examples

# Individual test runs
make -C examples run-c      # Basic C test
make -C examples run-cpp    # Enhanced C++ test suite
```

**Manual Testing:**
```bash
# Load kernel module first (Linux)
make load

# Run individual examples
out/examples/basic_test
out/examples/cpp_test --help
out/examples/cpp_test --pattern small-fast --verbose
```

## Testing Scenarios

### 🎯 **Performance Benchmarking**

**OTPU Baseline Testing:**
```bash
# Small, fast transfers optimized for OTPU workloads
out/examples/cpp_test --pattern small-fast --num-devices 8 \
                     --log-csv otpu_baseline.csv --verbose

# Analyze CSV for:
# - Average latency per device
# - Throughput consistency
# - Inter-device latency variance
```

**Throughput Testing:**
```bash
# Large burst transfers for maximum throughput
out/examples/cpp_test --pattern large-burst --duration 60 \
                     --log-csv throughput_test.csv

# Metrics to analyze:
# - Peak throughput (Mbps)
# - Sustained performance
# - Transfer efficiency
```

### 🚨 **Fault Tolerance Testing**

**Error Recovery Testing:**
```bash
# Test all error scenarios
out/examples/cpp_test --error-scenario timeout --verbose --duration 30
out/examples/cpp_test --error-scenario corruption --verbose --duration 30
out/examples/cpp_test --error-scenario overrun --verbose --duration 30

# Validate:
# - Error detection accuracy
# - Recovery time consistency
# - System stability under errors
```

**Error Rate Analysis:**
```bash
# Long-duration error testing with logging
out/examples/cpp_test --error-scenario timeout --duration 300 \
                     --log-csv error_analysis.csv --pattern mixed

# CSV analysis:
# - Actual vs expected error rate
# - Error distribution over time
# - Recovery performance metrics
```

### ⚡ **Concurrency Testing**

**Thread Safety Validation:**
```bash
# High-concurrency testing
out/examples/cpp_test --threads 32 --duration 60 --num-devices 8 \
                     --pattern mixed --verbose

# Validates:
# - No race conditions
# - Consistent performance under load
# - Proper resource sharing
```

**Load Testing:**
```bash
# Combined load and error testing
out/examples/cpp_test --threads 16 --duration 120 --error-scenario timeout \
                     --pattern small-fast --log-csv load_test.csv

# Stress tests:
# - High-frequency operations
# - Error handling under load
# - Memory management under stress
```

### 📈 **Regression Testing**

**Automated Regression Suite:**
```bash
#!/bin/bash
# Regression test script

# Basic functionality
out/examples/basic_test || exit 1

# Default patterns
out/examples/cpp_test --pattern small-fast || exit 1
out/examples/cpp_test --pattern large-burst || exit 1
out/examples/cpp_test --pattern mixed || exit 1

# Error scenarios
out/examples/cpp_test --error-scenario timeout --duration 5 || exit 1

# Multi-threading
out/examples/cpp_test --threads 4 --duration 5 || exit 1

echo "Regression tests passed!"
```

## Data Analysis

### CSV Data Format

The enhanced logging system produces structured CSV data:

```csv
timestamp,session_time_ms,device_id,transfer_size,latency_us,throughput_mbps,direction,error_status,thread_id
# Session Start: 2025-01-15 14:30:25.123
# Configuration: pattern=mixed,devices=4,size=1024-65536,rate=1000
2025-01-15 14:30:25.124,0,0,4096,125.45,0.26,TO_DEVICE,SUCCESS,12345
2025-01-15 14:30:25.125,1,1,8192,98.32,0.67,TO_DEVICE,SUCCESS,12346
2025-01-15 14:30:25.126,2,0,2048,156.78,0.10,TO_DEVICE,TIMEOUT,12345
```

### Analysis Examples

**Python Analysis:**
```python
import pandas as pd
import matplotlib.pyplot as plt

# Load CSV data
df = pd.read_csv('results.csv', comment='#')

# Latency analysis
print(f"Average latency: {df['latency_us'].mean():.2f} μs")
print(f"95th percentile: {df['latency_us'].quantile(0.95):.2f} μs")

# Error rate analysis
error_rate = (df['error_status'] != 'SUCCESS').sum() / len(df) * 100
print(f"Error rate: {error_rate:.2f}%")

# Throughput by device
device_throughput = df.groupby('device_id')['throughput_mbps'].mean()
print(device_throughput)

# Plot latency distribution
df['latency_us'].hist(bins=50)
plt.xlabel('Latency (μs)')
plt.ylabel('Frequency')
plt.title('Transfer Latency Distribution')
plt.show()
```

**Spreadsheet Analysis:**
1. Import CSV into Excel/LibreOffice Calc
2. Create pivot tables for device-wise statistics
3. Generate charts for latency trends
4. Analyze error patterns over time

## Troubleshooting

### Common Issues

**Build Errors:**
```bash
# Missing dependencies
make deps

# Clean rebuild
make clean && make all

# Check specific component
make -C lib help
make -C examples help
```

**Runtime Errors:**
```bash
# Kernel module not loaded (Linux)
make load

# Check device files
ls -la /dev/pcie_sim*

# Verify module status
make status

# Check statistics
cat /proc/pcie_sim0/stats
```

**CSV Logging Issues:**
```bash
# Permission issues
chmod 755 .
touch test.csv  # Check write permissions

# File path issues
out/examples/cpp_test --log-csv ./results.csv  # Use relative path
out/examples/cpp_test --log-csv /tmp/results.csv  # Use absolute path
```

### Performance Issues

**High Latency:**
- Reduce transfer size with `--size` option
- Use `--pattern small-fast` for low-latency testing
- Check system load and CPU usage

**Low Throughput:**
- Increase transfer size with `--pattern large-burst`
- Reduce thread count if CPU-bound
- Check memory allocation patterns

**Error Injection Not Working:**
- Verify error scenario spelling: `timeout`, `corruption`, `overrun`
- Check verbose output for error injection messages
- Ensure error probability is reasonable (1-5%)

## Integration Examples

### Custom Test Scripts

**Automated Benchmark:**
```bash
#!/bin/bash
# comprehensive_benchmark.sh

RESULTS_DIR="benchmark_results_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$RESULTS_DIR"

echo "Running comprehensive benchmark suite..."

# Pattern testing
for pattern in small-fast large-burst mixed; do
    echo "Testing pattern: $pattern"
    out/examples/cpp_test --pattern $pattern --duration 30 \
                         --log-csv "$RESULTS_DIR/${pattern}_test.csv" \
                         --verbose
done

# Error testing
for error in timeout corruption overrun; do
    echo "Testing error scenario: $error"
    out/examples/cpp_test --error-scenario $error --duration 20 \
                         --log-csv "$RESULTS_DIR/${error}_test.csv" \
                         --verbose
done

# Stress testing
echo "Running stress test..."
out/examples/cpp_test --threads 8 --duration 60 --pattern mixed \
                     --log-csv "$RESULTS_DIR/stress_test.csv" \
                     --verbose

echo "Benchmark complete. Results in: $RESULTS_DIR"
```

**CI/CD Integration:**
```yaml
# .github/workflows/test.yml
name: PCIe Simulator Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v2

    - name: Install dependencies
      run: make deps

    - name: Build all components
      run: make all

    - name: Run basic tests
      run: |
        out/examples/basic_test
        out/examples/cpp_test --pattern mixed --duration 5
        out/examples/cpp_test --error-scenario timeout --duration 5
        out/examples/cpp_test --threads 4 --duration 5
```

---

**Comprehensive test applications demonstrating advanced PCIe simulation capabilities with OTPU-optimized performance testing and validation.**