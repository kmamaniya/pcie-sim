/*
 * PCIe Simulator - Enhanced C++ Test Application
 *
 * Copyright (c) 2025 Karan Mamaniya <kmamaniya@gmail.com>
 * MIT License
 *
 * Enhanced test application with modular configuration, CSV logging,
 * error injection, and stress testing capabilities.
 */

#include "../lib/device.hpp"
#include "../lib/monitor.hpp"
#include "../utils/options.hpp"
#include "../utils/csv_logger.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <random>
#include <future>
#include <iomanip>
#include <functional>
#include <sstream>

using namespace PCIeSimulator;

// Global configuration
static std::unique_ptr<pcie_sim_test_config> g_config;
static std::unique_ptr<SessionLogger> g_session_logger;

void print_header(const std::string& title) {
    std::cout << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    std::cout << "=== " << title << " ===" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

void print_config_summary(const pcie_sim_test_config& config) {
    std::cout << "\n📊 Test Configuration Summary:" << std::endl;
    std::cout << "  Devices: " << config.num_devices << std::endl;
    std::cout << "  Pattern: " << pcie_sim_pattern_to_string(config.transfer.pattern) << std::endl;
    std::cout << "  Transfer size: " << config.transfer.min_size;
    if (config.transfer.min_size != config.transfer.max_size) {
        std::cout << "-" << config.transfer.max_size;
    }
    std::cout << " bytes" << std::endl;
    std::cout << "  Rate: " << config.transfer.rate_hz << " Hz" << std::endl;

    if (config.error.scenario != PCIE_SIM_ERROR_SCENARIO_NONE) {
        std::cout << "  Error injection: " << pcie_sim_error_scenario_to_string(config.error.scenario);
        std::cout << " (" << (config.error.probability * 100.0f) << "%)" << std::endl;
    }

    if (config.flags & PCIE_SIM_CONFIG_ENABLE_STRESS) {
        std::cout << "  Stress testing: " << config.stress.num_threads << " threads for ";
        std::cout << config.stress.duration_seconds << "s" << std::endl;
    }

    if (config.flags & PCIE_SIM_CONFIG_ENABLE_LOGGING) {
        std::cout << "  CSV Logging: " << config.logging.csv_filename << std::endl;
    }
    std::cout << std::endl;
}

class ErrorInjector {
private:
    std::mt19937 rng_;
    std::uniform_real_distribution<float> dist_;
    pcie_sim_error_scenario_t scenario_;
    float probability_;

public:
    ErrorInjector(pcie_sim_error_scenario_t scenario, float probability)
        : rng_(std::chrono::high_resolution_clock::now().time_since_epoch().count()),
          dist_(0.0f, 1.0f), scenario_(scenario), probability_(probability) {}

    bool should_inject_error() {
        return dist_(rng_) < probability_;
    }

    std::string get_error_type() const {
        return pcie_sim_error_scenario_to_string(scenario_);
    }

    void simulate_error_delay() {
        // Simulate error recovery time
        switch (scenario_) {
        case PCIE_SIM_ERROR_SCENARIO_TIMEOUT:
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            break;
        case PCIE_SIM_ERROR_SCENARIO_CORRUPTION:
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            break;
        case PCIE_SIM_ERROR_SCENARIO_OVERRUN:
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            break;
        default:
            break;
        }
    }
};

void pattern_based_transfer_test(int device_id, const pcie_sim_transfer_config& config,
                                ErrorInjector* error_injector) {
    try {
        auto device = DeviceManager::open_device(device_id);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint32_t> size_dist(config.min_size, config.max_size);

        // Calculate number of transfers based on pattern
        uint32_t num_transfers = 0;
        uint32_t inter_transfer_delay_us = 0;

        switch (config.pattern) {
        case PCIE_SIM_PATTERN_SMALL_FAST:
            num_transfers = 100;
            inter_transfer_delay_us = 1000000 / config.rate_hz; // Convert Hz to microseconds
            break;
        case PCIE_SIM_PATTERN_LARGE_BURST:
            num_transfers = config.burst_count;
            inter_transfer_delay_us = config.burst_interval_ms * 1000;
            break;
        case PCIE_SIM_PATTERN_MIXED:
        case PCIE_SIM_PATTERN_CUSTOM:
            num_transfers = 50;
            inter_transfer_delay_us = 1000000 / config.rate_hz;
            break;
        }

        std::cout << "Device " << device_id << " - Pattern: "
                  << pcie_sim_pattern_to_string(config.pattern) << std::endl;

        for (uint32_t i = 0; i < num_transfers; ++i) {
            uint32_t transfer_size = size_dist(gen);
            std::vector<uint8_t> data(transfer_size, device_id + i);

            bool inject_error = error_injector && error_injector->should_inject_error();
            std::string error_status = "SUCCESS";

            auto start = std::chrono::high_resolution_clock::now();
            uint64_t latency_ns = 0;

            try {
                if (inject_error) {
                    error_injector->simulate_error_delay();
                    error_status = error_injector->get_error_type();
                    latency_ns = device->write(data) + 50000; // Add error overhead
                } else {
                    latency_ns = device->write(data);
                }
            } catch (const std::exception& e) {
                error_status = "EXCEPTION";
                latency_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::high_resolution_clock::now() - start).count();
            }

            double latency_us = latency_ns / 1000.0;
            double throughput_mbps = (transfer_size * 8.0) / (latency_us * 1000.0);

            // Log to CSV if enabled
            if (g_session_logger) {
                g_session_logger->log_transfer(device_id, transfer_size, latency_us,
                                             throughput_mbps, "TO_DEVICE", error_status,
                                             std::hash<std::thread::id>{}(std::this_thread::get_id()));
            }

            if (g_config->flags & PCIE_SIM_CONFIG_VERBOSE || inject_error) {
                std::cout << "  Transfer " << (i+1) << "/" << num_transfers
                          << ": " << transfer_size << " bytes, "
                          << std::fixed << std::setprecision(2) << latency_us << " μs";
                if (inject_error) {
                    std::cout << " [ERROR: " << error_status << "]";
                }
                std::cout << std::endl;
            }

            if (inter_transfer_delay_us > 0) {
                std::this_thread::sleep_for(std::chrono::microseconds(inter_transfer_delay_us));
            }
        }

        auto stats = device->get_statistics();
        std::cout << "  Completed " << num_transfers << " transfers" << std::endl;
        std::cout << "  Average latency: " << (stats.avg_latency_ns() / 1000.0) << " μs" << std::endl;
        std::cout << "  Throughput: " << stats.throughput_mbps() << " Mbps" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error testing device " << device_id << ": " << e.what() << std::endl;
    }
}

