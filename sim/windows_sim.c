/*
 * PCIe Simulator - Windows Simulation Backend
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
 * Windows-specific simulation backend using memory-mapped simulation
 * and high-resolution timing.
 */

#ifdef _WIN32

#include "api.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Windows-specific includes for high-resolution timing */
#pragma comment(lib, "kernel32.lib")

/* Maximum number of simulated devices */
#define MAX_DEVICES 8

/* Simulated device state */
struct windows_device_state {
    BOOL active;
    HANDLE mutex;
    struct pcie_sim_stats stats;
    LARGE_INTEGER frequency;
    char device_name[64];
};

/* Global device state array */
static struct windows_device_state g_devices[MAX_DEVICES];
static BOOL g_initialized = FALSE;
static CRITICAL_SECTION g_global_lock;

/* Initialize Windows simulation subsystem */
static int windows_sim_init(void)
{
    if (g_initialized)
        return 0;

    InitializeCriticalSection(&g_global_lock);

    for (int i = 0; i < MAX_DEVICES; i++) {
        g_devices[i].active = FALSE;
        g_devices[i].mutex = CreateMutex(NULL, FALSE, NULL);
        if (!g_devices[i].mutex) {
            /* Cleanup on failure */
            for (int j = 0; j < i; j++) {
                CloseHandle(g_devices[j].mutex);
            }
            DeleteCriticalSection(&g_global_lock);
            return -1;
        }

        /* Initialize statistics */
        memset(&g_devices[i].stats, 0, sizeof(g_devices[i].stats));
        g_devices[i].stats.min_latency_ns = UINT64_MAX;

        /* Get high-resolution timer frequency */
        if (!QueryPerformanceFrequency(&g_devices[i].frequency)) {
            g_devices[i].frequency.QuadPart = 1000000; /* Fallback to microsecond resolution */
        }

        snprintf(g_devices[i].device_name, sizeof(g_devices[i].device_name),
                "\\\\.\\PCIeSimulator%d", i);
    }

    g_initialized = TRUE;
    return 0;
}

/* Cleanup Windows simulation subsystem */
static void windows_sim_cleanup(void)
{
    if (!g_initialized)
        return;

    EnterCriticalSection(&g_global_lock);

    for (int i = 0; i < MAX_DEVICES; i++) {
        if (g_devices[i].mutex) {
            CloseHandle(g_devices[i].mutex);
            g_devices[i].mutex = NULL;
        }
        g_devices[i].active = FALSE;
    }

    g_initialized = FALSE;
    LeaveCriticalSection(&g_global_lock);
    DeleteCriticalSection(&g_global_lock);
}

/* Get high-resolution timestamp in nanoseconds */
static uint64_t get_timestamp_ns(struct windows_device_state *dev)
{
    LARGE_INTEGER counter;
    if (!QueryPerformanceCounter(&counter)) {
        /* Fallback to GetTickCount64 if high-res timer fails */
        return GetTickCount64() * 1000000ULL; /* Convert ms to ns */
    }

    /* Convert to nanoseconds */
    return (counter.QuadPart * 1000000000ULL) / dev->frequency.QuadPart;
}

/* Simulate transfer latency with realistic delay */
static void simulate_transfer_delay(uint32_t size)
{
    /* Simulate realistic PCIe transfer characteristics */
    /* Base latency: 1-10 microseconds */
    /* Throughput: ~1-8 GB/s depending on size */

    uint32_t base_delay_us = 1 + (rand() % 10);
    uint32_t throughput_delay_us = size / (1000 + (rand() % 7000)); /* 1-8 MB/s simulation */

    uint32_t total_delay_us = base_delay_us + throughput_delay_us;

    /* Use Windows Sleep for millisecond delays, busy wait for microseconds */
    if (total_delay_us >= 1000) {
        Sleep(total_delay_us / 1000);
        total_delay_us %= 1000;
    }

    if (total_delay_us > 0) {
        /* Busy wait for sub-millisecond precision */
        LARGE_INTEGER start, current, freq;
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&start);

        LONGLONG target_ticks = (total_delay_us * freq.QuadPart) / 1000000;

        do {
            QueryPerformanceCounter(&current);
        } while ((current.QuadPart - start.QuadPart) < target_ticks);
    }
}

/* Windows implementation of pcie_sim_open */
pcie_sim_error_t pcie_sim_open_impl(int device_id, pcie_sim_handle_t *handle)
{
    if (!handle || device_id < 0 || device_id >= MAX_DEVICES)
        return PCIE_SIM_ERROR_PARAM;

    /* Initialize subsystem if needed */
    if (windows_sim_init() != 0)
        return PCIE_SIM_ERROR_SYSTEM;

    EnterCriticalSection(&g_global_lock);

    /* Check if device is already in use */
    if (g_devices[device_id].active) {
        LeaveCriticalSection(&g_global_lock);
        return PCIE_SIM_ERROR_DEVICE;
    }

    /* Mark device as active */
    g_devices[device_id].active = TRUE;

    LeaveCriticalSection(&g_global_lock);

    /* Create handle structure */
    struct pcie_sim_handle *h = malloc(sizeof(struct pcie_sim_handle));
    if (!h) {
        g_devices[device_id].active = FALSE;
        return PCIE_SIM_ERROR_MEMORY;
    }

    h->fd = device_id; /* Use device_id as identifier */
    h->device_id = device_id;

    *handle = h;
    return PCIE_SIM_SUCCESS;
}

