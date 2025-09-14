/*
 * PCIe Simulator - Linux Simulation Backend
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
 * Linux-specific simulation backend using high-resolution timing and pthread
 * synchronization primitives.
 */

#ifndef _WIN32

#include "../lib/api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <time.h>
#include <pthread.h>

/* Maximum number of simulated devices */
#define MAX_DEVICES 8

/* Simulated device state for Linux */
struct linux_device_state {
    int active;
    pthread_mutex_t mutex;
    struct pcie_sim_stats stats;
    struct timespec start_time;
    char device_name[64];
};

/* Global simulation state */
static struct linux_device_state g_sim_devices[MAX_DEVICES];
static int g_sim_initialized = 0;
static pthread_mutex_t g_init_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Internal handle structure */
struct pcie_sim_handle {
    int fd;
    int device_id;
    int is_simulation;
};

/*
 * Initialize Linux simulation system
 */
static void linux_sim_init(void)
{
    pthread_mutex_lock(&g_init_mutex);

    if (!g_sim_initialized) {
        /* Initialize all device mutexes */
        for (int i = 0; i < MAX_DEVICES; i++) {
            pthread_mutex_init(&g_sim_devices[i].mutex, NULL);
            g_sim_devices[i].active = 0;
        }
        g_sim_initialized = 1;
    }

    pthread_mutex_unlock(&g_init_mutex);
}

/*
 * Get high-resolution timestamp in nanoseconds
 */
static uint64_t linux_sim_get_time_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

/*
 * Simulate transfer delay with nanosecond precision
 */
static void linux_sim_delay(uint64_t delay_ns)
{
    struct timespec ts;

    /* Only delay if significant enough (> 1000ns) */
    if (delay_ns < 1000)
        return;

    ts.tv_sec = delay_ns / 1000000000ULL;
    ts.tv_nsec = delay_ns % 1000000000ULL;

    nanosleep(&ts, NULL);
}

/*
 * Linux implementation of pcie_sim_open (pure simulation)
 */
pcie_sim_error_t pcie_sim_open_linux(int device_id, pcie_sim_handle_t *handle)
{
    struct pcie_sim_handle *h;

    if (!handle || device_id < 0 || device_id >= MAX_DEVICES)
        return PCIE_SIM_ERROR_PARAM;

    /* Initialize simulation if needed */
    linux_sim_init();

    /* Allocate handle structure */
    h = malloc(sizeof(*h));
    if (!h)
        return PCIE_SIM_ERROR_MEMORY;

    /* Set simulation mode */
    h->fd = -1;  /* No real device file descriptor */
    h->device_id = device_id;
    h->is_simulation = 1;

    /* Initialize device state */
    pthread_mutex_lock(&g_sim_devices[device_id].mutex);
    if (!g_sim_devices[device_id].active) {
        g_sim_devices[device_id].active = 1;
        memset(&g_sim_devices[device_id].stats, 0, sizeof(g_sim_devices[device_id].stats));
        clock_gettime(CLOCK_MONOTONIC, &g_sim_devices[device_id].start_time);
        snprintf(g_sim_devices[device_id].device_name, sizeof(g_sim_devices[device_id].device_name),
                 "pcie_sim%d", device_id);
    }
    pthread_mutex_unlock(&g_sim_devices[device_id].mutex);

    *handle = h;
    return PCIE_SIM_SUCCESS;
}

/*
 * Linux implementation of pcie_sim_close
 */
pcie_sim_error_t pcie_sim_close_linux(pcie_sim_handle_t handle)
{
    if (!handle)
        return PCIE_SIM_ERROR_PARAM;

    /* No real file descriptor to close in simulation mode */
    free(handle);
    return PCIE_SIM_SUCCESS;
}

/*
 * Linux implementation of pcie_sim_transfer (simulation)
 */
