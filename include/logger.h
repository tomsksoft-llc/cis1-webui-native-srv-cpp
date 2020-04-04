/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2020 TomskSoft LLC
 *   (c) Sergey Boyko [bso@tomsksoft.com]
 *
 */

#pragma once

#include <filesystem>
#include <scf/scf.h>
#include <scl/file_recorder.h>
#include <scl/console_recorder.h>
#include <cis1_webui_logger/webui_logger.h>
#include <cis1_webui_logger/webui_record.h>
#include "request_context.h"

#define LOG(level, format, ...) \
log(level, SCFormat(format, ##__VA_ARGS__))

using LoggerPtr = cis1::webui_logger::LoggerPtr;
using WebuiLogger = cis1::webui_logger::WebuiLogger;
using WebuiRecord = cis1::webui_logger::WebuiRecord;
using Protocol = cis1::webui_logger::Protocol;

using FileRecorder = scl::FileRecorder<WebuiRecord>;
using FileRecorderPtr = scl::FileRecorderPtr<WebuiRecord>;
using ConsoleRecorder = scl::ConsoleRecorder<WebuiRecord>;
using ConsoleRecorderPtr = scl::ConsoleRecorderPtr<WebuiRecord>;

/**
 * \brief Initialize webui logging
 * @param[in] options
 */
void init_log(
        const WebuiLogger::Options& options,
        const std::optional<FileRecorder::Options>& file_options,
        const std::optional<ConsoleRecorder::Options>& console_options);

/**
 * \brief Record the system message to log
 * @param[in] level
 * @param[in] protocol
 * @param[in] handler
 * @param[in] message
 */
void log(
        scl::Level level,
        const std::string& message);

/**
 * \brief Record the message to log with the user info (address, email)
 * @param[in] level
 * @param[in] protocol
 * @param[in] handler
 * @param[in] message
 */
void ex_log(
        scl::Level level,
        Protocol protocol,
        const std::string& handler,
        request_context& ctx,
        const std::string& message);

extern LoggerPtr logger_instance;
