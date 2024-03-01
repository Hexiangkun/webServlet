//
// Created by 37496 on 2024/1/30.
//

#ifndef WEBSERVER_LOG_H
#define WEBSERVER_LOG_H

#include "Logger.h"
#include "Loglevel.h"
#include "AsyncLog.h"
#include "LogStream.h"


#define SET_LOGLEVEL(x) Logger::setLogLevel(x);

#define SET_LOG_ASYNC(x) \
    {   \
        if(x != 0){  \
            static Tiny_muduo::log::AsyncLog g_async_;  \
            Tiny_muduo::log::Logger::setOutputFunc( [&](const Tiny_muduo::log::LogStream::Buffer& buf){ g_async_.append(buf.data(), buf.length());} );  \
            Tiny_muduo::log::Logger::setAsync(); \
        }   \
    }   \

#define LOG_TRACE if(Tiny_muduo::log::Logger::get_logLevel() <= Tiny_muduo::log::LogLevel::TRACE) \
    (Tiny_muduo::log::Logger(__FILE__, __LINE__,  Tiny_muduo::log::LogLevel::TRACE, __func__).stream())

#define LOG_DEBUG if(Tiny_muduo::log::Logger::get_logLevel() <= Tiny_muduo::log::LogLevel::DEBUG)   \
    (Tiny_muduo::log::Logger(__FILE__, __LINE__, Tiny_muduo::log::LogLevel::DEBUG, __func__).stream())

#define LOG_INFO Tiny_muduo::log::Logger(__FILE__, __LINE__, Tiny_muduo::log::LogLevel::INFO).stream()
#define LOG_WARN Tiny_muduo::log::Logger(__FILE__, __LINE__, Tiny_muduo::log::LogLevel::WARN).stream()
#define LOG_ERROR Tiny_muduo::log::Logger(__FILE__, __LINE__, Tiny_muduo::log::LogLevel::ERROR).stream()
#define LOG_FATAL Tiny_muduo::log::Logger(__FILE__, __LINE__, Tiny_muduo::log::LogLevel::FATAL).stream()



#endif //WEBSERVER_LOG_H
