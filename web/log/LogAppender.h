//
// Created by 37496 on 2024/2/9.
//

#ifndef WEBSERVER_LOGAPPENDER_H
#define WEBSERVER_LOGAPPENDER_H

#include <iostream>
#include <memory>
#include "Loglevel.h"
#include "LogEvent.h"
#include "LogFormatter.h"
#include "test/Mutex.h"
#include "LogFile.h"
#include "AsyncLog.h"

namespace Tiny_muduo
{
    namespace log
    {
        class LogAppender
        {
        public:
            using _ptr = std::shared_ptr<LogAppender>;

            explicit LogAppender(LogLevel::Level level = LogLevel::DEBUG) :m_level(level) {}
            virtual ~LogAppender() = default;     //基类能够析构

            virtual void log(LogLevel::Level level, const LogEvent& ev) = 0;

            LogFormatter::_ptr getFormatter()
            {
                LockGuard<MutexLock> lock(m_mutex);
                return m_formatter;
            }

            void setFormatter(LogFormatter::_ptr formatter)
            {
                LockGuard<MutexLock> lock(m_mutex);
                m_formatter = std::move(formatter);
            }

        protected:
            LogLevel::Level m_level;
            LogFormatter::_ptr m_formatter;
            MutexLock m_mutex;
        };


        class StdoutLogAppender : public LogAppender
        {
        public:
            using _ptr = std::shared_ptr<StdoutLogAppender>;

            explicit StdoutLogAppender(LogLevel::Level level = LogLevel::DEBUG) : LogAppender(level) {}
            void log(LogLevel::Level level, const LogEvent& ev) override
            {
                if(level < m_level) {
                    return;
                }
                LockGuard<MutexLock> lock(m_mutex);
                LogStream stream;
                m_formatter->format(stream, ev);
                std::cout << stream.getBuffer().data();
                std::cout.flush();
            }
        };

        class FileLogAppender : public LogAppender
        {
        public:
            using _ptr = std::shared_ptr<FileLogAppender>;

            explicit FileLogAppender(const std::string& filename, LogLevel::Level level = LogLevel::DEBUG, off_t roll_size = 20*1024*1024)
                : LogAppender(level),m_filename(filename), _file(filename, roll_size) { }
            ~FileLogAppender() = default;
            void log(LogLevel::Level level, const LogEvent& ev)
            {
                if(level < m_level) {
                    return;
                }
                LockGuard<MutexLock> lock(m_mutex);
                LogStream stream;
                m_formatter->format(stream, ev);
                _file.append(stream.getBuffer().data(), stream.getBuffer().length());
                _file.flush();
            }

        private:
            std::string m_filename;
            LogFile _file;
        };

        class AsyncFileLogAppender : public LogAppender
        {
        public:
            using _ptr = std::shared_ptr<AsyncFileLogAppender>;

            explicit AsyncFileLogAppender(const std::string& filename, LogLevel::Level level = LogLevel::DEBUG,
                                          int flush_interval = 400, int roll_size = 20*1024*1024)
                                          : m_basename(filename), _asyncLog(filename, flush_interval, roll_size)
            {

            }
            ~AsyncFileLogAppender() override = default;
            void log(LogLevel::Level level, const LogEvent& ev) override
            {
                if(level < m_level) {
                    return;
                }
                LockGuard<MutexLock> lock(m_mutex);
                LogStream stream;
                m_formatter->format(stream, ev);
                _asyncLog.append(stream.getBuffer().data(), stream.getBuffer().length());
            }

        private:
            std::string m_basename;
            AsyncLog _asyncLog;
        };

    }
}


#endif //WEBSERVER_LOGAPPENDER_H
