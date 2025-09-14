/*
 * PCIe Simulator Library - Main API
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
 * Main API functions for the PCIe simulator library.
 */

#ifndef PCIE_SIM_API_H
#define PCIE_SIM_API_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Open a PCIe simulator device
 * @param device_id Device ID (0, 1, 2, ...)
 * @param handle Pointer to store device handle
 * @return Error code
 */
pcie_sim_error_t pcie_sim_open(int device_id, pcie_sim_handle_t *handle);

/**
 * Close a PCIe simulator device
 * @param handle Device handle
 * @return Error code
 */
pcie_sim_error_t pcie_sim_close(pcie_sim_handle_t handle);

/**
 * Perform a DMA transfer
 * @param handle Device handle
 * @param buffer User buffer
 * @param size Transfer size in bytes
 * @param direction Transfer direction (TO_DEVICE or FROM_DEVICE)
 * @param latency_ns Pointer to store transfer latency (optional)
 * @return Error code
 */
pcie_sim_error_t pcie_sim_transfer(pcie_sim_handle_t handle,
                                 void *buffer,
                                 size_t size,
                                 uint32_t direction,
                                 uint64_t *latency_ns);

/**
 * Get device statistics
 * @param handle Device handle
 * @param stats Pointer to statistics structure
 * @return Error code
 */
pcie_sim_error_t pcie_sim_get_stats(pcie_sim_handle_t handle,
                                   struct pcie_sim_stats *stats);

/**
 * Reset device statistics
 * @param handle Device handle
 * @return Error code
 */
pcie_sim_error_t pcie_sim_reset_stats(pcie_sim_handle_t handle);

/**
 * Convert error code to string
 * @param error Error code
 * @return Error description string
 */
const char *pcie_sim_error_string(pcie_sim_error_t error);

#ifdef __cplusplus
}
#endif

#endif /* PCIE_SIM_API_H */