void stress_test_worker(int device_id, int thread_id, int duration_seconds,
                       const pcie_sim_transfer_config& config, ErrorInjector* error_injector) {
    try {
        auto device = DeviceManager::open_device(device_id);
        auto end_time = std::chrono::steady_clock::now() + std::chrono::seconds(duration_seconds);

        std::random_device rd;
        std::mt19937 gen(rd() + thread_id);
        std::uniform_int_distribution<uint32_t> size_dist(config.min_size, config.max_size);

        uint32_t transfer_count = 0;
        uint64_t total_latency = 0;

        while (std::chrono::steady_clock::now() < end_time) {
            uint32_t transfer_size = size_dist(gen);
            std::vector<uint8_t> data(transfer_size, thread_id);

            bool inject_error = error_injector && error_injector->should_inject_error();
            std::string error_status = "SUCCESS";
            uint64_t latency_ns = 0;

            try {
                if (inject_error) {
                    error_injector->simulate_error_delay();
                    error_status = error_injector->get_error_type();
                }
                latency_ns = device->write(data);
            } catch (const std::exception&) {
                error_status = "EXCEPTION";
            }

            total_latency += latency_ns;
            transfer_count++;

            double latency_us = latency_ns / 1000.0;
            double throughput_mbps = (transfer_size * 8.0) / (latency_us * 1000.0);

            // Log to CSV if enabled
            if (g_session_logger) {
                g_session_logger->log_transfer(device_id, transfer_size, latency_us,
                                             throughput_mbps, "TO_DEVICE", error_status, thread_id);
            }

            // Small delay to prevent overwhelming the system
            if (config.rate_hz > 0) {
                auto delay_us = 1000000 / (config.rate_hz / config.burst_count);
                std::this_thread::sleep_for(std::chrono::microseconds(delay_us));
            }
        }

        double avg_latency_us = (total_latency / transfer_count) / 1000.0;
        std::cout << "Thread " << thread_id << " (Device " << device_id << "): "
                  << transfer_count << " transfers, avg latency: "
                  << std::fixed << std::setprecision(2) << avg_latency_us << " μs" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Stress test worker error (device " << device_id
                  << ", thread " << thread_id << "): " << e.what() << std::endl;
    }
}

void run_pattern_tests() {
    print_header("Pattern-Based Transfer Tests");

    std::unique_ptr<ErrorInjector> error_injector;
    if (g_config->error.scenario != PCIE_SIM_ERROR_SCENARIO_NONE) {
        error_injector = std::unique_ptr<ErrorInjector>(new ErrorInjector(g_config->error.scenario,
                                                                          g_config->error.probability));
        std::cout << "🚨 Error injection enabled: "
                  << pcie_sim_error_scenario_to_string(g_config->error.scenario)
                  << " (" << (g_config->error.probability * 100.0f) << "%)" << std::endl;
    }

    for (uint32_t device_id = 0; device_id < g_config->num_devices; ++device_id) {
        pattern_based_transfer_test(device_id, g_config->transfer, error_injector.get());
    }
}

void run_stress_tests() {
    if (!(g_config->flags & PCIE_SIM_CONFIG_ENABLE_STRESS)) {
        return;
    }

    print_header("Multi-threaded Stress Testing");

    std::unique_ptr<ErrorInjector> error_injector;
    if (g_config->error.scenario != PCIE_SIM_ERROR_SCENARIO_NONE) {
        error_injector = std::unique_ptr<ErrorInjector>(new ErrorInjector(g_config->error.scenario,
                                                                          g_config->error.probability));
    }

    std::vector<std::future<void>> futures;

    std::cout << "🔥 Starting " << g_config->stress.num_threads << " concurrent threads for "
              << g_config->stress.duration_seconds << " seconds..." << std::endl;

    auto start_time = std::chrono::steady_clock::now();

    for (uint32_t i = 0; i < g_config->stress.num_threads; ++i) {
        int device_id = i % g_config->num_devices;  // Round-robin device assignment

        futures.push_back(std::async(std::launch::async, stress_test_worker,
                                   device_id, i, g_config->stress.duration_seconds,
                                   g_config->transfer, error_injector.get()));
    }

    // Wait for all threads to complete
    for (auto& future : futures) {
        future.wait();
    }

    auto end_time = std::chrono::steady_clock::now();
    auto actual_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "✅ Stress test completed in " << actual_duration.count() << " ms" << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "PCIe Simulator - Enhanced C++ Test Application" << std::endl;
    std::cout << "Copyright (c) 2025 Karan Mamaniya" << std::endl;
    std::cout << std::string(60, '=') << std::endl;

    // Create and parse options
    auto options = ProgramOptions::create_otpu_options();
    if (!options->parse(argc, argv)) {
        return options->has_option("help") ? 0 : 1;
    }

    // Convert to configuration
    g_config = options->to_config();
    if (pcie_sim_config_validate(g_config.get()) != 0) {
        std::cerr << "❌ Invalid configuration" << std::endl;
        return 1;
    }

    // Setup CSV logging if requested
    if (g_config->flags & PCIE_SIM_CONFIG_ENABLE_LOGGING) {
        std::string filename = g_config->logging.csv_filename;
        if (filename.empty()) {
            filename = CSVLogger::create_timestamped_filename("otpu_test");
        }

        std::ostringstream config_summary;
        config_summary << "pattern=" << pcie_sim_pattern_to_string(g_config->transfer.pattern)
                      << ",devices=" << g_config->num_devices
                      << ",size=" << g_config->transfer.min_size << "-" << g_config->transfer.max_size
                      << ",rate=" << g_config->transfer.rate_hz;

        g_session_logger = std::unique_ptr<SessionLogger>(new SessionLogger(filename, config_summary.str()));
    }

    print_config_summary(*g_config);

    try {
        // Run pattern-based tests
        run_pattern_tests();

        // Run stress tests if enabled
        run_stress_tests();

        print_header("All Tests Completed Successfully");
        std::cout << "✅ Test session completed" << std::endl;

        if (g_session_logger) {
            std::cout << "📊 Results logged to: " << g_session_logger->get_logger()->get_filename() << std::endl;
            std::cout << "📈 Total records: " << g_session_logger->get_logger()->get_record_count() << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}