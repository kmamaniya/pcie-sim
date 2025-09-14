/*
 * PCIe Simulator - Ring Buffer DMA Protocol
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
 * This module implements ring buffer DMA protocol for high-throughput
 * transfers as required for OTPU simulation.
 */

#include "common.h"
#include <linux/dma-mapping.h>

#define RING_SIZE 256  /* Number of descriptors per ring */

/*
 * Initialize a single ring buffer
 */
static int init_ring(struct pcie_sim_device *dev, struct pcie_sim_ring *ring, const char *name)
{
    size_t desc_size;

    pr_debug("Initializing %s ring buffer\n", name);

    /* Initialize ring structure */
    ring->size = RING_SIZE;
    ring->head = 0;
    ring->tail = 0;
    atomic_set(&ring->count, 0);
    spin_lock_init(&ring->lock);

    /* Reset statistics */
    atomic64_set(&ring->submissions, 0);
    atomic64_set(&ring->completions, 0);
    atomic64_set(&ring->overruns, 0);

    /* Allocate DMA-coherent memory for descriptors */
    desc_size = ring->size * sizeof(struct pcie_sim_ring_desc);
    ring->descriptors = dma_alloc_coherent(&dev->pdev->dev, desc_size,
                                         &ring->desc_dma_addr, GFP_KERNEL);
    if (!ring->descriptors) {
        pr_err("Failed to allocate %s ring descriptors\n", name);
        return -ENOMEM;
    }

    /* Initialize all descriptors */
    memset(ring->descriptors, 0, desc_size);

    pr_info("%s ring initialized: %u descriptors at %p (DMA: %pad)\n",
           name, ring->size, ring->descriptors, &ring->desc_dma_addr);

    return 0;
}

/*
 * Cleanup a single ring buffer
 */
static void cleanup_ring(struct pcie_sim_device *dev, struct pcie_sim_ring *ring)
{
    size_t desc_size;

    if (!ring->descriptors)
        return;

    desc_size = ring->size * sizeof(struct pcie_sim_ring_desc);
    dma_free_coherent(&dev->pdev->dev, desc_size, ring->descriptors, ring->desc_dma_addr);

    ring->descriptors = NULL;
    ring->desc_dma_addr = 0;
}

/*
 * Get number of used entries in ring
 */
u32 pcie_sim_ring_count(struct pcie_sim_ring *ring)
{
    return atomic_read(&ring->count);
}

/*
 * Get number of available entries in ring
 */
u32 pcie_sim_ring_space(struct pcie_sim_ring *ring)
{
    return ring->size - atomic_read(&ring->count);
}

/*
 * Submit a descriptor to the ring
 */
int pcie_sim_ring_submit(struct pcie_sim_ring *ring, u64 buffer_addr,
                        u32 length, u32 flags)
{
    unsigned long irq_flags;
    u32 next_head;

    spin_lock_irqsave(&ring->lock, irq_flags);

    /* Check for space */
    if (atomic_read(&ring->count) >= ring->size) {
        atomic64_inc(&ring->overruns);
        spin_unlock_irqrestore(&ring->lock, irq_flags);
        pr_warn("Ring buffer overrun\n");
        return -ENOSPC;
    }

    /* Fill descriptor */
    ring->descriptors[ring->head].buffer_addr = buffer_addr;
    ring->descriptors[ring->head].length = length;
    ring->descriptors[ring->head].flags = flags;
    ring->descriptors[ring->head].timestamp = ktime_get_ns();
    ring->descriptors[ring->head].status = 0;  /* Pending */

    /* Advance head pointer */
    next_head = (ring->head + 1) % ring->size;
    ring->head = next_head;

    /* Update count */
    atomic_inc(&ring->count);
    atomic64_inc(&ring->submissions);

    spin_unlock_irqrestore(&ring->lock, irq_flags);

    pr_debug("Ring submit: addr=%llx len=%u flags=%x count=%u\n",
            buffer_addr, length, flags, atomic_read(&ring->count));

    return 0;
}

/*
 * Complete a descriptor from the ring
 */
int pcie_sim_ring_complete(struct pcie_sim_ring *ring, u32 *length,
                          u64 *latency_ns, u32 status)
{
    unsigned long irq_flags;
    struct pcie_sim_ring_desc *desc;
    u64 completion_time;

    spin_lock_irqsave(&ring->lock, irq_flags);

    /* Check if ring has entries */
    if (atomic_read(&ring->count) == 0) {
        spin_unlock_irqrestore(&ring->lock, irq_flags);
        return -ENODATA;
    }

    /* Get descriptor to complete */
    desc = &ring->descriptors[ring->tail];
    completion_time = ktime_get_ns();

    /* Fill return values */
    if (length)
        *length = desc->length;
    if (latency_ns)
        *latency_ns = completion_time - desc->timestamp;

    /* Mark as completed */
    desc->status = status;

    /* Advance tail pointer */
    ring->tail = (ring->tail + 1) % ring->size;

    /* Update count */
    atomic_dec(&ring->count);
    atomic64_inc(&ring->completions);

    spin_unlock_irqrestore(&ring->lock, irq_flags);

    pr_debug("Ring complete: len=%u status=%u latency=%llu ns count=%u\n",
            desc->length, status, completion_time - desc->timestamp,
            atomic_read(&ring->count));

    return 0;
}

/*
 * Initialize ring buffers for a device
 */
int pcie_sim_ring_init(struct pcie_sim_device *dev)
{
    int ret;

    pr_debug("Initializing ring buffers for device %d\n", dev->device_id);

    /* Initialize TX ring */
    ret = init_ring(dev, &dev->tx_ring, "TX");
    if (ret) {
        pr_err("Failed to initialize TX ring: %d\n", ret);
        return ret;
    }

    /* Initialize RX ring */
    ret = init_ring(dev, &dev->rx_ring, "RX");
    if (ret) {
        pr_err("Failed to initialize RX ring: %d\n", ret);
        cleanup_ring(dev, &dev->tx_ring);
        return ret;
    }

    pr_info("Ring buffers initialized for device %d\n", dev->device_id);
    return 0;
}

/*
 * Cleanup ring buffers for a device
 */
void pcie_sim_ring_cleanup(struct pcie_sim_device *dev)
{
    if (!dev)
        return;

    pr_debug("Cleaning up ring buffers for device %d\n", dev->device_id);

    cleanup_ring(dev, &dev->rx_ring);
    cleanup_ring(dev, &dev->tx_ring);

    pr_debug("Ring buffer cleanup complete for device %d\n", dev->device_id);
}