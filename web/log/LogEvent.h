//
// Created by 37496 on 2024/1/30.
//

#ifndef WEBSERVER_LOGEVENT_H
#define WEBSERVER_LOGEVENT_H

#include <cstdint>
#include <csignal>
#include <sys/syscall.h>
#include <cassert>
#include "LogStream.h"
#include "Loglevel.h"
#include "base/TimeStamp.h"
#include "LogUtil.h"

namespace Tiny_muduo
{
    namespace log
    {
        inline LogStream& operator<<(LogStream& s, T v)
        {
            s.append(v.str_, static_cast<int>(v.len_));
            return s;
        }

        inline LogStream& operator<<(LogStream& s, SourceFile v) {
            s.append(v._data, v._size);
            return s;
        }
        class LogEvent : public Noncopyable
        {
        public:
            LogEvent(LogLevel::Level level, const SourceFile& file, int line, long thread_id)
                            : _level(level), _file(file), _stream(), _line(line),
                            _thread_id(thread_id), _time(TimeStamp::now())
            {
                _stream << _time.toFormatString() << ' ' << _thread_id << ' ' << T(LogLevel::toString(_level), 7) << ' ' << _file << " : " << _line << " -> ";
            }

            LogStream& getStream() {
                return _stream;
            }

            LogLevel::Level getLevel() const
            {
                return _level;
            }

            const SourceFile& getFilename() const
            {
                return _file;
            }

            int getLine() const
            {
                return _line;
            }

            long getThreadId() const
            {
                return _thread_id;
            }

            TimeStamp getTime() const
            {
                return _time;
            }
            const std::string& getContent() const
            {
                return _content;
            }


        private:
            TimeStamp _time;
            LogStream _stream;
            LogLevel::Level _level;
            SourceFile _file;
            int _line;
            long _thread_id;
            std::string _content;
        };
    }
}

#endif //WEBSERVER_LOGEVENT_H
