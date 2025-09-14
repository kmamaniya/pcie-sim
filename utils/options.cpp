/*
 * PCIe Simulator - C++ Options Implementation
 *
 * Copyright (c) 2025 Karan Mamaniya <kmamaniya@gmail.com>
 * MIT License
 */

#include "options.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>

namespace PCIeSimulator {

bool ProgramOptions::parse(int argc, char* argv[]) {
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
                std::cerr << "Unknown option: " << arg << std::endl;
                return false;
            }

            // Check if option expects a value
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                std::string value = argv[++i];

                // Validate value if validator exists
                if (options_[option_name].validator) {
                    if (!options_[option_name].validator(value)) {
                        std::cerr << "Invalid value for option " << arg << ": " << value << std::endl;
                        return false;
                    }
                }

                values_[option_name] = value;
            } else {
                // Boolean option or option without value
                values_[option_name] = "true";
            }
        } else if (arg.substr(0, 1) == "-" && arg.length() > 1) {
            std::string alias = arg.substr(1);

            // Check if it's a known alias
            auto alias_it = aliases_.find(alias);
            if (alias_it == aliases_.end()) {
                std::cerr << "Unknown option: " << arg << std::endl;
                return false;
            }

            std::string option_name = alias_it->second;

            // Check if option expects a value
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                std::string value = argv[++i];

                // Validate value if validator exists
                if (options_[option_name].validator) {
                    if (!options_[option_name].validator(value)) {
                        std::cerr << "Invalid value for option " << arg << ": " << value << std::endl;
                        return false;
                    }
                }

                values_[option_name] = value;
            } else {
                // Boolean option
                values_[option_name] = "true";
            }
        } else {
            std::cerr << "Invalid argument: " << arg << std::endl;
            return false;
        }
    }

    // Check required options
    for (const auto& opt : options_) {
        if (opt.second.required && values_.find(opt.first) == values_.end()) {
            std::cerr << "Required option missing: --" << opt.first << std::endl;
            return false;
        }
    }

    return true;
}

void ProgramOptions::print_help() const {
    std::cout << "PCIe Simulator - C++ Interface Test" << std::endl;
    std::cout << "Copyright (c) 2025 Karan Mamaniya" << std::endl;
    std::cout << "====================================" << std::endl;
    std::cout << "Usage: " << program_name_ << " [OPTIONS]" << std::endl << std::endl;

    std::cout << "Options:" << std::endl;

    for (const auto& opt : options_) {
        std::string aliases_str;
        for (const auto& alias : aliases_) {
            if (alias.second == opt.first) {
                if (!aliases_str.empty()) aliases_str += ", ";
                aliases_str += "-" + alias.first;
            }
        }

        std::cout << "  --" << std::left << std::setw(15) << opt.first;
        if (!aliases_str.empty()) {
            std::cout << " " << std::setw(10) << aliases_str;
        } else {
            std::cout << " " << std::setw(10) << "";
        }
        std::cout << " " << opt.second.description;

        if (!opt.second.default_value.empty()) {
            std::cout << " (default: " << opt.second.default_value << ")";
        }
        std::cout << std::endl;
    }

    std::cout << std::endl << "Examples:" << std::endl;
    std::cout << "  " << program_name_ << "                        # Use default settings" << std::endl;
    std::cout << "  " << program_name_ << " --num-devices 1       # Test single device" << std::endl;
    std::cout << "  " << program_name_ << " --num-devices 8       # Test all 8 devices" << std::endl;
    std::cout << "  " << program_name_ << " -d 4                  # Test 4 devices (short form)" << std::endl;
    std::cout << "  " << program_name_ << " --pattern small-fast  # Small/fast transfers" << std::endl;
    std::cout << "  " << program_name_ << " --pattern custom --size 2048 --rate 1000" << std::endl;
    std::cout << "  " << program_name_ << " --threads 8 --duration 60  # Stress test" << std::endl;
    std::cout << "  " << program_name_ << " --log-csv results.csv  # Log to CSV file" << std::endl;
    std::cout << "  " << program_name_ << " --error-scenario timeout # Inject timeout errors" << std::endl;
}

bool ProgramOptions::has_option(const std::string& name) const {
    return values_.find(name) != values_.end();
}

