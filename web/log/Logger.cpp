//
// Created by 37496 on 2024/1/31.
//
#include "Logger.h"


namespace HLog
{
    LogLevel::Level g_log_level = LogLevel::INFO;

    LogLevel::Level Logger::get_logLevel() {
        return g_log_level;
    }

    bool g_is_async = false;

    void Logger::setAsync() {
        g_is_async = true;
    }

    void Logger::setLogLevel(LogLevel::Level level) {
        g_log_level = level;
    }

    void defaultOutput(const LogStream::Buffer& buf)//默认向终端写日志
    {
        size_t n = fwrite(buf.data(), 1, static_cast<size_t>(buf.length()), stdout);
    }

    Logger::OutputFunc g_output_func = defaultOutput;

    void Logger::setOutputFunc(OutputFunc func)
    {
        g_output_func = func;
    }

    Logger::~Logger() {

        stream() << "\n";
        const LogStream::Buffer& buf(stream().getBuffer());
        g_output_func(buf);
    }
}
