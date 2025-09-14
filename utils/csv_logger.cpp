/*
 * PCIe Simulator - CSV Logger Implementation
 *
 * Copyright (c) 2025 Karan Mamaniya <kmamaniya@gmail.com>
 * MIT License
 */

#include "csv_logger.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <thread>

namespace PCIeSimulator {

CSVLogger::CSVLogger(const std::string& filename)
    : filename_(filename), header_written_(false), record_count_(0),
      session_start_(std::chrono::high_resolution_clock::now()) {

    file_.open(filename_, std::ios::out | std::ios::trunc);
    if (!file_.is_open()) {
        throw std::runtime_error("Failed to open CSV file: " + filename_);
    }

    write_header();
}

CSVLogger::~CSVLogger() {
    if (file_.is_open()) {
        flush();
        file_.close();
    }
}

void CSVLogger::write_header() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!header_written_ && file_.is_open()) {
        file_ << "timestamp,session_time_ms,device_id,transfer_size,latency_us,"
              << "throughput_mbps,direction,error_status,thread_id" << std::endl;
        header_written_ = true;
    }
}

std::string CSVLogger::format_timestamp(const std::chrono::high_resolution_clock::time_point& tp) const {
    auto time_t = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now() + (tp - std::chrono::high_resolution_clock::now()));
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

void CSVLogger::log_transfer(const TransferRecord& record) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!file_.is_open()) return;

    auto session_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        record.timestamp - session_start_).count();

    file_ << format_timestamp(record.timestamp) << ","
          << session_time_ms << ","
          << record.device_id << ","
          << record.transfer_size << ","
          << std::fixed << std::setprecision(3) << record.latency_us << ","
          << std::fixed << std::setprecision(2) << record.throughput_mbps << ","
          << record.direction << ","
          << record.error_status << ","
          << record.thread_id << std::endl;

    record_count_++;
}

void CSVLogger::log_transfer(uint32_t device_id, uint32_t transfer_size, double latency_us,
                           double throughput_mbps, const std::string& direction,
                           const std::string& error_status, uint32_t thread_id) {
    TransferRecord record;
    record.timestamp = std::chrono::high_resolution_clock::now();
    record.device_id = device_id;
    record.transfer_size = transfer_size;
    record.latency_us = latency_us;
    record.throughput_mbps = throughput_mbps;
    record.direction = direction;
    record.error_status = error_status;
    record.thread_id = thread_id;

    log_transfer(record);
}

void CSVLogger::log_transfers(const std::vector<TransferRecord>& records) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!file_.is_open()) return;

    for (const auto& record : records) {
        auto session_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            record.timestamp - session_start_).count();

        file_ << format_timestamp(record.timestamp) << ","
              << session_time_ms << ","
              << record.device_id << ","
              << record.transfer_size << ","
              << std::fixed << std::setprecision(3) << record.latency_us << ","
              << std::fixed << std::setprecision(2) << record.throughput_mbps << ","
              << record.direction << ","
              << record.error_status << ","
              << record.thread_id << std::endl;
    }

    record_count_ += records.size();
}

void CSVLogger::flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_.is_open()) {
        file_.flush();
    }
}

void CSVLogger::log_session_start(const std::string& test_config) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!file_.is_open()) return;

    file_ << "# Session Start: " << format_timestamp(session_start_) << std::endl;
    file_ << "# Configuration: " << test_config << std::endl;
    file_ << "# Columns: timestamp, session_time_ms, device_id, transfer_size, "
          << "latency_us, throughput_mbps, direction, error_status, thread_id" << std::endl;
}

void CSVLogger::log_session_end(const std::string& summary) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!file_.is_open()) return;

    auto session_end = std::chrono::high_resolution_clock::now();
    auto session_duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        session_end - session_start_).count();

    file_ << "# Session End: " << format_timestamp(session_end) << std::endl;
    file_ << "# Duration: " << session_duration_ms << " ms" << std::endl;
    file_ << "# Total Records: " << record_count_ << std::endl;
    file_ << "# Summary: " << summary << std::endl;
}

std::string CSVLogger::create_timestamped_filename(const std::string& prefix, const std::string& suffix) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << prefix << "_" << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S") << suffix;
    return ss.str();
}

// SessionLogger implementation

SessionLogger::SessionLogger(const std::string& filename, const std::string& config)
    : session_config_(config) {
    try {
        logger_ = std::unique_ptr<CSVLogger>(new CSVLogger(filename));
        logger_->log_session_start(config);
        std::cout << "Started logging session to: " << filename << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to create logger: " << e.what() << std::endl;
        logger_.reset();
    }
}

SessionLogger::~SessionLogger() {
    if (logger_) {
        std::stringstream summary;
        summary << "Session completed with " << logger_->get_record_count() << " transfers logged";
        logger_->log_session_end(summary.str());
        std::cout << "Completed logging session. Records: " << logger_->get_record_count() << std::endl;
    }
}

} // namespace PCIeSimulator