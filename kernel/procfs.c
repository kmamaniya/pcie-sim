/*
 * PCIe Simulator - Proc Filesystem Interface
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
 * This module provides /proc filesystem entries for monitoring device statistics.
 * It demonstrates kernel-to-userspace information export using seq_file interface.
 */

#include "common.h"

/*
 * Show device statistics in /proc/pcie_simX/stats
 */
static int proc_stats_show(struct seq_file *m, void *v)
{
    struct pcie_sim_device *dev = m->private;
    u64 total_transfers, total_bytes, total_errors;
    double avg_throughput_mbps = 0.0;

    if (!dev) {
        seq_puts(m, "Error: No device context\n");
        return 0;
    }

    /* Read statistics atomically */
    total_transfers = atomic64_read(&dev->stats.total_transfers);
    total_bytes = atomic64_read(&dev->stats.total_bytes);
    total_errors = atomic64_read(&dev->stats.total_errors);

    /* Calculate average throughput if we have latency data */
    if (dev->stats.avg_latency_ns > 0 && total_bytes > 0) {
        /* Convert to Mbps: (bytes * 8 bits/byte) / (latency_ns / 1e9 s/ns) / 1e6 */
        avg_throughput_mbps = (double)(total_bytes * 8) /
                             ((dev->stats.avg_latency_ns / 1000000000.0) * total_transfers) / 1000000.0;
    }

    /* Display statistics in human-readable format */
    seq_printf(m, "PCIe Simulator Device %d Statistics\n", dev->device_id);
    seq_puts(m, "===================================\n\n");

    seq_puts(m, "Transfer Summary:\n");
    seq_printf(m, "  Total Transfers:     %llu\n", total_transfers);
    seq_printf(m, "  Total Bytes:         %llu (%llu KB, %llu MB)\n",
              total_bytes, total_bytes / 1024, total_bytes / (1024 * 1024));
    seq_printf(m, "  Total Errors:        %llu\n", total_errors);

    if (total_transfers > 0) {
        seq_printf(m, "  Average Transfer Size: %llu bytes\n",
                  total_bytes / total_transfers);
        seq_printf(m, "  Error Rate:          %.2f%%\n",
                  (double)total_errors * 100.0 / (total_transfers + total_errors));
    }

    seq_puts(m, "\nLatency Statistics:\n");
    seq_printf(m, "  Average Latency:     %llu ns (%.2f µs)\n",
              dev->stats.avg_latency_ns, dev->stats.avg_latency_ns / 1000.0);

    if (dev->stats.min_latency_ns > 0) {
        seq_printf(m, "  Minimum Latency:     %llu ns (%.2f µs)\n",
                  dev->stats.min_latency_ns, dev->stats.min_latency_ns / 1000.0);
    } else {
        seq_puts(m, "  Minimum Latency:     Not measured\n");
    }

    seq_printf(m, "  Maximum Latency:     %llu ns (%.2f µs)\n",
              dev->stats.max_latency_ns, dev->stats.max_latency_ns / 1000.0);

    if (dev->stats.max_latency_ns > dev->stats.min_latency_ns && dev->stats.min_latency_ns > 0) {
        u64 jitter_ns = dev->stats.max_latency_ns - dev->stats.min_latency_ns;
        seq_printf(m, "  Jitter (max-min):    %llu ns (%.2f µs)\n",
                  jitter_ns, jitter_ns / 1000.0);
    }

    seq_puts(m, "\nPerformance Metrics:\n");
    if (avg_throughput_mbps > 0) {
        seq_printf(m, "  Average Throughput:  %.2f Mbps (%.2f MB/s)\n",
                  avg_throughput_mbps, avg_throughput_mbps / 8.0);
    } else {
        seq_puts(m, "  Average Throughput:  Not calculated\n");
    }

    seq_puts(m, "\nDevice Status:\n");
    seq_printf(m, "  Device Enabled:      %s\n", dev->enabled ? "Yes" : "No");
    seq_printf(m, "  Device File:         /dev/pcie_sim%d\n", dev->device_id);

    return 0;
}

/*
 * Open function for statistics proc file
 */
static int proc_stats_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_stats_show, PDE_DATA(inode));
}

/* Proc file operations structure */
static const struct proc_ops proc_stats_ops = {
    .proc_open = proc_stats_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

/*
 * Initialize proc filesystem interface for a device
 */
int pcie_sim_proc_init(struct pcie_sim_device *dev)
{
    char dir_name[32];

    if (!dev)
        return -EINVAL;

    pr_debug("Initializing proc interface for device %d\n", dev->device_id);

    /* Create device directory: /proc/pcie_simX */
    snprintf(dir_name, sizeof(dir_name), "pcie_sim%d", dev->device_id);
    dev->proc_dir = proc_mkdir(dir_name, NULL);
    if (!dev->proc_dir) {
        pr_err("Failed to create proc directory: %s\n", dir_name);
        return -ENOMEM;
    }

    /* Create statistics file: /proc/pcie_simX/stats */
    dev->proc_stats = proc_create_data("stats", 0644, dev->proc_dir,
                                      &proc_stats_ops, dev);
    if (!dev->proc_stats) {
        pr_err("Failed to create proc stats file\n");
        proc_remove(dev->proc_dir);
        dev->proc_dir = NULL;
        return -ENOMEM;
    }

    pr_info("Proc interface created: /proc/%s/stats\n", dir_name);
    return 0;
}

/*
 * Cleanup proc filesystem interface for a device
 */
void pcie_sim_proc_cleanup(struct pcie_sim_device *dev)
{
    if (!dev)
        return;

    pr_debug("Cleaning up proc interface for device %d\n", dev->device_id);

    /* Remove statistics file */
    if (dev->proc_stats) {
        proc_remove(dev->proc_stats);
        dev->proc_stats = NULL;
    }

    /* Remove device directory */
    if (dev->proc_dir) {
        proc_remove(dev->proc_dir);
        dev->proc_dir = NULL;
    }

    pr_debug("Proc interface cleanup complete for device %d\n", dev->device_id);
}