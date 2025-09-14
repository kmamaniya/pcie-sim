/*
 * PCIe Simulator - Memory-Mapped I/O (BAR) Simulation
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
 *
 * This module simulates PCIe BAR (Base Address Register) memory-mapped I/O
 * regions for control registers and status reporting, as required for OTPU
 * simulation.
 */

#include "common.h"
#include <linux/io.h>

/* BAR0 - Control registers (4KB) */
#define BAR0_SIZE 0x1000

/* Control register offsets */
#define REG_DEVICE_ID      0x000  /* Device identification */
#define REG_STATUS         0x004  /* Device status */
#define REG_CONTROL        0x008  /* Device control */
#define REG_DMA_ADDR_LO    0x010  /* DMA address low 32 bits */
#define REG_DMA_ADDR_HI    0x014  /* DMA address high 32 bits */
#define REG_DMA_SIZE       0x018  /* DMA transfer size */
#define REG_DMA_CONTROL    0x01C  /* DMA control */
#define REG_INTERRUPT_STATUS 0x020 /* Interrupt status */
#define REG_INTERRUPT_ENABLE 0x024 /* Interrupt enable */
#define REG_PERF_LATENCY   0x030  /* Last transfer latency */
#define REG_PERF_COUNT     0x034  /* Transfer counter */
#define REG_ERROR_STATUS   0x040  /* Error status */
#define REG_ERROR_INJECT   0x044  /* Error injection control */

/* Status register bits */
#define STATUS_DEVICE_READY   BIT(0)
#define STATUS_DMA_BUSY       BIT(1)
#define STATUS_ERROR          BIT(2)
#define STATUS_INTERRUPT_PENDING BIT(3)

/* Control register bits */
#define CONTROL_DEVICE_ENABLE BIT(0)
#define CONTROL_DMA_START     BIT(1)
#define CONTROL_DMA_RESET     BIT(2)
#define CONTROL_IRQ_ENABLE    BIT(3)

/* DMA control register bits */
#define DMA_CONTROL_DIRECTION BIT(0)  /* 0=TO_DEVICE, 1=FROM_DEVICE */
#define DMA_CONTROL_ENABLE    BIT(1)
#define DMA_CONTROL_INTERRUPT BIT(2)

/* Interrupt status/enable bits */
#define IRQ_DMA_COMPLETE      BIT(0)
#define IRQ_DMA_ERROR         BIT(1)
#define IRQ_BUFFER_OVERRUN    BIT(2)
#define IRQ_DEVICE_ERROR      BIT(3)

/*
 * Initialize BAR memory-mapped I/O simulation
 */
int pcie_sim_mmio_init(struct pcie_sim_device *dev)
{
    pr_debug("Initializing MMIO simulation for device %d\n", dev->device_id);

    /* Allocate BAR0 memory region */
    dev->bar0_virt = kzalloc(BAR0_SIZE, GFP_KERNEL);
    if (!dev->bar0_virt) {
        pr_err("Failed to allocate BAR0 memory\n");
        return -ENOMEM;
    }

    dev->bar0_size = BAR0_SIZE;

    /* Initialize control registers with default values */
    writel(0x1234ABCD, dev->bar0_virt + REG_DEVICE_ID);  /* Fake device ID */
    writel(STATUS_DEVICE_READY, dev->bar0_virt + REG_STATUS);
    writel(CONTROL_DEVICE_ENABLE, dev->bar0_virt + REG_CONTROL);
    writel(0, dev->bar0_virt + REG_DMA_CONTROL);
    writel(IRQ_DMA_COMPLETE | IRQ_DMA_ERROR, dev->bar0_virt + REG_INTERRUPT_ENABLE);

    pr_info("MMIO simulation initialized: BAR0=%p size=%zu\n",
           dev->bar0_virt, dev->bar0_size);

    return 0;
}

/*
 * Cleanup BAR memory-mapped I/O simulation
 */
void pcie_sim_mmio_cleanup(struct pcie_sim_device *dev)
{
    if (!dev)
        return;

    pr_debug("Cleaning up MMIO simulation for device %d\n", dev->device_id);

    if (dev->bar0_virt) {
        kfree(dev->bar0_virt);
        dev->bar0_virt = NULL;
    }

    dev->bar0_size = 0;
}

/*
 * Read from control register
 */
