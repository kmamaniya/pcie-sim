# PCIe Simulator - Kernel Module

**Directory:** `kernel/`
**Purpose:** Linux kernel module providing realistic PCIe device simulation with enhanced error injection support

## Overview

The kernel module implements a complete platform device driver that simulates a PCIe device with DMA capabilities, memory-mapped I/O, interrupt handling, and ring buffer communication protocols. Enhanced with error injection scenarios for comprehensive testing.

## Module Components

### 🔧 **Core Platform Driver (`driver.c`)**

Main platform driver implementing the Linux device model framework.

**Key Features:**
- Platform device registration and management
- Device lifecycle management (probe/remove)
- Power management support
- Sysfs interface integration

**Device Registration:**
```c
static struct platform_driver pcie_sim_driver = {
    .driver = {
        .name = DRIVER_NAME,
        .owner = THIS_MODULE,
    },
    .probe = pcie_sim_probe,
    .remove = pcie_sim_remove,
};
```

### 📁 **Character Device Interface (`chardev.c`)**

Character device interface providing `/dev/pcie_sim*` device files for userspace communication.

**Key Features:**
- IOCTL command interface for control operations
- Enhanced error injection configuration support
- Device statistics and status reporting
- User-kernel data exchange

**Device Files:**
- `/dev/pcie_sim0` - Primary device interface
- `/dev/pcie_sim1` - Secondary device (if configured)

**IOCTL Commands:**
```c
#define PCIE_SIM_IOC_TRANSFER    _IOWR(PCIE_SIM_IOC_MAGIC, 1, struct pcie_sim_transfer_req)
#define PCIE_SIM_IOC_GET_STATS   _IOR(PCIE_SIM_IOC_MAGIC, 2, struct pcie_sim_stats)
#define PCIE_SIM_IOC_RESET_STATS _IO(PCIE_SIM_IOC_MAGIC, 3)
#define PCIE_SIM_IOC_SET_ERROR   _IOW(PCIE_SIM_IOC_MAGIC, 4, struct pcie_sim_error_config)
```

### 💾 **DMA Simulation (`dma.c`)**

Realistic DMA transfer simulation with configurable latency and enhanced error injection.

**Key Features:**
- Coherent DMA buffer management
- Realistic transfer timing simulation
- **Enhanced Error Injection**: Timeout, corruption, overrun scenarios
- Scatter-gather operation support
- DMA mapping and unmapping

**Error Injection Support:**
```c
// Enhanced error scenarios (integrated with utils/config.h)
#define PCIE_SIM_ERROR_SCENARIO_NONE        0
#define PCIE_SIM_ERROR_SCENARIO_TIMEOUT     1
#define PCIE_SIM_ERROR_SCENARIO_CORRUPTION  2
#define PCIE_SIM_ERROR_SCENARIO_OVERRUN     3

struct pcie_sim_error_config {
    u32 scenario;           // Error scenario (0-3)
    u32 probability;        // Error probability in 0.01% units (0-10000)
    u32 recovery_time_ms;   // Recovery time after error
    u32 flags;              // Configuration flags
};
```

**Transfer Process with Error Injection:**
1. Validate transfer request
2. Check error injection probability
3. Simulate appropriate error scenario if triggered
4. Perform DMA transfer with realistic timing
5. Apply recovery delays for error scenarios
6. Update statistics and error counters

### 🗺️ **Memory-Mapped I/O (`mmio.c`)**

BAR (Base Address Register) simulation providing memory-mapped register interface.

**Key Features:**
- Virtual BAR0 region simulation
- Register read/write operations
- DMA descriptor ring integration
- Status and control register simulation

