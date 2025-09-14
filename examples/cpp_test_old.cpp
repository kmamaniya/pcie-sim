/*
 * PCIe Simulator - C++ Test Application
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
 */

#include "../lib/device.hpp"
#include "../lib/monitor.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <iomanip>
#include <random>
#include <memory>
#include <cstring>
#include <map>
#include <sstream>
#include <functional>

using namespace PCIeSimulator;

// Simple but extensible command line options parser using STL
class ProgramOptions {
public:
    struct Option {
        std::string description;
        std::string default_value;
        bool required;
        std::function<bool(const std::string&)> validator;

        Option() : required(false) {}

        Option(const std::string& desc, const std::string& def_val = "", bool req = false,
               std::function<bool(const std::string&)> val = nullptr)
            : description(desc), default_value(def_val), required(req), validator(val) {}
    };

private:
    std::map<std::string, Option> options_;
    std::map<std::string, std::string> aliases_;
    std::map<std::string, std::string> values_;
    std::string program_name_;

public:
    void add_option(const std::string& name, const Option& option) {
        options_[name] = option;
        if (!option.default_value.empty()) {
            values_[name] = option.default_value;
        }
    }

    void add_alias(const std::string& alias, const std::string& option) {
        aliases_[alias] = option;
    }

    bool parse(int argc, char* argv[]) {
        program_name_ = argv[0];

        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];

            if (arg == "--help" || arg == "-h") {
                print_help();
                return false;
            }

            if (arg.substr(0, 2) == "--") {
                std::string option_name = arg.substr(2);

                // Check if it's a known option
                if (options_.find(option_name) == options_.end()) {
                    std::cerr << "Error: Unknown option '" << arg << "'" << std::endl;
                    print_help();
                    return false;
                }

                // Get value
                if (i + 1 >= argc) {
                    std::cerr << "Error: Option '" << arg << "' requires a value" << std::endl;
                    return false;
                }

                std::string value = argv[++i];

                // Validate if validator exists
                if (options_[option_name].validator && !options_[option_name].validator(value)) {
                    std::cerr << "Error: Invalid value '" << value << "' for option '" << arg << "'" << std::endl;
                    return false;
                }

                values_[option_name] = value;
            }
            else if (arg.substr(0, 1) == "-") {
                std::string alias = arg.substr(1);
                if (aliases_.find(alias) == aliases_.end()) {
                    std::cerr << "Error: Unknown option '" << arg << "'" << std::endl;
                    print_help();
                    return false;
                }

                std::string option_name = aliases_[alias];
                if (i + 1 >= argc) {
                    std::cerr << "Error: Option '" << arg << "' requires a value" << std::endl;
                    return false;
                }

                std::string value = argv[++i];

                if (options_[option_name].validator && !options_[option_name].validator(value)) {
                    std::cerr << "Error: Invalid value '" << value << "' for option '" << arg << "'" << std::endl;
                    return false;
                }

                values_[option_name] = value;
            }
            else {
                std::cerr << "Error: Unexpected argument '" << arg << "'" << std::endl;
                print_help();
                return false;
            }
        }

        // Check required options
        for (const auto& opt : options_) {
            if (opt.second.required && values_.find(opt.first) == values_.end()) {
                std::cerr << "Error: Required option '--" << opt.first << "' is missing" << std::endl;
                return false;
            }
        }

        return true;
    }

    template<typename T>
    T get(const std::string& option_name) const {
        auto it = values_.find(option_name);
        if (it == values_.end()) {
            throw std::runtime_error("Option '" + option_name + "' not found");
        }

        std::stringstream ss(it->second);
        T value;
        ss >> value;
        if (ss.fail()) {
            throw std::runtime_error("Failed to convert option '" + option_name + "' to requested type");
        }
        return value;
    }

    std::string get_string(const std::string& option_name) const {
        auto it = values_.find(option_name);
        if (it == values_.end()) {
            throw std::runtime_error("Option '" + option_name + "' not found");
        }
        return it->second;
    }

    bool has_option(const std::string& option_name) const {
        return values_.find(option_name) != values_.end();
    }

    void print_help() const {
        std::cout << "Usage: " << program_name_ << " [OPTIONS]\n";
        std::cout << "\nOptions:\n";

        for (const auto& opt : options_) {
            std::cout << "  --" << std::left << std::setw(15) << opt.first;

            // Show aliases
            for (const auto& alias : aliases_) {
                if (alias.second == opt.first) {
                    std::cout << " -" << alias.first;
                    break;
                }
            }

            std::cout << std::setw(5) << " ";
            std::cout << opt.second.description;

            if (!opt.second.default_value.empty()) {
                std::cout << " (default: " << opt.second.default_value << ")";
            }

            if (opt.second.required) {
                std::cout << " [REQUIRED]";
            }

            std::cout << std::endl;
        }

        std::cout << "\nExamples:\n";
        std::cout << "  " << program_name_ << "                        # Use default settings\n";
        std::cout << "  " << program_name_ << " --num-devices 1       # Test single device\n";
        std::cout << "  " << program_name_ << " --num-devices 8       # Test all 8 devices\n";
        std::cout << "  " << program_name_ << " -d 4                  # Test 4 devices (short form)\n";
    }
};