u32 pcie_sim_mmio_read32(struct pcie_sim_device *dev, u32 offset)
{
    u32 value;

    if (!dev->bar0_virt || offset >= dev->bar0_size) {
        pr_warn("Invalid MMIO read: offset=0x%x\n", offset);
        return 0xFFFFFFFF;
    }

    value = readl(dev->bar0_virt + offset);

    /* Handle special register reads */
    switch (offset) {
    case REG_STATUS:
        /* Update dynamic status bits */
        value &= ~(STATUS_DMA_BUSY | STATUS_INTERRUPT_PENDING);
        if (atomic_read(&dev->dma_active))
            value |= STATUS_DMA_BUSY;
        if (atomic_read(&dev->pending_interrupts))
            value |= STATUS_INTERRUPT_PENDING;
        break;

    case REG_PERF_LATENCY:
        /* Return last measured latency */
        value = (u32)(dev->stats.avg_latency_ns / 1000);  /* Convert to μs */
        break;

    case REG_PERF_COUNT:
        /* Return transfer count */
        value = (u32)atomic64_read(&dev->stats.total_transfers);
        break;
    }

    pr_debug("MMIO read: offset=0x%03x value=0x%08x\n", offset, value);
    return value;
}

/*
 * Write to control register
 */
void pcie_sim_mmio_write32(struct pcie_sim_device *dev, u32 offset, u32 value)
{
    if (!dev->bar0_virt || offset >= dev->bar0_size) {
        pr_warn("Invalid MMIO write: offset=0x%x value=0x%x\n", offset, value);
        return;
    }

    pr_debug("MMIO write: offset=0x%03x value=0x%08x\n", offset, value);

    /* Handle special register writes */
    switch (offset) {
    case REG_CONTROL:
        /* Handle control register changes */
        if (value & CONTROL_DMA_START) {
            pr_debug("DMA start triggered via MMIO\n");
            /* Trigger DMA operation */
        }
        if (value & CONTROL_DMA_RESET) {
            pr_debug("DMA reset triggered via MMIO\n");
            atomic_set(&dev->dma_active, 0);
            value &= ~CONTROL_DMA_RESET;  /* Self-clearing bit */
        }
        break;

    case REG_INTERRUPT_STATUS:
        /* Writing to interrupt status clears the bits */
        {
            u32 current = readl(dev->bar0_virt + offset);
            u32 new_status = current & ~value;  /* Clear written bits */
            writel(new_status, dev->bar0_virt + offset);

            /* Update pending interrupt count */
            if (new_status == 0)
                atomic_set(&dev->pending_interrupts, 0);
            return;  /* Don't write the original value */
        }

    case REG_ERROR_INJECT:
        /* Handle error injection */
        if (value & 0xFF) {
            dev->fault_injection_rate = value & 0xFF;
            dev->simulate_errors = true;
            pr_debug("Error injection enabled: rate=1/%u\n", dev->fault_injection_rate);
        } else {
            dev->simulate_errors = false;
            pr_debug("Error injection disabled\n");
        }
        break;
    }

    writel(value, dev->bar0_virt + offset);
}

/*
 * Update DMA-related registers after transfer
 */
void pcie_sim_mmio_update_dma(struct pcie_sim_device *dev,
                             struct pcie_sim_transfer_req *req,
                             bool success)
{
    u32 status, irq_status;

    if (!dev->bar0_virt)
        return;

    /* Update status register */
    status = readl(dev->bar0_virt + REG_STATUS);
    status &= ~STATUS_DMA_BUSY;

    if (!success) {
        status |= STATUS_ERROR;
    }

    writel(status, dev->bar0_virt + REG_STATUS);

    /* Update interrupt status */
    irq_status = readl(dev->bar0_virt + REG_INTERRUPT_STATUS);

    if (success) {
        irq_status |= IRQ_DMA_COMPLETE;
    } else {
        irq_status |= IRQ_DMA_ERROR;
    }

    writel(irq_status, dev->bar0_virt + REG_INTERRUPT_STATUS);

    /* Update performance registers */
    if (success && req) {
        writel((u32)(req->latency_ns / 1000), dev->bar0_virt + REG_PERF_LATENCY);
        writel((u32)atomic64_read(&dev->stats.total_transfers),
               dev->bar0_virt + REG_PERF_COUNT);
    }

    /* Set interrupt pending flag */
    atomic_set(&dev->pending_interrupts, 1);

    pr_debug("MMIO DMA update: success=%d, irq_status=0x%x\n", success, irq_status);
}