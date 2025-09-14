/*
 * PCIe Simulator - Configuration Implementation
 *
 * Copyright (c) 2025 Karan Mamaniya <kmamaniya@gmail.com>
 * MIT License
 */

#include "config.h"
#include <string.h>
#include <stdlib.h>

/* Predefined transfer pattern configurations */
const struct pcie_sim_transfer_config PCIE_SIM_PATTERN_SMALL_FAST_CONFIG = {
    .pattern = PCIE_SIM_PATTERN_SMALL_FAST,
    .min_size = 64,
    .max_size = 1024,
    .rate_hz = 10000,
    .burst_count = 1,
    .burst_interval_ms = 0
};

const struct pcie_sim_transfer_config PCIE_SIM_PATTERN_LARGE_BURST_CONFIG = {
    .pattern = PCIE_SIM_PATTERN_LARGE_BURST,
    .min_size = 1048576,        /* 1MB */
    .max_size = 4194304,        /* 4MB */
    .rate_hz = 100,
    .burst_count = 10,
    .burst_interval_ms = 100
};

const struct pcie_sim_transfer_config PCIE_SIM_PATTERN_MIXED_CONFIG = {
    .pattern = PCIE_SIM_PATTERN_MIXED,
    .min_size = 1024,
    .max_size = 65536,
    .rate_hz = 1000,
    .burst_count = 5,
    .burst_interval_ms = 50
};

/*
 * Initialize configuration with defaults
 */
int pcie_sim_config_init(struct pcie_sim_test_config *config)
{
    if (!config)
        return -1;

    memset(config, 0, sizeof(*config));

    /* Default values */
    config->num_devices = 1;
    config->transfer = PCIE_SIM_PATTERN_MIXED_CONFIG;
    config->error.scenario = PCIE_SIM_ERROR_SCENARIO_NONE;
    config->error.probability = 0.0f;
    config->stress.load_type = PCIE_SIM_LOAD_NORMAL;
    config->stress.num_threads = 1;
    config->stress.duration_seconds = 10;
    config->logging.log_interval_ms = 1000;
    config->logging.max_entries = 10000;
    config->logging.buffer_size = 4096;

    return 0;
}

/*
 * Set transfer pattern
 */
int pcie_sim_config_set_pattern(struct pcie_sim_test_config *config,
                               pcie_sim_pattern_t pattern)
{
    if (!config)
        return -1;

    switch (pattern) {
    case PCIE_SIM_PATTERN_SMALL_FAST:
        config->transfer = PCIE_SIM_PATTERN_SMALL_FAST_CONFIG;
        break;
    case PCIE_SIM_PATTERN_LARGE_BURST:
        config->transfer = PCIE_SIM_PATTERN_LARGE_BURST_CONFIG;
        break;
    case PCIE_SIM_PATTERN_MIXED:
        config->transfer = PCIE_SIM_PATTERN_MIXED_CONFIG;
        break;
    case PCIE_SIM_PATTERN_CUSTOM:
        /* Keep existing custom configuration */
        config->transfer.pattern = PCIE_SIM_PATTERN_CUSTOM;
        break;
    default:
        return -1;
    }

    return 0;
}

/*
 * Set custom transfer pattern
 */
int pcie_sim_config_set_custom_pattern(struct pcie_sim_test_config *config,
                                      uint32_t size, uint32_t rate)
{
    if (!config || size < 64 || size > 4194304 || rate < 1 || rate > 10000)
        return -1;

    config->transfer.pattern = PCIE_SIM_PATTERN_CUSTOM;
    config->transfer.min_size = size;
    config->transfer.max_size = size;
    config->transfer.rate_hz = rate;
    config->transfer.burst_count = 1;
    config->transfer.burst_interval_ms = 0;

    return 0;
}

/*
 * Set error injection scenario
 */
