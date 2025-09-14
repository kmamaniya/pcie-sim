/*
 * PCIe Simulator - Performance Monitoring Interface
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

#ifndef MONITOR_H
#define MONITOR_H

#include "device.hpp"
#include <chrono>
#include <thread>
#include <atomic>
#include <functional>
#include <iostream>
#include <iomanip>

namespace PCIeSimulator {

struct PerformanceMetrics {
    uint64_t transfers;
    uint64_t bytes;
    uint64_t errors;
    double throughput_mbps;
    double latency_avg_us;
    double latency_min_us;
    double latency_max_us;
    double error_rate;

    void print(std::ostream& os = std::cout) const {
        os << std::fixed << std::setprecision(2)
           << "Transfers: " << transfers << "\n"
           << "Bytes: " << bytes << " (" << (bytes / 1024.0 / 1024.0) << " MB)\n"
           << "Throughput: " << throughput_mbps << " Mbps\n"
           << "Latency - Avg: " << latency_avg_us << " μs, "
           << "Min: " << latency_min_us << " μs, "
           << "Max: " << latency_max_us << " μs\n"
           << "Error Rate: " << (error_rate * 100.0) << "%\n";
    }
};

class PerformanceMonitor {
public:
    explicit PerformanceMonitor(Device& device) : device_(device) {}

    PerformanceMetrics get_current_metrics() const {
        auto stats = device_.get_statistics();

        PerformanceMetrics metrics{};
        metrics.transfers = stats.total_transfers();
        metrics.bytes = stats.total_bytes();
        metrics.errors = stats.total_errors();
        metrics.throughput_mbps = stats.throughput_mbps();
        metrics.latency_avg_us = stats.avg_latency_ns() / 1000.0;
        metrics.latency_min_us = stats.min_latency_ns() / 1000.0;
        metrics.latency_max_us = stats.max_latency_ns() / 1000.0;
        metrics.error_rate = metrics.transfers > 0 ?
            static_cast<double>(metrics.errors) / metrics.transfers : 0.0;

        return metrics;
    }

    void start_monitoring(std::chrono::milliseconds interval = std::chrono::milliseconds(1000),
                         std::function<void(const PerformanceMetrics&)> callback = nullptr) {
        stop_monitoring();
        monitoring_ = true;

        monitor_thread_ = std::thread([this, interval, callback]() {
            while (monitoring_) {
                auto metrics = get_current_metrics();

                if (callback) {
                    callback(metrics);
                } else {
                    std::cout << "\n=== Performance Metrics ===" << std::endl;
                    metrics.print();
                    std::cout << std::endl;
                }

                std::this_thread::sleep_for(interval);
            }
        });
    }

    void stop_monitoring() {
        if (monitoring_) {
            monitoring_ = false;
            if (monitor_thread_.joinable()) {
                monitor_thread_.join();
            }
        }
    }

    ~PerformanceMonitor() {
        stop_monitoring();
    }

private:
    Device& device_;
    std::atomic<bool> monitoring_{false};
    std::thread monitor_thread_;
};

class BenchmarkRunner {
public:
    explicit BenchmarkRunner(Device& device) : device_(device) {}

    struct BenchmarkConfig {
        size_t transfer_size;
        size_t num_transfers;
        Direction direction;
        bool warmup;
        size_t warmup_transfers;

        BenchmarkConfig() : transfer_size(4096), num_transfers(1000),
                           direction(Direction::TO_DEVICE), warmup(true),
                           warmup_transfers(100) {}
    };

    PerformanceMetrics run_benchmark(const BenchmarkConfig& config = BenchmarkConfig()) {
        std::vector<uint8_t> buffer(config.transfer_size);

        device_.reset_statistics();

        if (config.warmup) {
            for (size_t i = 0; i < config.warmup_transfers; ++i) {
                device_.transfer(buffer.data(), buffer.size(), config.direction);
            }
            device_.reset_statistics();
        }

        auto start_time = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < config.num_transfers; ++i) {
            device_.transfer(buffer.data(), buffer.size(), config.direction);
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end_time - start_time);

        auto stats = device_.get_statistics();

        PerformanceMetrics metrics{};
        metrics.transfers = stats.total_transfers();
        metrics.bytes = stats.total_bytes();
        metrics.errors = stats.total_errors();
        metrics.latency_avg_us = stats.avg_latency_ns() / 1000.0;
        metrics.latency_min_us = stats.min_latency_ns() / 1000.0;
        metrics.latency_max_us = stats.max_latency_ns() / 1000.0;
        metrics.error_rate = metrics.transfers > 0 ?
            static_cast<double>(metrics.errors) / metrics.transfers : 0.0;

        double total_time_s = duration.count() / 1e6;
        metrics.throughput_mbps = (metrics.bytes * 8.0) / (total_time_s * 1e6);

        return metrics;
    }

private:
    Device& device_;
};

} // namespace PCIeSimulator

#endif // MONITOR_H