std::unique_ptr<pcie_sim_test_config> ProgramOptions::to_config() const {
    auto config = std::unique_ptr<pcie_sim_test_config>(new pcie_sim_test_config());
    pcie_sim_config_init(config.get());

    // Set basic parameters
    config->num_devices = get<int>("num-devices");

    // Set transfer pattern
    std::string pattern_str = get<std::string>("pattern");
    pcie_sim_pattern_t pattern = pcie_sim_parse_pattern(pattern_str.c_str());

    if (pattern == PCIE_SIM_PATTERN_CUSTOM) {
        pcie_sim_config_set_custom_pattern(config.get(),
                                          get<int>("size"),
                                          get<int>("rate"));
    } else {
        pcie_sim_config_set_pattern(config.get(), pattern);
    }

    // Set error scenario
    std::string error_str = get<std::string>("error-scenario");
    pcie_sim_error_scenario_t error_scenario = pcie_sim_parse_error_scenario(error_str.c_str());
    pcie_sim_config_set_error_scenario(config.get(), error_scenario);

    // Set stress testing parameters
    int num_threads = get<int>("threads");
    if (num_threads > 1) {
        config->stress.load_type = PCIE_SIM_LOAD_STRESS;
        config->stress.num_threads = num_threads;
        config->stress.duration_seconds = get<int>("duration");
        config->flags |= PCIE_SIM_CONFIG_ENABLE_STRESS;
    }

    // Set logging configuration
    std::string csv_file = get<std::string>("log-csv");
    if (!csv_file.empty()) {
        strncpy(config->logging.csv_filename, csv_file.c_str(),
                sizeof(config->logging.csv_filename) - 1);
        config->flags |= PCIE_SIM_CONFIG_ENABLE_LOGGING;
    }

    // Set verbosity
    if (has_option("verbose")) {
        config->flags |= PCIE_SIM_CONFIG_VERBOSE;
    }

    return config;
}

std::unique_ptr<ProgramOptions> ProgramOptions::create_otpu_options() {
    auto options = std::unique_ptr<ProgramOptions>(new ProgramOptions());

    // Device configuration
    options->add_option("num-devices",
        Option("Number of devices to test (1-8)", "1", false,
            [](const std::string& value) {
                int num = std::stoi(value);
                return num >= 1 && num <= 8;
            }));

    // Transfer pattern options
    options->add_option("pattern",
        Option("Transfer pattern: small-fast, large-burst, mixed, custom", "mixed", false,
            [](const std::string& value) {
                return value == "small-fast" || value == "large-burst" ||
                       value == "mixed" || value == "custom";
            }));

    options->add_option("size",
        Option("Custom transfer size in bytes (64-4194304)", "4096", false,
            [](const std::string& value) {
                int size = std::stoi(value);
                return size >= 64 && size <= 4194304;
            }));

    options->add_option("rate",
        Option("Custom transfer rate in Hz (1-10000)", "1000", false,
            [](const std::string& value) {
                int rate = std::stoi(value);
                return rate >= 1 && rate <= 10000;
            }));

    // Logging options
    options->add_option("log-csv",
        Option("Log results to CSV file", "", false));

    options->add_option("verbose",
        Option("Enable verbose output", "false", false));

    // Error injection options
    options->add_option("error-scenario",
        Option("Error injection: timeout, corruption, overrun, none", "none", false,
            [](const std::string& value) {
                return value == "timeout" || value == "corruption" ||
                       value == "overrun" || value == "none";
            }));

    // Stress testing options
    options->add_option("threads",
        Option("Number of concurrent threads for stress testing", "1", false,
            [](const std::string& value) {
                int threads = std::stoi(value);
                return threads >= 1 && threads <= 64;
            }));

    options->add_option("duration",
        Option("Test duration in seconds", "10", false,
            [](const std::string& value) {
                int duration = std::stoi(value);
                return duration >= 1 && duration <= 3600;
            }));

    // Add convenient aliases
    options->add_alias("d", "num-devices");
    options->add_alias("p", "pattern");
    options->add_alias("s", "size");
    options->add_alias("r", "rate");
    options->add_alias("l", "log-csv");
    options->add_alias("v", "verbose");
    options->add_alias("e", "error-scenario");
    options->add_alias("t", "threads");
    options->add_alias("dur", "duration");

    return options;
}

} // namespace PCIeSimulator