int pcie_sim_config_set_error_scenario(struct pcie_sim_test_config *config,
                                      pcie_sim_error_scenario_t scenario)
{
    if (!config)
        return -1;

    config->error.scenario = scenario;

    switch (scenario) {
    case PCIE_SIM_ERROR_SCENARIO_NONE:
        config->error.probability = 0.0f;
        config->flags &= ~PCIE_SIM_CONFIG_ENABLE_ERRORS;
        break;
    case PCIE_SIM_ERROR_SCENARIO_TIMEOUT:
        config->error.probability = 0.01f;      /* 1% timeout rate */
        config->error.recovery_time_ms = 100;
        config->flags |= PCIE_SIM_CONFIG_ENABLE_ERRORS;
        break;
    case PCIE_SIM_ERROR_SCENARIO_CORRUPTION:
        config->error.probability = 0.005f;     /* 0.5% corruption rate */
        config->error.recovery_time_ms = 50;
        config->flags |= PCIE_SIM_CONFIG_ENABLE_ERRORS;
        break;
    case PCIE_SIM_ERROR_SCENARIO_OVERRUN:
        config->error.probability = 0.02f;      /* 2% overrun rate */
        config->error.recovery_time_ms = 200;
        config->flags |= PCIE_SIM_CONFIG_ENABLE_ERRORS;
        break;
    default:
        return -1;
    }

    return 0;
}

/*
 * Validate configuration
 */
int pcie_sim_config_validate(const struct pcie_sim_test_config *config)
{
    if (!config)
        return -1;

    /* Validate device count */
    if (config->num_devices < 1 || config->num_devices > 8)
        return -1;

    /* Validate transfer sizes */
    if (config->transfer.min_size < 64 || config->transfer.max_size > 4194304)
        return -1;

    if (config->transfer.min_size > config->transfer.max_size)
        return -1;

    /* Validate transfer rate */
    if (config->transfer.rate_hz < 1 || config->transfer.rate_hz > 10000)
        return -1;

    /* Validate error probability */
    if (config->error.probability < 0.0f || config->error.probability > 1.0f)
        return -1;

    /* Validate stress testing parameters */
    if (config->stress.num_threads > 64)
        return -1;

    if (config->stress.duration_seconds > 3600)  /* Max 1 hour */
        return -1;

    return 0;
}

/*
 * Parse pattern string
 */
pcie_sim_pattern_t pcie_sim_parse_pattern(const char *pattern_str)
{
    if (!pattern_str)
        return PCIE_SIM_PATTERN_MIXED;

    if (strcmp(pattern_str, "small-fast") == 0)
        return PCIE_SIM_PATTERN_SMALL_FAST;
    else if (strcmp(pattern_str, "large-burst") == 0)
        return PCIE_SIM_PATTERN_LARGE_BURST;
    else if (strcmp(pattern_str, "mixed") == 0)
        return PCIE_SIM_PATTERN_MIXED;
    else if (strcmp(pattern_str, "custom") == 0)
        return PCIE_SIM_PATTERN_CUSTOM;

    return PCIE_SIM_PATTERN_MIXED;  /* Default */
}

/*
 * Parse error scenario string
 */
pcie_sim_error_scenario_t pcie_sim_parse_error_scenario(const char *error_str)
{
    if (!error_str)
        return PCIE_SIM_ERROR_SCENARIO_NONE;

    if (strcmp(error_str, "timeout") == 0)
        return PCIE_SIM_ERROR_SCENARIO_TIMEOUT;
    else if (strcmp(error_str, "corruption") == 0)
        return PCIE_SIM_ERROR_SCENARIO_CORRUPTION;
    else if (strcmp(error_str, "overrun") == 0)
        return PCIE_SIM_ERROR_SCENARIO_OVERRUN;
    else if (strcmp(error_str, "none") == 0)
        return PCIE_SIM_ERROR_SCENARIO_NONE;

    return PCIE_SIM_ERROR_SCENARIO_NONE;  /* Default */
}

/*
 * Convert pattern to string
 */
const char *pcie_sim_pattern_to_string(pcie_sim_pattern_t pattern)
{
    switch (pattern) {
    case PCIE_SIM_PATTERN_SMALL_FAST:
        return "small-fast";
    case PCIE_SIM_PATTERN_LARGE_BURST:
        return "large-burst";
    case PCIE_SIM_PATTERN_MIXED:
        return "mixed";
    case PCIE_SIM_PATTERN_CUSTOM:
        return "custom";
    default:
        return "unknown";
    }
}

/*
 * Convert error scenario to string
 */
const char *pcie_sim_error_scenario_to_string(pcie_sim_error_scenario_t scenario)
{
    switch (scenario) {
    case PCIE_SIM_ERROR_SCENARIO_NONE:
        return "none";
    case PCIE_SIM_ERROR_SCENARIO_TIMEOUT:
        return "timeout";
    case PCIE_SIM_ERROR_SCENARIO_CORRUPTION:
        return "corruption";
    case PCIE_SIM_ERROR_SCENARIO_OVERRUN:
        return "overrun";
    default:
        return "unknown";
    }
}