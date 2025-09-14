/*
 * PCIe Simulator - C++ Options Parsing Utility
 *
 * Copyright (c) 2025 Karan Mamaniya <kmamaniya@gmail.com>
 * MIT License
 *
 * Modern C++ command-line options parser using STL containers
 */

#ifndef PCIE_SIM_OPTIONS_HPP
#define PCIE_SIM_OPTIONS_HPP

#include "config.h"
#include <string>
#include <map>
#include <functional>
#include <iostream>
#include <memory>

namespace PCIeSimulator {

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

    bool parse(int argc, char* argv[]);
    void print_help() const;
    bool has_option(const std::string& name) const;

    template<typename T>
    T get(const std::string& option_name) const;

    // Convert to C configuration structure
    std::unique_ptr<pcie_sim_test_config> to_config() const;

    // Setup standard OTPU test options
    static std::unique_ptr<ProgramOptions> create_otpu_options();
};

// Template specializations
template<>
inline int ProgramOptions::get<int>(const std::string& option_name) const {
    auto it = values_.find(option_name);
    if (it != values_.end()) {
        return std::stoi(it->second);
    }
    return 0;
}

template<>
inline std::string ProgramOptions::get<std::string>(const std::string& option_name) const {
    auto it = values_.find(option_name);
    if (it != values_.end()) {
        return it->second;
    }
    return "";
}

template<>
inline float ProgramOptions::get<float>(const std::string& option_name) const {
    auto it = values_.find(option_name);
    if (it != values_.end()) {
        return std::stof(it->second);
    }
    return 0.0f;
}

template<>
inline bool ProgramOptions::get<bool>(const std::string& option_name) const {
    auto it = values_.find(option_name);
    if (it != values_.end()) {
        return it->second == "true" || it->second == "1" || it->second == "yes";
    }
    return false;
}

} // namespace PCIeSimulator

#endif // PCIE_SIM_OPTIONS_HPP