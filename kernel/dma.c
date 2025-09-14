/*
 * PCIe Simulator - DMA Transfer Simulation
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
 * This module simulates DMA transfers with realistic timing and validation.
 * It demonstrates proper memory handling and performance measurement.
 */

#include "common.h"

/* Transfer size limits */
#define MIN_TRANSFER_SIZE 1
#define MAX_TRANSFER_SIZE (1024 * 1024)  /* 1MB max */

/*
 * Validate transfer request parameters
 */
static int validate_transfer_request(struct pcie_sim_transfer_req *req)
{
    /* Check buffer pointer */
    if (!req->buffer) {
        pr_debug("Invalid buffer pointer\n");
        return -EINVAL;
    }

    /* Check transfer size */
    if (req->size < MIN_TRANSFER_SIZE || req->size > MAX_TRANSFER_SIZE) {
        pr_debug("Invalid transfer size: %zu (min: %d, max: %d)\n",
                req->size, MIN_TRANSFER_SIZE, MAX_TRANSFER_SIZE);
        return -EINVAL;
    }

    /* Check direction */
    if (req->direction > 1) {
        pr_debug("Invalid transfer direction: %u\n", req->direction);
        return -EINVAL;
    }

    return 0;
}

/*
 * Update device statistics after a transfer
 */
static void update_transfer_stats(struct pcie_sim_device *dev,
                                struct pcie_sim_transfer_req *req,
                                u64 latency_ns, bool success)
{
    if (success) {
        /* Update counters */
        atomic64_inc(&dev->stats.total_transfers);
        atomic64_add(req->size, &dev->stats.total_bytes);

        /* Update latency statistics */
        if (dev->stats.min_latency_ns == 0 || latency_ns < dev->stats.min_latency_ns)
            dev->stats.min_latency_ns = latency_ns;

        if (latency_ns > dev->stats.max_latency_ns)
            dev->stats.max_latency_ns = latency_ns;

        /* Calculate running average latency */
        dev->stats.avg_latency_ns = (dev->stats.avg_latency_ns + latency_ns) / 2;

    } else {
        /* Update error counter */
        atomic64_inc(&dev->stats.total_errors);
    }
}

/*
 * Simulate realistic PCIe transfer latency
 * Real PCIe transfers have base latency + size-dependent component
 */
static void simulate_transfer_delay(size_t size)
{
    unsigned int base_delay_us = 10;  /* Base latency: 10µs */
    unsigned int size_delay_us = size / 1024;  /* ~1µs per KB */

    /* Add some randomness to simulate real-world variation */
    unsigned int jitter_us = get_random_u32_below(20);

    unsigned int total_delay = base_delay_us + size_delay_us + jitter_us;

    /* Use usleep_range for delays > 10µs */
    if (total_delay > 10)
        usleep_range(total_delay, total_delay + 10);
    else
        udelay(total_delay);
}

/*
 * Perform DMA transfer simulation
 */
int pcie_sim_dma_transfer(struct pcie_sim_device *dev,
                         struct pcie_sim_transfer_req *req)
{
    ktime_t start_time, end_time;
    u64 latency_ns;
    void *kernel_buf = NULL;
    int ret;

    /* Validate request parameters */
    ret = validate_transfer_request(req);
    if (ret)
        goto error_exit;

    /* Allocate temporary kernel buffer */
    kernel_buf = kzalloc(req->size, GFP_KERNEL);
    if (!kernel_buf) {
        pr_err("Failed to allocate kernel buffer of size %zu\n", req->size);
        ret = -ENOMEM;
        goto error_exit;
    }

    /* Start timing the transfer */
    start_time = ktime_get();

    /* Simulate the actual DMA operation */
    if (req->direction == 0) {
        /* TO_DEVICE: Copy data from userspace to kernel buffer */
        pr_debug("DMA TO_DEVICE: %zu bytes\n", req->size);

        if (copy_from_user(kernel_buf, req->buffer, req->size)) {
            pr_err("Failed to copy data from userspace\n");
            ret = -EFAULT;
            goto error_cleanup;
        }

        /* Simulate hardware processing the data */
        simulate_transfer_delay(req->size);

    } else {
        /* FROM_DEVICE: Fill kernel buffer and copy to userspace */
        pr_debug("DMA FROM_DEVICE: %zu bytes\n", req->size);

        /* Simulate hardware filling buffer with data pattern */
        memset(kernel_buf, 0xAA, req->size);
        simulate_transfer_delay(req->size);

        if (copy_to_user(req->buffer, kernel_buf, req->size)) {
            pr_err("Failed to copy data to userspace\n");
            ret = -EFAULT;
            goto error_cleanup;
        }
    }

    /* End timing and calculate latency */
    end_time = ktime_get();
    latency_ns = ktime_to_ns(ktime_sub(end_time, start_time));

    /* Update statistics */
    update_transfer_stats(dev, req, latency_ns, true);

    /* Return latency to userspace */
    req->latency_ns = latency_ns;

    pr_debug("Transfer completed: %zu bytes in %llu ns (%.2f µs)\n",
            req->size, latency_ns, latency_ns / 1000.0);

    kfree(kernel_buf);
    return 0;

error_cleanup:
    kfree(kernel_buf);
error_exit:
    /* Update error statistics */
    update_transfer_stats(dev, req, 0, false);
    return ret;
}