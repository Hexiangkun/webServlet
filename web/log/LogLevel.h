//
// Created by 37496 on 2024/1/30.
//

#ifndef WEBSERVER_LOGLEVEL_H
#define WEBSERVER_LOGLEVEL_H

#include <string>


namespace HLog
{
    class LogLevel
    {
    public:
        enum Level{
            TRACE,
            DEBUG,
            INFO,
            WARN,
            ERROR,
            FATAL,
            UNKONWN,
        };

        LogLevel() = default;
        ~LogLevel() = default;

        static const char* toString(LogLevel::Level level) {
            switch (level) {
                case TRACE:
                    return "[TRACE]";
                case DEBUG:
                    return "[DEBUG]";
                case INFO:
                    return "[INFO ]";
                case WARN:
                    return "[WARN ]";
                case ERROR:
                    return "[ERROR]";
                case FATAL:
                    return "[FATAL]";
                default:
                    return "UNKNOWN";
            }
            return "UNKNOWN";
        }
    };
}


#endif //WEBSERVER_LOGLEVEL_H
