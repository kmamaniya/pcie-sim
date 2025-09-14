/*
 * PCIe Simulator - Shared Configuration Utilities
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
 * Shared configuration structures and utilities for kernel, simulation,
 * and application components.
 */

#ifndef PCIE_SIM_CONFIG_H
#define PCIE_SIM_CONFIG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Transfer pattern types */
typedef enum {
    PCIE_SIM_PATTERN_SMALL_FAST = 0,    /* 64B-1KB @ high frequency */
    PCIE_SIM_PATTERN_LARGE_BURST = 1,   /* 1-4MB @ lower frequency */
    PCIE_SIM_PATTERN_MIXED = 2,         /* Mixed workload */
    PCIE_SIM_PATTERN_CUSTOM = 3         /* User-defined */
} pcie_sim_pattern_t;

/* Error injection scenarios */
typedef enum {
    PCIE_SIM_ERROR_SCENARIO_NONE = 0,
    PCIE_SIM_ERROR_SCENARIO_TIMEOUT = 1,         /* Simulate transfer timeouts */
    PCIE_SIM_ERROR_SCENARIO_CORRUPTION = 2,      /* Simulate data corruption */
    PCIE_SIM_ERROR_SCENARIO_OVERRUN = 3          /* Simulate buffer overruns */
} pcie_sim_error_scenario_t;

/* Load testing types */
typedef enum {
    PCIE_SIM_LOAD_NORMAL = 0,
    PCIE_SIM_LOAD_STRESS = 1,           /* High concurrent load */
    PCIE_SIM_LOAD_BURST = 2             /* Intermittent bursts */
} pcie_sim_load_type_t;

/* Transfer configuration */
struct pcie_sim_transfer_config {
    pcie_sim_pattern_t pattern;
    uint32_t min_size;              /* Minimum transfer size in bytes */
    uint32_t max_size;              /* Maximum transfer size in bytes */
    uint32_t rate_hz;               /* Target transfer rate in Hz */
    uint32_t burst_count;           /* Transfers per burst */
    uint32_t burst_interval_ms;     /* Interval between bursts in ms */
};

/* Error injection configuration */
struct pcie_sim_error_config {
    pcie_sim_error_scenario_t scenario;
    float probability;              /* Error probability (0.0-1.0) */
    uint32_t inject_after_count;    /* Inject error after N transfers */
    uint32_t recovery_time_ms;      /* Recovery time after error */
};

/* Stress testing configuration */
struct pcie_sim_stress_config {
    pcie_sim_load_type_t load_type;
    uint32_t num_threads;           /* Number of concurrent threads */
    uint32_t duration_seconds;      /* Test duration */
    uint32_t ramp_up_seconds;       /* Gradual load increase time */
};

/* Logging configuration */
struct pcie_sim_log_config {
    char csv_filename[256];         /* CSV log file path */
    uint32_t log_interval_ms;       /* Logging interval */
    uint32_t max_entries;           /* Maximum log entries */
    uint32_t buffer_size;           /* Log buffer size */
};

/* Complete test configuration */
struct pcie_sim_test_config {
    uint32_t num_devices;
    struct pcie_sim_transfer_config transfer;
    struct pcie_sim_error_config error;
    struct pcie_sim_stress_config stress;
    struct pcie_sim_log_config logging;
    uint32_t flags;                 /* Configuration flags */
};

/* Configuration flags */
#define PCIE_SIM_CONFIG_ENABLE_LOGGING    (1 << 0)
#define PCIE_SIM_CONFIG_ENABLE_ERRORS     (1 << 1)
#define PCIE_SIM_CONFIG_ENABLE_STRESS     (1 << 2)
#define PCIE_SIM_CONFIG_VERBOSE           (1 << 3)
#define PCIE_SIM_CONFIG_REAL_TIME         (1 << 4)

/* Predefined transfer patterns */
extern const struct pcie_sim_transfer_config PCIE_SIM_PATTERN_SMALL_FAST_CONFIG;
extern const struct pcie_sim_transfer_config PCIE_SIM_PATTERN_LARGE_BURST_CONFIG;
extern const struct pcie_sim_transfer_config PCIE_SIM_PATTERN_MIXED_CONFIG;

/* Configuration utility functions */
int pcie_sim_config_init(struct pcie_sim_test_config *config);
int pcie_sim_config_set_pattern(struct pcie_sim_test_config *config,
                                pcie_sim_pattern_t pattern);
int pcie_sim_config_set_custom_pattern(struct pcie_sim_test_config *config,
                                      uint32_t size, uint32_t rate);
int pcie_sim_config_set_error_scenario(struct pcie_sim_test_config *config,
                                      pcie_sim_error_scenario_t scenario);
int pcie_sim_config_validate(const struct pcie_sim_test_config *config);

/* Configuration string parsers */
pcie_sim_pattern_t pcie_sim_parse_pattern(const char *pattern_str);
pcie_sim_error_scenario_t pcie_sim_parse_error_scenario(const char *error_str);
const char *pcie_sim_pattern_to_string(pcie_sim_pattern_t pattern);
const char *pcie_sim_error_scenario_to_string(pcie_sim_error_scenario_t scenario);

#ifdef __cplusplus
}
#endif

#endif /* PCIE_SIM_CONFIG_H */