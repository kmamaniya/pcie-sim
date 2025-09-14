/*
 * PCIe Simulator - CSV Logging Utility
 *
 * Copyright (c) 2025 Karan Mamaniya <kmamaniya@gmail.com>
 * MIT License
 *
 * High-performance CSV logger for OTPU performance analysis
 */

#ifndef PCIE_SIM_CSV_LOGGER_HPP
#define PCIE_SIM_CSV_LOGGER_HPP

#include <string>
#include <fstream>
#include <chrono>
#include <mutex>
#include <vector>
#include <memory>

namespace PCIeSimulator {

struct TransferRecord {
    std::chrono::high_resolution_clock::time_point timestamp;
    uint32_t device_id;
    uint32_t transfer_size;
    double latency_us;
    double throughput_mbps;
    std::string direction;
    std::string error_status;
    uint32_t thread_id;

    TransferRecord() : device_id(0), transfer_size(0), latency_us(0.0),
                      throughput_mbps(0.0), direction("TO_DEVICE"),
                      error_status("SUCCESS"), thread_id(0) {}
};

class CSVLogger {
private:
    std::ofstream file_;
    std::mutex mutex_;
    std::string filename_;
    bool header_written_;
    size_t record_count_;
    std::chrono::high_resolution_clock::time_point session_start_;

    void write_header();
    std::string format_timestamp(const std::chrono::high_resolution_clock::time_point& tp) const;

public:
    explicit CSVLogger(const std::string& filename);
    ~CSVLogger();

    // Log individual transfer
    void log_transfer(const TransferRecord& record);

    // Log transfer with current timestamp
    void log_transfer(uint32_t device_id, uint32_t transfer_size, double latency_us,
                     double throughput_mbps, const std::string& direction = "TO_DEVICE",
                     const std::string& error_status = "SUCCESS", uint32_t thread_id = 0);

    // Batch logging for high performance
    void log_transfers(const std::vector<TransferRecord>& records);

    // Force flush to disk
    void flush();

    // Get statistics
    size_t get_record_count() const { return record_count_; }
    std::string get_filename() const { return filename_; }

    // Session management
    void log_session_start(const std::string& test_config);
    void log_session_end(const std::string& summary);

    // Static utility for creating timestamped filenames
    static std::string create_timestamped_filename(const std::string& prefix = "otpu_test",
                                                   const std::string& suffix = ".csv");
};

// RAII CSV Logger with automatic session management
class SessionLogger {
private:
    std::unique_ptr<CSVLogger> logger_;
    std::string session_config_;

public:
    SessionLogger(const std::string& filename, const std::string& config);
    ~SessionLogger();

    CSVLogger* get_logger() { return logger_.get(); }

    // Convenience methods
    void log_transfer(uint32_t device_id, uint32_t transfer_size, double latency_us,
                     double throughput_mbps, const std::string& direction = "TO_DEVICE",
                     const std::string& error_status = "SUCCESS", uint32_t thread_id = 0) {
        if (logger_) {
            logger_->log_transfer(device_id, transfer_size, latency_us, throughput_mbps,
                                direction, error_status, thread_id);
        }
    }
};

} // namespace PCIeSimulator

#endif // PCIE_SIM_CSV_LOGGER_HPP