pcie_sim_error_t pcie_sim_transfer_linux(pcie_sim_handle_t handle,
                                        void *buffer,
                                        size_t size,
                                        uint32_t direction,
                                        uint64_t *latency_ns)
{
    uint64_t start_time, end_time, transfer_latency;
    uint64_t base_latency_per_mb = 10000; /* 10μs per MB base latency */
    uint64_t size_mb;

    if (!handle || !buffer || size == 0 || handle->device_id >= MAX_DEVICES)
        return PCIE_SIM_ERROR_PARAM;

    start_time = linux_sim_get_time_ns();

    /* Simulate transfer with realistic timing */
    size_mb = (size + 1024*1024 - 1) / (1024*1024); /* Round up to MB */
    if (size_mb == 0) size_mb = 1;

    transfer_latency = base_latency_per_mb * size_mb;

    /* Add some variance based on direction */
    if (direction == PCIE_SIM_FROM_DEVICE) {
        transfer_latency = (transfer_latency * 12) / 10; /* 20% slower for reads */
    }

    /* Simulate the transfer delay */
    linux_sim_delay(transfer_latency);

    end_time = linux_sim_get_time_ns();

    /* Update device statistics */
    pthread_mutex_lock(&g_sim_devices[handle->device_id].mutex);
    g_sim_devices[handle->device_id].stats.total_transfers++;
    g_sim_devices[handle->device_id].stats.total_bytes += size;

    /* Update latency statistics */
    uint64_t current_latency = end_time - start_time;
    if (g_sim_devices[handle->device_id].stats.total_transfers == 1) {
        g_sim_devices[handle->device_id].stats.avg_latency_ns = current_latency;
        g_sim_devices[handle->device_id].stats.min_latency_ns = current_latency;
        g_sim_devices[handle->device_id].stats.max_latency_ns = current_latency;
    } else {
        g_sim_devices[handle->device_id].stats.avg_latency_ns =
            (g_sim_devices[handle->device_id].stats.avg_latency_ns + current_latency) / 2;
        if (current_latency < g_sim_devices[handle->device_id].stats.min_latency_ns)
            g_sim_devices[handle->device_id].stats.min_latency_ns = current_latency;
        if (current_latency > g_sim_devices[handle->device_id].stats.max_latency_ns)
            g_sim_devices[handle->device_id].stats.max_latency_ns = current_latency;
    }
    pthread_mutex_unlock(&g_sim_devices[handle->device_id].mutex);

    if (latency_ns)
        *latency_ns = end_time - start_time;

    return PCIE_SIM_SUCCESS;
}

/*
 * Linux implementation of pcie_sim_get_stats (simulation)
 */
pcie_sim_error_t pcie_sim_get_stats_linux(pcie_sim_handle_t handle,
                                          struct pcie_sim_stats *stats)
{
    if (!handle || !stats || handle->device_id >= MAX_DEVICES)
        return PCIE_SIM_ERROR_PARAM;

    /* Copy current stats from simulation */
    pthread_mutex_lock(&g_sim_devices[handle->device_id].mutex);
    *stats = g_sim_devices[handle->device_id].stats;
    pthread_mutex_unlock(&g_sim_devices[handle->device_id].mutex);

    return PCIE_SIM_SUCCESS;
}

/*
 * Linux implementation of pcie_sim_reset_stats (simulation)
 */
pcie_sim_error_t pcie_sim_reset_stats_linux(pcie_sim_handle_t handle)
{
    if (!handle || handle->device_id >= MAX_DEVICES)
        return PCIE_SIM_ERROR_PARAM;

    /* Reset simulation stats */
    pthread_mutex_lock(&g_sim_devices[handle->device_id].mutex);
    memset(&g_sim_devices[handle->device_id].stats, 0, sizeof(g_sim_devices[handle->device_id].stats));
    pthread_mutex_unlock(&g_sim_devices[handle->device_id].mutex);

    return PCIE_SIM_SUCCESS;
}

#endif /* !_WIN32 */