/* Windows implementation of pcie_sim_close */
pcie_sim_error_t pcie_sim_close_impl(pcie_sim_handle_t handle)
{
    if (!handle)
        return PCIE_SIM_ERROR_PARAM;

    struct pcie_sim_handle *h = (struct pcie_sim_handle *)handle;

    if (h->device_id < 0 || h->device_id >= MAX_DEVICES)
        return PCIE_SIM_ERROR_PARAM;

    EnterCriticalSection(&g_global_lock);
    g_devices[h->device_id].active = FALSE;
    LeaveCriticalSection(&g_global_lock);

    free(h);
    return PCIE_SIM_SUCCESS;
}

/* Windows implementation of pcie_sim_transfer */
pcie_sim_error_t pcie_sim_transfer_impl(pcie_sim_handle_t handle, void *buffer,
                                       size_t size, uint32_t direction,
                                       uint64_t *latency_ns)
{
    if (!handle || !buffer || size == 0 || size > (1024 * 1024))
        return PCIE_SIM_ERROR_PARAM;

    struct pcie_sim_handle *h = (struct pcie_sim_handle *)handle;

    if (h->device_id < 0 || h->device_id >= MAX_DEVICES)
        return PCIE_SIM_ERROR_PARAM;

    struct windows_device_state *dev = &g_devices[h->device_id];

    if (!dev->active)
        return PCIE_SIM_ERROR_DEVICE;

    /* Lock device for transfer */
    if (WaitForSingleObject(dev->mutex, 5000) != WAIT_OBJECT_0)
        return PCIE_SIM_ERROR_TIMEOUT;

    uint64_t start_time = get_timestamp_ns(dev);

    /* Simulate the transfer operation */
    if (direction == PCIE_SIM_TO_DEVICE) {
        /* TO_DEVICE: Simulate writing data */
        /* In real implementation, this would write to hardware */
        volatile uint8_t *src = (volatile uint8_t *)buffer;
        volatile uint8_t checksum = 0;
        for (size_t i = 0; i < size; i++) {
            checksum ^= src[i]; /* Simulate processing the data */
        }
        (void)checksum; /* Prevent optimization */
    } else {
        /* FROM_DEVICE: Simulate reading data */
        memset(buffer, 0xAA, size); /* Fill with test pattern */
    }

    /* Simulate realistic transfer delay */
    simulate_transfer_delay((uint32_t)size);

    uint64_t end_time = get_timestamp_ns(dev);
    uint64_t transfer_latency = end_time - start_time;

    /* Update statistics */
    dev->stats.total_transfers++;
    dev->stats.total_bytes += size;

    if (transfer_latency < dev->stats.min_latency_ns)
        dev->stats.min_latency_ns = transfer_latency;
    if (transfer_latency > dev->stats.max_latency_ns)
        dev->stats.max_latency_ns = transfer_latency;

    /* Update running average */
    if (dev->stats.total_transfers == 1) {
        dev->stats.avg_latency_ns = transfer_latency;
    } else {
        dev->stats.avg_latency_ns =
            (dev->stats.avg_latency_ns * (dev->stats.total_transfers - 1) + transfer_latency) /
            dev->stats.total_transfers;
    }

    if (latency_ns)
        *latency_ns = transfer_latency;

    ReleaseMutex(dev->mutex);
    return PCIE_SIM_SUCCESS;
}

/* Windows implementation of pcie_sim_get_stats */
pcie_sim_error_t pcie_sim_get_stats_impl(pcie_sim_handle_t handle,
                                         struct pcie_sim_stats *stats)
{
    if (!handle || !stats)
        return PCIE_SIM_ERROR_PARAM;

    struct pcie_sim_handle *h = (struct pcie_sim_handle *)handle;

    if (h->device_id < 0 || h->device_id >= MAX_DEVICES)
        return PCIE_SIM_ERROR_PARAM;

    struct windows_device_state *dev = &g_devices[h->device_id];

    if (!dev->active)
        return PCIE_SIM_ERROR_DEVICE;

    /* Lock device for stats access */
    if (WaitForSingleObject(dev->mutex, 1000) != WAIT_OBJECT_0)
        return PCIE_SIM_ERROR_TIMEOUT;

    *stats = dev->stats;

    ReleaseMutex(dev->mutex);
    return PCIE_SIM_SUCCESS;
}

/* Windows implementation of pcie_sim_reset_stats */
pcie_sim_error_t pcie_sim_reset_stats_impl(pcie_sim_handle_t handle)
{
    if (!handle)
        return PCIE_SIM_ERROR_PARAM;

    struct pcie_sim_handle *h = (struct pcie_sim_handle *)handle;

    if (h->device_id < 0 || h->device_id >= MAX_DEVICES)
        return PCIE_SIM_ERROR_PARAM;

    struct windows_device_state *dev = &g_devices[h->device_id];

    if (!dev->active)
        return PCIE_SIM_ERROR_DEVICE;

    /* Lock device for stats reset */
    if (WaitForSingleObject(dev->mutex, 1000) != WAIT_OBJECT_0)
        return PCIE_SIM_ERROR_TIMEOUT;

    memset(&dev->stats, 0, sizeof(dev->stats));
    dev->stats.min_latency_ns = UINT64_MAX;

    ReleaseMutex(dev->mutex);
    return PCIE_SIM_SUCCESS;
}

/* Windows cleanup function - call at program exit */
void pcie_sim_windows_cleanup(void)
{
    windows_sim_cleanup();
}

#endif /* _WIN32 */