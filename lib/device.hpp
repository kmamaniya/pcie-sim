/*
 * PCIe Simulator - C++ Device Interface
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

#ifndef DEVICE_H
#define DEVICE_H

#include <memory>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstdint>

extern "C" {
#include "pcie_sim.h"
}

namespace PCIeSimulator {

class DeviceError : public std::runtime_error {
public:
    explicit DeviceError(pcie_sim_error_t error_code)
        : std::runtime_error(pcie_sim_error_string(error_code))
        , error_code_(error_code) {}

    pcie_sim_error_t code() const { return error_code_; }

private:
    pcie_sim_error_t error_code_;
};

class Statistics {
public:
    Statistics(const pcie_sim_stats& stats) : stats_(stats) {}

    uint64_t total_transfers() const { return stats_.total_transfers; }
    uint64_t total_bytes() const { return stats_.total_bytes; }
    uint64_t total_errors() const { return stats_.total_errors; }
    uint64_t avg_latency_ns() const { return stats_.avg_latency_ns; }
    uint64_t min_latency_ns() const { return stats_.min_latency_ns; }
    uint64_t max_latency_ns() const { return stats_.max_latency_ns; }

    double throughput_mbps() const {
        if (total_transfers() == 0) return 0.0;
        return (total_bytes() * 8.0 * 1000.0) / (avg_latency_ns() * total_transfers());
    }

private:
    pcie_sim_stats stats_;
};

enum class Direction {
    TO_DEVICE = PCIE_SIM_TO_DEVICE,
    FROM_DEVICE = PCIE_SIM_FROM_DEVICE
};

class Device {
public:
    explicit Device(int device_id = 0) {
        pcie_sim_error_t err = pcie_sim_open(device_id, &handle_);
        if (err != PCIE_SIM_SUCCESS) {
            throw DeviceError(err);
        }
    }

    ~Device() {
        if (handle_) {
            pcie_sim_close(handle_);
        }
    }

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    Device(Device&& other) noexcept : handle_(other.handle_) {
        other.handle_ = nullptr;
    }

    Device& operator=(Device&& other) noexcept {
        if (this != &other) {
            if (handle_) {
                pcie_sim_close(handle_);
            }
            handle_ = other.handle_;
            other.handle_ = nullptr;
        }
        return *this;
    }

    uint64_t transfer(void* buffer, size_t size, Direction direction) {
        uint64_t latency_ns;
        pcie_sim_error_t err = pcie_sim_transfer(
            handle_, buffer, size,
            static_cast<uint32_t>(direction),
            &latency_ns
        );

        if (err != PCIE_SIM_SUCCESS) {
            throw DeviceError(err);
        }

        return latency_ns;
    }

    template<typename T>
    uint64_t write(const std::vector<T>& data) {
        return transfer(const_cast<T*>(data.data()),
                       data.size() * sizeof(T),
                       Direction::TO_DEVICE);
    }

    template<typename T>
    uint64_t read(std::vector<T>& data) {
        return transfer(data.data(),
                       data.size() * sizeof(T),
                       Direction::FROM_DEVICE);
    }

    Statistics get_statistics() const {
        pcie_sim_stats stats;
        pcie_sim_error_t err = pcie_sim_get_stats(handle_, &stats);
        if (err != PCIE_SIM_SUCCESS) {
            throw DeviceError(err);
        }
        return Statistics(stats);
    }

    void reset_statistics() {
        pcie_sim_error_t err = pcie_sim_reset_stats(handle_);
        if (err != PCIE_SIM_SUCCESS) {
            throw DeviceError(err);
        }
    }

    bool is_valid() const { return handle_ != nullptr; }

private:
    pcie_sim_handle_t handle_ = nullptr;
};

class DeviceManager {
public:
    static std::unique_ptr<Device> open_device(int device_id = 0) {
        return std::unique_ptr<Device>(new Device(device_id));
    }

    static std::vector<std::unique_ptr<Device>> open_all_devices(int max_devices = 8) {
        std::vector<std::unique_ptr<Device>> devices;

        for (int i = 0; i < max_devices; ++i) {
            try {
                devices.push_back(std::unique_ptr<Device>(new Device(i)));
            } catch (const DeviceError&) {
                break;
            }
        }

        return devices;
    }
};

} // namespace PCIeSimulator

#endif // DEVICE_H