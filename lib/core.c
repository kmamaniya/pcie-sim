/*
 * PCIe Simulator Library - Core Implementation
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
 * Core library functions for cross-platform device management and operations.
 */

#include "api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    /* Windows-specific includes are in windows_sim.c */
    #include <windows.h>
#else
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>
    #include <sys/ioctl.h>
    #include <time.h>
    #include <pthread.h>
#endif

/* Internal handle structure */
struct pcie_sim_handle {
    int fd;
    int device_id;
    int is_simulation;  /* Flag to indicate simulation mode */
};

#ifdef _WIN32
/* Forward declarations for Windows implementations (defined in windows_sim.c) */
extern pcie_sim_error_t pcie_sim_open_impl(int device_id, pcie_sim_handle_t *handle);
extern pcie_sim_error_t pcie_sim_close_impl(pcie_sim_handle_t handle);
extern pcie_sim_error_t pcie_sim_transfer_impl(pcie_sim_handle_t handle, void *buffer,
                                              size_t size, uint32_t direction,
                                              uint64_t *latency_ns);
extern pcie_sim_error_t pcie_sim_get_stats_impl(pcie_sim_handle_t handle,
                                                struct pcie_sim_stats *stats);
extern pcie_sim_error_t pcie_sim_reset_stats_impl(pcie_sim_handle_t handle);
#else
/* Forward declarations for Linux implementations (defined in sim/linux_sim.c) */
extern pcie_sim_error_t pcie_sim_open_linux(int device_id, pcie_sim_handle_t *handle);
extern pcie_sim_error_t pcie_sim_close_linux(pcie_sim_handle_t handle);
extern pcie_sim_error_t pcie_sim_transfer_linux(pcie_sim_handle_t handle, void *buffer,
                                               size_t size, uint32_t direction,
                                               uint64_t *latency_ns);
extern pcie_sim_error_t pcie_sim_get_stats_linux(pcie_sim_handle_t handle,
                                                 struct pcie_sim_stats *stats);
extern pcie_sim_error_t pcie_sim_reset_stats_linux(pcie_sim_handle_t handle);
#endif

/*
 * Open a PCIe simulator device
 */
pcie_sim_error_t pcie_sim_open(int device_id, pcie_sim_handle_t *handle)
{
#ifdef _WIN32
    return pcie_sim_open_impl(device_id, handle);
#else
    return pcie_sim_open_linux(device_id, handle);
#endif
}

/*
 * Close a PCIe simulator device
 */
pcie_sim_error_t pcie_sim_close(pcie_sim_handle_t handle)
{
#ifdef _WIN32
    return pcie_sim_close_impl(handle);
#else
    return pcie_sim_close_linux(handle);
#endif
}

/*
 * Perform a DMA transfer
 */
pcie_sim_error_t pcie_sim_transfer(pcie_sim_handle_t handle,
                                 void *buffer,
                                 size_t size,
                                 uint32_t direction,
                                 uint64_t *latency_ns)
{
#ifdef _WIN32
    return pcie_sim_transfer_impl(handle, buffer, size, direction, latency_ns);
#else
    return pcie_sim_transfer_linux(handle, buffer, size, direction, latency_ns);
#endif
}

/*
 * Get device statistics
 */
pcie_sim_error_t pcie_sim_get_stats(pcie_sim_handle_t handle,
                                  struct pcie_sim_stats *stats)
{
#ifdef _WIN32
    return pcie_sim_get_stats_impl(handle, stats);
#else
    return pcie_sim_get_stats_linux(handle, stats);
#endif
}

/*
 * Reset device statistics
 */
pcie_sim_error_t pcie_sim_reset_stats(pcie_sim_handle_t handle)
{
#ifdef _WIN32
    return pcie_sim_reset_stats_impl(handle);
#else
    return pcie_sim_reset_stats_linux(handle);
#endif
}

#ifndef _WIN32
/* Forward declarations for Linux implementations (defined in sim/linux_sim.c) */
extern pcie_sim_error_t pcie_sim_open_linux(int device_id, pcie_sim_handle_t *handle);
extern pcie_sim_error_t pcie_sim_close_linux(pcie_sim_handle_t handle);
extern pcie_sim_error_t pcie_sim_transfer_linux(pcie_sim_handle_t handle, void *buffer,
                                               size_t size, uint32_t direction,
                                               uint64_t *latency_ns);
extern pcie_sim_error_t pcie_sim_get_stats_linux(pcie_sim_handle_t handle,
                                                 struct pcie_sim_stats *stats);
extern pcie_sim_error_t pcie_sim_reset_stats_linux(pcie_sim_handle_t handle);
#endif /* !_WIN32 */