**Register Map:**
```c
#define REG_DEVICE_ID      0x000  // Device identification
#define REG_STATUS         0x004  // Device status
#define REG_CONTROL        0x008  // Control register
#define REG_DMA_ADDR_LO    0x010  // DMA address (low 32 bits)
#define REG_DMA_ADDR_HI    0x014  // DMA address (high 32 bits)
#define REG_DMA_SIZE       0x018  // DMA transfer size
#define REG_DMA_CONTROL    0x01C  // DMA control
#define REG_INTERRUPT_STATUS 0x020  // Interrupt status
#define REG_INTERRUPT_ENABLE 0x024  // Interrupt enable
```

### 📊 **Proc Filesystem Interface (`procfs.c`)**

Proc filesystem entries for real-time statistics and device information.

**Key Features:**
- Real-time statistics display
- Device configuration information
- **Enhanced Error Statistics**: Per-scenario error tracking
- Performance metrics
- Debug information

**Proc Entries:**
- `/proc/pcie_sim0/stats` - Device statistics
- `/proc/pcie_sim0/config` - Device configuration
- `/proc/pcie_sim0/errors` - **Enhanced**: Error injection statistics

**Enhanced Statistics Output:**
```
PCIe Simulator Device Statistics
================================
Device ID: 0
Status: Active

Transfer Statistics:
  Total Transfers: 15432
  Total Bytes: 234567890
  Average Latency: 125.45 μs
  Peak Throughput: 2.34 Gbps

Error Injection Statistics:
  Scenario: TIMEOUT
  Error Rate: 1.02% (157/15432)
  Recovery Time: 100ms avg
  Total Error Time: 15.7s

  Timeout Errors: 157
  Corruption Errors: 0
  Overrun Errors: 0
```

### 🔄 **Ring Buffer Management (`ringbuffer.c`)**

High-performance ring buffer implementation for DMA descriptor management.

**Key Features:**
- Lock-free producer/consumer operations
- 256-descriptor rings (configurable)
- Atomic operations for thread safety
- Overflow and underflow detection

**Ring Buffer Structure:**
```c
struct pcie_sim_ring {
    struct pcie_sim_ring_desc *descriptors;
    dma_addr_t desc_dma_addr;
    u32 size;           // Number of descriptors
    u32 head;           // Producer index
    u32 tail;           // Consumer index
    atomic_t count;     // Current entries
    spinlock_t lock;    // Ring protection

    // Enhanced statistics
    atomic64_t submissions;
    atomic64_t completions;
    atomic64_t overruns;
};
```

### 📋 **Common Definitions (`common.h`)**

Shared kernel definitions and structures with enhanced error injection support.

**Key Features:**
- **Enhanced Error Injection Structures**: Compatible with utils/config.h
- Device state management
- IOCTL command definitions
- Statistics structures
- Cross-component data structures

## Enhanced Error Injection System

### Error Scenarios

The kernel module now supports sophisticated error injection scenarios integrated with the shared configuration system:

**Timeout Simulation:**
- Simulates device timeouts and busy conditions
- Configurable probability (default: 1%)
- Recovery time: 100ms
- Use case: Network timeouts, device busy scenarios

**Data Corruption Simulation:**
- Simulates data integrity issues
- Configurable probability (default: 0.5%)
- Recovery time: 50ms
- Use case: ECC validation, data integrity testing

**Buffer Overrun Simulation:**
- Simulates memory boundary violations
- Configurable probability (default: 2%)
- Recovery time: 200ms
- Use case: Memory safety testing, boundary validation

### Configuration Interface

**IOCTL Configuration:**
```c
struct pcie_sim_error_config config = {
    .scenario = PCIE_SIM_ERROR_SCENARIO_TIMEOUT,
    .probability = 100,        // 1% (in 0.01% units)
    .recovery_time_ms = 100,
    .flags = 0
};

ioctl(fd, PCIE_SIM_IOC_SET_ERROR, &config);
```

**Runtime Control:**
```bash
# Enable timeout errors via proc interface
echo "scenario=timeout probability=1.5" > /proc/pcie_sim0/error_config

# Disable error injection
echo "scenario=none" > /proc/pcie_sim0/error_config

# Check current error configuration
cat /proc/pcie_sim0/error_config
```

## Build System

