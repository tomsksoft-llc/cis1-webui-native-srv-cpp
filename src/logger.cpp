/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2020 TomskSoft LLC
 *   (c) Sergey Boyko [bso@tomsksoft.com]
 *
 */

#include "logger.h"

#include <iostream>

#include <tpl_helpers/overloaded.h>
#include <scf/scf.h>

using RecordersCont = scl::RecordersCont<WebuiRecord>;

LoggerPtr logger_instance;

void init_log(
        const WebuiLogger::Options& options,
        const std::optional<FileRecorder::Options>& file_options,
        const std::optional<ConsoleRecorder::Options>& console_options)
{
    const auto on_error
            = [](std::string_view error)
            {
                std::cerr << "Couldn't create a cis log: error = " << error << std::endl;
                exit(1);
            };

    RecordersCont recorders;

    if(file_options)
    {
        auto recorder = std::visit(
                meta::overloaded{
                        [](FileRecorderPtr&& val)
                        { return std::move(val); },
                        [&on_error](const auto& error)
                        {
                            on_error(FileRecorder::ToStr(error));
                            return FileRecorderPtr{};
                        }
                },
                FileRecorder::Init(file_options.value())
        );

        recorders.push_back(std::move(recorder));
    }

    if(console_options)
    {
        auto recorder = ConsoleRecorder::Init(console_options.value());
        recorders.push_back(std::move(recorder));
    }

    logger_instance = std::visit(
            meta::overloaded{
                    [](LoggerPtr&& val)
                    { return std::move(val); },
                    [&on_error](const auto& error)
                    {
                        on_error(WebuiLogger::ToStr(error));
                        return LoggerPtr{};
                    },
            },
            WebuiLogger::Init(options, std::move(recorders))
    );

}

void log(
        scl::Level level,
        const std::string& message)
{
    if(!logger_instance)
    {
        return;
    }

    logger_instance->Record(level, message);
}

void ex_log(
        scl::Level level,
        Protocol protocol,
        const std::string& handler,
        request_context& ctx,
        const std::string& message)
{
    if(!logger_instance)
    {
        return;
    }

    std::optional<std::string> email;
    if(ctx.client_info)
    {
        email = ctx.client_info.value().email;
    }

    const auto remote_addr = SCFormat("%s:%d", ctx.remote_addr.first, ctx.remote_addr.second);

    logger_instance->ExRecord(level, protocol, handler, remote_addr, email, message);
}
