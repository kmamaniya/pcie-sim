/*
 * PCIe Simulator - Common Definitions and Structures
 *
 * Copyright (c) 2025 Karan Mamaniya <kmamaniya@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef PCIE_SIM_COMMON_H
#define PCIE_SIM_COMMON_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/delay.h>
#include <linux/ktime.h>
#include <linux/random.h>

#define DRIVER_NAME "pcie_sim"
#define DRIVER_VERSION "1.0"
#define DEVICE_COUNT 1

/* IOCTL commands */
#define PCIE_SIM_IOC_MAGIC 'P'
#define PCIE_SIM_IOC_TRANSFER    _IOWR(PCIE_SIM_IOC_MAGIC, 1, struct pcie_sim_transfer_req)
#define PCIE_SIM_IOC_GET_STATS   _IOR(PCIE_SIM_IOC_MAGIC, 2, struct pcie_sim_stats)
#define PCIE_SIM_IOC_RESET_STATS _IO(PCIE_SIM_IOC_MAGIC, 3)
#define PCIE_SIM_IOC_SET_ERROR   _IOW(PCIE_SIM_IOC_MAGIC, 4, struct pcie_sim_error_config)

/* Error scenario constants */
#define PCIE_SIM_ERROR_SCENARIO_NONE        0
#define PCIE_SIM_ERROR_SCENARIO_TIMEOUT     1
#define PCIE_SIM_ERROR_SCENARIO_CORRUPTION  2
#define PCIE_SIM_ERROR_SCENARIO_OVERRUN     3

/* Device statistics structure */
struct pcie_sim_stats {
    atomic64_t total_transfers;
    atomic64_t total_bytes;
    atomic64_t total_errors;
    u64 avg_latency_ns;
    u64 min_latency_ns;
    u64 max_latency_ns;
};

/* Transfer request structure */
struct pcie_sim_transfer_req {
    void __user *buffer;
    size_t size;
    u32 direction;  /* 0=to_device, 1=from_device */
    u64 latency_ns; /* Returned latency */
};

/* Error configuration structure */
struct pcie_sim_error_config {
    u32 scenario;           /* Error scenario (0-3) */
    u32 probability;        /* Error probability in 0.01% units (0-10000) */
    u32 recovery_time_ms;   /* Recovery time after error */
    u32 flags;              /* Configuration flags */
};

/* Ring buffer descriptor */
struct pcie_sim_ring_desc {
    u64 buffer_addr;    /* Physical address of buffer */
    u32 length;         /* Transfer length */
    u32 flags;          /* Control flags */
    u64 timestamp;      /* Submission timestamp */
    u32 status;         /* Completion status */
    u32 reserved;
};

/* Ring buffer structure */
struct pcie_sim_ring {
    struct pcie_sim_ring_desc *descriptors;
    dma_addr_t desc_dma_addr;
    u32 size;           /* Number of descriptors */
    u32 head;           /* Producer index */
    u32 tail;           /* Consumer index */
    atomic_t count;     /* Current entries */
    spinlock_t lock;    /* Ring protection */

    /* Statistics */
    atomic64_t submissions;
    atomic64_t completions;
    atomic64_t overruns;
};

/* Main device structure */
struct pcie_sim_device {
    struct platform_device *pdev;
    struct cdev cdev;
    struct device *dev;
    dev_t devt;
    struct mutex mutex;

    /* Statistics */
    struct pcie_sim_stats stats;

    /* Proc entries */
    struct proc_dir_entry *proc_dir;
    struct proc_dir_entry *proc_stats;

    /* MMIO simulation (BAR regions) */
    void *bar0_virt;
    size_t bar0_size;

    /* Ring buffers for DMA */
    struct pcie_sim_ring tx_ring;
    struct pcie_sim_ring rx_ring;

    /* Interrupt simulation */
    atomic_t pending_interrupts;
    atomic_t dma_active;

    /* Error injection */
    bool simulate_errors;
    u32 fault_injection_rate;
    u32 error_scenario;         /* 0=none, 1=timeout, 2=corruption, 3=overrun */
    u32 error_probability;      /* Error probability in 0.01% units */
    u32 error_recovery_time_ms; /* Recovery time after error */

    bool enabled;
    int device_id;
};

/* Global driver state */
extern struct {
    struct class *class;
    dev_t devt_base;
    int major;
    struct pcie_sim_device *devices[DEVICE_COUNT];
} driver_state;

/* Function prototypes */
int pcie_sim_char_init(struct pcie_sim_device *dev);
void pcie_sim_char_cleanup(struct pcie_sim_device *dev);
long pcie_sim_char_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

int pcie_sim_proc_init(struct pcie_sim_device *dev);
void pcie_sim_proc_cleanup(struct pcie_sim_device *dev);

int pcie_sim_dma_transfer(struct pcie_sim_device *dev, struct pcie_sim_transfer_req *req);

int pcie_sim_mmio_init(struct pcie_sim_device *dev);
void pcie_sim_mmio_cleanup(struct pcie_sim_device *dev);
u32 pcie_sim_mmio_read32(struct pcie_sim_device *dev, u32 offset);
void pcie_sim_mmio_write32(struct pcie_sim_device *dev, u32 offset, u32 value);
void pcie_sim_mmio_update_dma(struct pcie_sim_device *dev,
                             struct pcie_sim_transfer_req *req, bool success);

int pcie_sim_ring_init(struct pcie_sim_device *dev);
void pcie_sim_ring_cleanup(struct pcie_sim_device *dev);

int pcie_sim_interrupt_init(struct pcie_sim_device *dev);
void pcie_sim_interrupt_cleanup(struct pcie_sim_device *dev);
void pcie_sim_trigger_interrupt(struct pcie_sim_device *dev, u32 irq_mask);

#endif /* PCIE_SIM_COMMON_H */