### Building the Kernel Module

```bash
# Build kernel module
make -C kernel

# Or from project root
make kernel

# Install module (optional)
sudo make -C kernel install
```

### Module Management

```bash
# Load module
make -C kernel load
# Or: sudo insmod kernel/pcie_sim.ko

# Check module status
lsmod | grep pcie_sim
dmesg | tail -20  # Check kernel messages

# Unload module
make -C kernel unload
# Or: sudo rmmod pcie_sim

# Auto-load at boot (optional)
sudo make -C kernel install
echo "pcie_sim" | sudo tee -a /etc/modules
```

### Build Configuration

**Kernel Version Compatibility:**
```bash
# Check kernel headers
ls /lib/modules/$(uname -r)/build

# Install headers if missing (Ubuntu/Debian)
sudo apt install linux-headers-$(uname -r)

# Check kernel configuration
grep CONFIG_MODULES /boot/config-$(uname -r)
```

**Debug Build:**
```bash
# Build with debug symbols
make -C kernel DEBUG=1

# Enable dynamic debug
echo 'module pcie_sim +p' > /sys/kernel/debug/dynamic_debug/control
```

## Device Interface

### Character Device Operations

**Device Files:**
```bash
# Device files created automatically
ls -la /dev/pcie_sim*
crw-rw-rw- 1 root root 240, 0 Jan 15 14:30 /dev/pcie_sim0
```

**Basic Operations:**
```c
int fd = open("/dev/pcie_sim0", O_RDWR);

// Transfer data
struct pcie_sim_transfer_req req = {
    .buffer = data,
    .size = sizeof(data),
    .direction = 0,  // TO_DEVICE
    .latency_ns = 0  // Returned
};
ioctl(fd, PCIE_SIM_IOC_TRANSFER, &req);

// Get statistics
struct pcie_sim_stats stats;
ioctl(fd, PCIE_SIM_IOC_GET_STATS, &stats);

close(fd);
```

### Proc Interface

**Statistics Monitoring:**
```bash
# Real-time statistics
watch -n 1 cat /proc/pcie_sim0/stats

# Error statistics
cat /proc/pcie_sim0/errors

# Device configuration
cat /proc/pcie_sim0/config
```

**Configuration:**
```bash
# View current error configuration
cat /proc/pcie_sim0/error_config
scenario=none probability=0.00% recovery_time=0ms

# Enable timeout errors
echo "timeout 1.5 100" > /proc/pcie_sim0/error_config

# Verify configuration
cat /proc/pcie_sim0/error_config
scenario=timeout probability=1.50% recovery_time=100ms
```

## Integration with Userspace

### Enhanced Library Integration

The kernel module integrates seamlessly with the enhanced userspace library and utilities:

**Configuration Flow:**
1. **Application**: Uses `utils/options.hpp` for command-line parsing
2. **Library**: Converts config to IOCTL commands
3. **Kernel**: Applies configuration to error injection system
4. **Monitoring**: Real-time statistics via proc interface

**Example Integration:**
```cpp
// Application (cpp_test.cpp)
auto options = ProgramOptions::create_otpu_options();
auto config = options->to_config();

// Library (device.cpp) - converts to kernel format
struct pcie_sim_error_config kernel_config = {
    .scenario = map_error_scenario(config->error.scenario),
    .probability = (u32)(config->error.probability * 10000),
    .recovery_time_ms = config->error.recovery_time_ms
};

// Kernel applies configuration
ioctl(device_fd, PCIE_SIM_IOC_SET_ERROR, &kernel_config);
```

### Performance Integration

**CSV Logging Support:**
- Kernel provides microsecond-precision latency measurements
- Statistics exported via proc interface
- Real-time monitoring integration with `SessionLogger`

**Multi-threading Support:**
- Thread-safe device access with proper locking
- Per-thread statistics tracking
- Concurrent error injection across threads

## Performance Characteristics

### Latency Simulation

