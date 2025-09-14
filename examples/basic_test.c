/*
 * PCIe Simulator Test Program
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
 * Simple test program demonstrating the PCIe simulator functionality.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <inttypes.h>
#include "../lib/pcie_sim.h"

int main(int argc, char *argv[])
{
    pcie_sim_handle_t handle;
    pcie_sim_error_t ret;
    struct pcie_sim_stats stats;
    char buffer[4096];
    uint64_t latency;
    int device_id = 0;
    int i;

    printf("PCIe Simulator Test Program\n");
    printf("Copyright (c) 2025 Karan Mamaniya\n\n");

    /* Parse command line */
    if (argc > 1) {
        device_id = atoi(argv[1]);
    }

    /* Open device */
    printf("Opening device %d...\n", device_id);
    ret = pcie_sim_open(device_id, &handle);
    if (ret != PCIE_SIM_SUCCESS) {
        printf("Failed to open device: %s\n", pcie_sim_error_string(ret));
        return 1;
    }
    printf("Device opened successfully\n");

    /* Reset statistics */
    pcie_sim_reset_stats(handle);

    /* Test data transfers */
    printf("\nTesting data transfers...\n");

    for (i = 0; i < 10; i++) {
        /* Fill buffer with test pattern */
        memset(buffer, 0x55 + i, sizeof(buffer));

        /* Write to device */
        ret = pcie_sim_transfer(handle, buffer, sizeof(buffer),
                              PCIE_SIM_TO_DEVICE, &latency);
        if (ret != PCIE_SIM_SUCCESS) {
            printf("Transfer %d failed: %s\n", i, pcie_sim_error_string(ret));
            continue;
        }

        printf("Transfer %d: %zu bytes, latency: %" PRIu64 " ns (%.2f μs)\n",
               i + 1, sizeof(buffer), latency, latency / 1000.0);

        /* Small delay between transfers */
        usleep(10000);
    }

    /* Read back test */
    printf("\nTesting read-back...\n");
    memset(buffer, 0, sizeof(buffer));
    ret = pcie_sim_transfer(handle, buffer, 1024, PCIE_SIM_FROM_DEVICE, &latency);
    if (ret == PCIE_SIM_SUCCESS) {
        printf("Read-back successful: %zu bytes, latency: %" PRIu64 " ns (%.2f μs)\n",
               sizeof(buffer), latency, latency / 1000.0);
        printf("First few bytes: %02x %02x %02x %02x\n",
               buffer[0], buffer[1], buffer[2], buffer[3]);
    } else {
        printf("Read-back failed: %s\n", pcie_sim_error_string(ret));
    }

    /* Get statistics */
    printf("\nDevice statistics:\n");
    ret = pcie_sim_get_stats(handle, &stats);
    if (ret == PCIE_SIM_SUCCESS) {
        printf("  Total transfers: %" PRIu64 "\n", stats.total_transfers);
        printf("  Total bytes: %" PRIu64 " (%" PRIu64 " KB)\n",
               stats.total_bytes, stats.total_bytes / 1024);
        printf("  Total errors: %" PRIu64 "\n", stats.total_errors);
        printf("  Average latency: %" PRIu64 " ns (%.2f μs)\n",
               stats.avg_latency_ns, stats.avg_latency_ns / 1000.0);
        printf("  Min latency: %" PRIu64 " ns (%.2f μs)\n",
               stats.min_latency_ns, stats.min_latency_ns / 1000.0);
        printf("  Max latency: %" PRIu64 " ns (%.2f μs)\n",
               stats.max_latency_ns, stats.max_latency_ns / 1000.0);

        if (stats.total_transfers > 0) {
            double avg_throughput = (double)stats.total_bytes /
                                  (stats.avg_latency_ns / 1000000000.0) /
                                  (1024 * 1024);
            printf("  Average throughput: %.2f MB/s\n", avg_throughput);
        }
    } else {
        printf("Failed to get statistics: %s\n", pcie_sim_error_string(ret));
    }

    /* Close device */
    pcie_sim_close(handle);
    printf("\nTest completed successfully\n");

    return 0;
}