//
// Created by 37496 on 2024/1/31.
//

#ifndef WEBSERVER_LOGGER_H
#define WEBSERVER_LOGGER_H

#include <memory>
#include <functional>
#include "LogStream.h"
#include "LogEvent.h"
#include "LogUtil.h"


namespace HLog
{
    class Logger
    {
    public:
        using _ptr = std::shared_ptr<Logger>;
        Logger(SourceFile sourcefile, int line) : _event(LogLevel::INFO, sourcefile, line, getThreadId()) {

        }
        Logger(SourceFile sourcefile, int line, LogLevel::Level level) : _event(level, sourcefile, line, getThreadId()) {

        }
        Logger(SourceFile sourcefile, int line, LogLevel::Level level, const char* func_name) : _event(level, sourcefile, line, getThreadId()) {
            _event.getStream() << func_name << ' ';
        }

        ~Logger() ;

        LogStream& stream() {
            return _event.getStream();
        }


        static LogLevel::Level get_logLevel();
        static void setLogLevel(LogLevel::Level level);
        static void setAsync();
        typedef std::function<void(const LogStream::Buffer&)> OutputFunc;
        static void setOutputFunc(OutputFunc func);


    private:
        LogEvent _event;
        static bool is_async;
    };
}


#endif //WEBSERVER_LOGGER_H