**Realistic Timing:**
- Base latency: 50-200 μs (configurable)
- Size-dependent scaling: Larger transfers = higher latency
- Jitter simulation: ±10% variance
- **Error Recovery**: Additional delays for error scenarios

**Throughput Modeling:**
- Peak theoretical: ~8 Gbps (PCIe x8 Gen3)
- Sustained practical: 2-4 Gbps
- Size efficiency: Larger transfers = better efficiency
- **Error Impact**: Reduced throughput during error recovery

### Memory Management

**DMA Buffers:**
- Coherent DMA allocation using `dma_alloc_coherent()`
- Proper cache management
- IOMMU compatibility
- **Error Injection**: Safe corruption simulation without actual memory corruption

**Ring Buffers:**
- High-performance descriptor rings
- Lock-free operations where possible
- Memory-efficient circular buffer design
- **Enhanced Monitoring**: Overflow/underflow detection and reporting

## Debugging and Troubleshooting

### Debug Information

**Kernel Messages:**
```bash
# Enable debug output
echo 8 > /proc/sys/kernel/printk

# Module debug messages
dmesg | grep pcie_sim

# Real-time debug
tail -f /var/log/kern.log | grep pcie_sim
```

**Performance Debugging:**
```bash
# Check interrupt statistics
cat /proc/interrupts | grep pcie_sim

# Monitor memory usage
cat /proc/slabinfo | grep pcie_sim

# Check DMA mappings
cat /proc/iomem | grep pcie_sim
```

### Common Issues

**Module Load Failures:**
```bash
# Check kernel version compatibility
uname -r
modinfo kernel/pcie_sim.ko

# Verify kernel headers
ls /lib/modules/$(uname -r)/build

# Check dependencies
lsmod | grep -E "(platform|cdev)"
```

**Device Creation Issues:**
```bash
# Check device creation
ls -la /dev/pcie_sim*

# Manual device creation if needed
sudo mknod /dev/pcie_sim0 c 240 0
sudo chmod 666 /dev/pcie_sim0
```

**Performance Issues:**
```bash
# Check system load
top -p $(pgrep -f pcie_sim)

# Monitor I/O statistics
iostat -x 1

# Check memory fragmentation
cat /proc/buddyinfo
```

## Security Considerations

### Access Control

**Device Permissions:**
```bash
# Restrict device access (production)
sudo chmod 660 /dev/pcie_sim*
sudo chgrp users /dev/pcie_sim*

# Development access (permissive)
sudo chmod 666 /dev/pcie_sim*
```

**Module Security:**
- No privileged escalation paths
- Proper input validation for all IOCTL commands
- **Safe Error Injection**: No actual memory corruption or system instability
- Bounded resource usage

### Error Injection Safety

**Safety Measures:**
- Error injection is simulation only - no actual hardware corruption
- Memory boundaries are respected during "overrun" simulation
- Timeouts are artificial delays, not actual hardware timeouts
- Recovery procedures are controlled and predictable

**Isolation:**
- Each device instance has independent error injection state
- Process isolation maintained during error scenarios
- System stability preserved under all error conditions

## Future Enhancements

### Planned Features

**Advanced Error Injection:**
- Custom error scenarios with user-defined probability distributions
- Temporal error patterns (burst errors, periodic failures)
- Correlated errors across multiple devices

**Enhanced Monitoring:**
- Real-time performance monitoring with configurable thresholds
- Event tracing integration with ftrace
- Advanced statistics with histogram data

**Hardware Simulation:**
- More realistic PCIe protocol simulation
- Power management state simulation
- Hot-plug event simulation

### Extension Points

**Plugin Architecture:**
- Loadable error injection modules
- Custom transfer pattern simulation
- Third-party monitoring integration

**Advanced Features:**
- NUMA awareness for multi-socket systems
- SR-IOV simulation for virtualization
- MSI-X interrupt simulation

---

**Professional Linux kernel module providing realistic PCIe device simulation with advanced error injection capabilities for comprehensive driver testing and validation.**