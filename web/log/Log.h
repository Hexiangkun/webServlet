//
// Created by 37496 on 2024/1/30.
//

#ifndef WEBSERVER_LOG_H
#define WEBSERVER_LOG_H

#include "Logger.h"
#include "LogLevel.h"
#include "AsyncLog.h"
#include "LogStream.h"
#include "config/Config.h"


#define SET_LOGLEVEL(x) Logger::setLogLevel(x);

#define SET_LOG_ASYNC(x) \
    {   \
        if(x != 0){  \
            static HLog::AsyncLog g_async_( \
                    config::GET_CONFIG<std::string>("log.destination", "/root/webserver/log"), \
                    config::GET_CONFIG<std::string>("log.filename", "basename"));  \
            HLog::Logger::setOutputFunc( [&](const HLog::LogStream::Buffer& buf){ g_async_.append(buf.data(), buf.length());} );  \
            HLog::Logger::setAsync(); \
        }   \
    }   \

#define LOG_TRACE if(HLog::Logger::get_logLevel() <= HLog::LogLevel::TRACE) \
    (HLog::Logger(__FILE__, __LINE__,  HLog::LogLevel::TRACE, __func__).stream())

#define LOG_DEBUG if(HLog::Logger::get_logLevel() <= HLog::LogLevel::DEBUG)   \
    (HLog::Logger(__FILE__, __LINE__, HLog::LogLevel::DEBUG, __func__).stream())

#define LOG_INFO HLog::Logger(__FILE__, __LINE__, HLog::LogLevel::INFO).stream()
#define LOG_WARN HLog::Logger(__FILE__, __LINE__, HLog::LogLevel::WARN).stream()
#define LOG_ERROR HLog::Logger(__FILE__, __LINE__, HLog::LogLevel::ERROR).stream()
#define LOG_FATAL HLog::Logger(__FILE__, __LINE__, HLog::LogLevel::FATAL).stream()



#endif //WEBSERVER_LOG_H