void print_header(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "=== " << title << " ===" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

void basic_transfer_test(int num_devices) {
    print_header("Basic Transfer Test");
    std::cout << "Testing " << num_devices << " device" << (num_devices > 1 ? "s" : "") << std::endl;

    try {
        for (int dev = 0; dev < num_devices; ++dev) {
            std::cout << "\n--- Device " << dev << " ---" << std::endl;
            auto device = DeviceManager::open_device(dev);

            std::vector<uint32_t> write_data(1024);
            for (size_t i = 0; i < write_data.size(); ++i) {
                write_data[i] = static_cast<uint32_t>(i + dev * 1000); // Unique pattern per device
            }

            std::cout << "Writing " << write_data.size() * sizeof(uint32_t) << " bytes..." << std::endl;
            uint64_t write_latency = device->write(write_data);
            std::cout << "Write latency: " << std::fixed << std::setprecision(3)
                      << (write_latency / 1000.0) << " μs" << std::endl;

            std::vector<uint32_t> read_data(1024);
            std::cout << "Reading " << read_data.size() * sizeof(uint32_t) << " bytes..." << std::endl;
            uint64_t read_latency = device->read(read_data);
            std::cout << "Read latency: " << std::fixed << std::setprecision(3)
                      << (read_latency / 1000.0) << " μs" << std::endl;

            auto stats = device->get_statistics();
            std::cout << "\nDevice Statistics:" << std::endl;
            std::cout << "Total transfers: " << stats.total_transfers() << std::endl;
            std::cout << "Total bytes: " << stats.total_bytes() << std::endl;
            std::cout << "Average latency: " << std::fixed << std::setprecision(3)
                      << (stats.avg_latency_ns() / 1000.0) << " μs" << std::endl;
        }
    } catch (const DeviceError& e) {
        std::cerr << "Transfer test failed: " << e.what() << std::endl;
    }
}

void benchmark_test(int num_devices) {
    print_header("Benchmark Test");
    std::cout << "Running benchmark on " << num_devices << " device"
              << (num_devices > 1 ? "s" : "") << std::endl;

    const int transfers_per_device = 100;
    const size_t transfer_size = 4096;

    try {
        std::vector<std::unique_ptr<Device>> devices;
        for (int i = 0; i < num_devices; ++i) {
            devices.push_back(DeviceManager::open_device(i));
        }

        std::vector<uint8_t> data(transfer_size, 0xAA);
        auto start = std::chrono::high_resolution_clock::now();

        for (int transfer = 0; transfer < transfers_per_device; ++transfer) {
            for (int dev = 0; dev < num_devices; ++dev) {
                devices[dev]->write(data);
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        uint64_t total_transfers = num_devices * transfers_per_device;
        uint64_t total_bytes = total_transfers * transfer_size;
        double throughput_mbps = (total_bytes * 8.0) / duration.count(); // Mbps

        std::cout << "\nBenchmark Results:" << std::endl;
        std::cout << "Devices: " << num_devices << std::endl;
        std::cout << "Transfers: " << total_transfers << std::endl;
        std::cout << "Bytes: " << total_bytes << " (" << std::fixed << std::setprecision(2)
                  << (total_bytes / 1024.0 / 1024.0) << " MB)" << std::endl;
        std::cout << "Duration: " << duration.count() << " μs" << std::endl;
        std::cout << "Throughput: " << std::fixed << std::setprecision(2)
                  << throughput_mbps << " Mbps" << std::endl;

        // Individual device statistics
        if (num_devices > 1) {
            std::cout << "\nPer-Device Statistics:" << std::endl;
            std::cout << std::setw(8) << "Device" << std::setw(12) << "Transfers"
                      << std::setw(15) << "Avg Latency" << std::setw(15) << "Min Latency"
                      << std::setw(15) << "Max Latency" << std::endl;
            std::cout << std::string(65, '-') << std::endl;

            for (int dev = 0; dev < num_devices; ++dev) {
                auto stats = devices[dev]->get_statistics();
                std::cout << std::setw(8) << dev
                          << std::setw(12) << stats.total_transfers()
                          << std::setw(13) << std::fixed << std::setprecision(2)
                          << (stats.avg_latency_ns() / 1000.0) << " μs"
                          << std::setw(13) << std::fixed << std::setprecision(2)
                          << (stats.min_latency_ns() / 1000.0) << " μs"
                          << std::setw(13) << std::fixed << std::setprecision(2)
                          << (stats.max_latency_ns() / 1000.0) << " μs" << std::endl;
            }
        }

    } catch (const DeviceError& e) {
        std::cerr << "Benchmark test failed: " << e.what() << std::endl;
    }
}

void parallel_worker(int device_id, int num_transfers, std::vector<double>& throughputs) {
    try {
        auto device = DeviceManager::open_device(device_id);
        std::vector<uint8_t> data(4096, 0xBB + device_id); // Unique pattern per device

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < num_transfers; ++i) {
            device->write(data);
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        uint64_t total_bytes = num_transfers * 4096;
        double throughput_mbps = (total_bytes * 8.0) / duration.count(); // Mbps
        throughputs[device_id] = throughput_mbps;

    } catch (const DeviceError& e) {
        std::cerr << "Worker " << device_id << " failed: " << e.what() << std::endl;
        throughputs[device_id] = 0.0;
    }
}

void parallel_test(int num_devices) {
    if (num_devices == 1) {
        return; // Skip parallel test for single device
    }

    print_header("Parallel Operations Test");
    std::cout << "Testing " << num_devices << " devices operating in parallel..." << std::endl;

    const int transfers_per_device = 50;
    std::vector<double> throughputs(num_devices, 0.0);
    std::vector<std::thread> workers;

    auto start = std::chrono::high_resolution_clock::now();

    // Launch parallel workers
    for (int i = 0; i < num_devices; ++i) {
        workers.emplace_back(parallel_worker, i, transfers_per_device, std::ref(throughputs));
    }

    // Wait for all workers to complete
    for (auto& worker : workers) {
        worker.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "\nParallel Operations Results:" << std::endl;
    std::cout << "Total time: " << total_duration.count() << " ms" << std::endl;
    std::cout << std::setw(8) << "Device" << std::setw(15) << "Throughput" << std::endl;
    std::cout << std::string(25, '-') << std::endl;

    double total_throughput = 0.0;
    for (int i = 0; i < num_devices; ++i) {
        std::cout << std::setw(8) << i
                  << std::setw(13) << std::fixed << std::setprecision(2)
                  << throughputs[i] << " Mbps" << std::endl;
        total_throughput += throughputs[i];
    }

    std::cout << std::string(25, '-') << std::endl;
    std::cout << std::setw(8) << "Total"
              << std::setw(13) << std::fixed << std::setprecision(2)
              << total_throughput << " Mbps" << std::endl;
}

void device_independence_test(int num_devices) {
    if (num_devices == 1) {
        return; // Skip independence test for single device
    }

    print_header("Device Independence Test");
    std::cout << "Testing that " << num_devices << " devices maintain separate state..." << std::endl;

    try {
        std::vector<std::unique_ptr<Device>> devices;
        for (int i = 0; i < num_devices; ++i) {
            devices.push_back(DeviceManager::open_device(i));
            std::cout << "✓ Device " << i << " opened successfully" << std::endl;
        }

        // Test different transfer sizes per device to verify independence
        std::vector<size_t> transfer_sizes = {1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072};

        for (int i = 0; i < num_devices && i < static_cast<int>(transfer_sizes.size()); ++i) {
            std::vector<uint8_t> data(transfer_sizes[i], i + 1);

            // Perform unique number of transfers per device
            for (int j = 0; j < (i + 1); ++j) {
                devices[i]->write(data);
            }
        }

        // Verify each device has independent statistics
        std::cout << "\nDevice Statistics (showing independence):" << std::endl;
        std::cout << std::setw(8) << "Device" << std::setw(12) << "Transfers"
                  << std::setw(15) << "Bytes" << std::setw(15) << "Avg Latency" << std::endl;
        std::cout << std::string(50, '-') << std::endl;

        for (int i = 0; i < num_devices; ++i) {
            auto stats = devices[i]->get_statistics();
            std::cout << std::setw(8) << i
                      << std::setw(12) << stats.total_transfers()
                      << std::setw(15) << stats.total_bytes()
                      << std::setw(13) << std::fixed << std::setprecision(2)
                      << (stats.avg_latency_ns() / 1000.0) << " μs" << std::endl;
        }

    } catch (const DeviceError& e) {
        std::cerr << "Independence test failed: " << e.what() << std::endl;
    }
}

void performance_monitoring_test(int num_devices) {
    print_header("Real-time Performance Monitoring Test");
    std::cout << "Starting real-time monitoring for " << num_devices << " device"
              << (num_devices > 1 ? "s" : "") << " (5 seconds)..." << std::endl;

    try {
        std::vector<std::unique_ptr<Device>> devices;
        for (int i = 0; i < num_devices; ++i) {
            devices.push_back(DeviceManager::open_device(i));
        }

        std::vector<uint8_t> data(4096, 0xCC);
        auto start_time = std::chrono::steady_clock::now();
        auto next_report = start_time + std::chrono::seconds(1);

        int report_count = 0;
        while (report_count < 5) {
            // Perform transfers on all devices
            for (int dev = 0; dev < num_devices; ++dev) {
                devices[dev]->write(data);
            }

            auto now = std::chrono::steady_clock::now();
            if (now >= next_report) {
                report_count++;
                next_report += std::chrono::seconds(1);

                if (num_devices == 1) {
                    auto stats = devices[0]->get_statistics();
                    std::cout << "\n=== Performance Metrics (Device 0) ===" << std::endl;
                    std::cout << "Transfers: " << stats.total_transfers() << std::endl;
                    std::cout << "Bytes: " << stats.total_bytes() << " ("
                              << std::fixed << std::setprecision(2)
                              << (stats.total_bytes() / 1024.0 / 1024.0) << " MB)" << std::endl;
                    std::cout << "Latency - Avg: " << std::fixed << std::setprecision(2)
                              << (stats.avg_latency_ns() / 1000.0) << " μs, "
                              << "Min: " << (stats.min_latency_ns() / 1000.0) << " μs, "
                              << "Max: " << (stats.max_latency_ns() / 1000.0) << " μs" << std::endl;
                } else {
                    std::cout << "\n=== Performance Metrics (All " << num_devices << " Devices) ===" << std::endl;
                    uint64_t total_transfers = 0, total_bytes = 0;
                    for (int dev = 0; dev < num_devices; ++dev) {
                        auto stats = devices[dev]->get_statistics();
                        total_transfers += stats.total_transfers();
                        total_bytes += stats.total_bytes();
                    }
                    std::cout << "Total Transfers: " << total_transfers << std::endl;
                    std::cout << "Total Bytes: " << total_bytes << " ("
                              << std::fixed << std::setprecision(2)
                              << (total_bytes / 1024.0 / 1024.0) << " MB)" << std::endl;
                }
            }
        }

        std::cout << "\nMonitoring stopped." << std::endl;

    } catch (const DeviceError& e) {
        std::cerr << "Monitoring test failed: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::cout << "PCIe Simulator - C++ Interface Test" << std::endl;
    std::cout << "Copyright (c) 2025 Karan Mamaniya" << std::endl;
    std::cout << "====================================" << std::endl;

    // Setup program options
    ProgramOptions options;

    // Add num-devices option with validation
    options.add_option("num-devices",
        ProgramOptions::Option("Number of devices to test (1-8)", "1", false,
            [](const std::string& value) {
                int num = std::stoi(value);
                return num >= 1 && num <= 8;
            }));

    // Add transfer pattern option
    options.add_option("pattern",
        ProgramOptions::Option("Transfer pattern: small-fast, large-burst, mixed, custom", "mixed", false,
            [](const std::string& value) {
                return value == "small-fast" || value == "large-burst" ||
                       value == "mixed" || value == "custom";
            }));

    // Add custom transfer size (used with --pattern custom)
    options.add_option("size",
        ProgramOptions::Option("Custom transfer size in bytes (64-4194304)", "4096", false,
            [](const std::string& value) {
                int size = std::stoi(value);
                return size >= 64 && size <= 4194304;
            }));

    // Add custom transfer rate (used with --pattern custom)
    options.add_option("rate",
        ProgramOptions::Option("Custom transfer rate in Hz (1-10000)", "1000", false,
            [](const std::string& value) {
                int rate = std::stoi(value);
                return rate >= 1 && rate <= 10000;
            }));

    // Add CSV logging option
    options.add_option("log-csv",
        ProgramOptions::Option("Log results to CSV file", "", false));

    // Add error injection scenario
    options.add_option("error-scenario",
        ProgramOptions::Option("Error injection: timeout, corruption, overrun, none", "none", false,
            [](const std::string& value) {
                return value == "timeout" || value == "corruption" ||
                       value == "overrun" || value == "none";
            }));

    // Add stress testing options
    options.add_option("threads",
        ProgramOptions::Option("Number of concurrent threads for stress testing", "1", false,
            [](const std::string& value) {
                int threads = std::stoi(value);
                return threads >= 1 && threads <= 64;
            }));

    options.add_option("duration",
        ProgramOptions::Option("Test duration in seconds", "10", false,
            [](const std::string& value) {
                int duration = std::stoi(value);
                return duration >= 1 && duration <= 3600;
            }));

    // Add convenience aliases
    options.add_alias("d", "num-devices");
    options.add_alias("p", "pattern");
    options.add_alias("s", "size");
    options.add_alias("r", "rate");
    options.add_alias("l", "log-csv");
    options.add_alias("e", "error-scenario");
    options.add_alias("t", "threads");
    options.add_alias("dur", "duration");

    // Parse command line arguments
    if (!options.parse(argc, argv)) {
        return options.has_option("help") ? 0 : 1;
    }

    // Get parsed values
    int num_devices = options.get<int>("num-devices");
    std::string pattern = options.get<std::string>("pattern");
    int custom_size = options.get<int>("size");
    int custom_rate = options.get<int>("rate");
    std::string csv_file = options.get<std::string>("log-csv");
    std::string error_scenario = options.get<std::string>("error-scenario");
    int num_threads = options.get<int>("threads");
    int duration = options.get<int>("duration");

    std::cout << "\nTesting with " << num_devices << " device"
              << (num_devices > 1 ? "s" : "") << std::endl;
    std::cout << "Transfer pattern: " << pattern << std::endl;
    if (pattern == "custom") {
        std::cout << "Custom size: " << custom_size << " bytes, Rate: " << custom_rate << " Hz" << std::endl;
    }
    if (error_scenario != "none") {
        std::cout << "Error injection: " << error_scenario << std::endl;
    }
    if (num_threads > 1) {
        std::cout << "Stress testing: " << num_threads << " threads for " << duration << "s" << std::endl;
    }
    if (!csv_file.empty()) {
        std::cout << "Logging to CSV: " << csv_file << std::endl;
    }
    std::cout << std::endl;

    try {
        // Initialize CSV logger if requested
        std::unique_ptr<CSVLogger> logger;
        if (!csv_file.empty()) {
            logger = std::make_unique<CSVLogger>(csv_file);
        }

        // Configure transfer pattern
        TransferConfig config = configure_transfer_pattern(pattern, custom_size, custom_rate);

        // Run tests based on configuration
        if (num_threads > 1) {
            stress_test(num_devices, num_threads, duration, config, error_scenario, logger.get());
        } else {
            // Run standard tests with new configuration
            pattern_based_test(num_devices, config, error_scenario, logger.get());
            benchmark_test(num_devices);
            performance_monitoring_test(num_devices);
        }

        // Multi-device specific tests
        if (num_devices > 1) {
            device_independence_test(num_devices);
            parallel_test(num_devices);
        }

        print_header("All Tests Completed Successfully");
        if (num_devices == 1) {
            std::cout << "✓ Single device tests passed" << std::endl;
        } else {
            std::cout << "✓ All " << num_devices << " devices working independently" << std::endl;
            std::cout << "✓ Parallel operations confirmed" << std::endl;
            std::cout << "✓ Device independence verified" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}