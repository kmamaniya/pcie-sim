/*
 * PCIe Simulator Library - Type Definitions
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
 * Common type definitions and constants shared between library components.
 */

#ifndef PCIE_SIM_TYPES_H
#define PCIE_SIM_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Error codes */
typedef enum {
    PCIE_SIM_SUCCESS = 0,
    PCIE_SIM_ERROR_DEVICE = -1,
    PCIE_SIM_ERROR_PARAM = -2,
    PCIE_SIM_ERROR_MEMORY = -3,
    PCIE_SIM_ERROR_TIMEOUT = -4,
    PCIE_SIM_ERROR_SYSTEM = -5
} pcie_sim_error_t;

/* Device handle (opaque pointer) */
typedef struct pcie_sim_handle *pcie_sim_handle_t;

/* Transfer directions */
#define PCIE_SIM_TO_DEVICE   0
#define PCIE_SIM_FROM_DEVICE 1

/* Device statistics structure */
struct pcie_sim_stats {
    uint64_t total_transfers;
    uint64_t total_bytes;
    uint64_t total_errors;
    uint64_t avg_latency_ns;
    uint64_t min_latency_ns;
    uint64_t max_latency_ns;
};

/* IOCTL definitions (must match kernel) */
#define PCIE_SIM_IOC_MAGIC 'P'

struct pcie_sim_transfer_req {
    void *buffer;
    size_t size;
    uint32_t direction;
    uint64_t latency_ns;
};

#define PCIE_SIM_IOC_TRANSFER    _IOWR(PCIE_SIM_IOC_MAGIC, 1, struct pcie_sim_transfer_req)
#define PCIE_SIM_IOC_GET_STATS   _IOR(PCIE_SIM_IOC_MAGIC, 2, struct pcie_sim_stats)
#define PCIE_SIM_IOC_RESET_STATS _IO(PCIE_SIM_IOC_MAGIC, 3)

#ifdef __cplusplus
}
#endif

#endif /* PCIE_SIM_TYPES_H */