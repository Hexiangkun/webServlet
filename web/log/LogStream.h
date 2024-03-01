//
// Created by 37496 on 2024/1/30.
//

#ifndef WEBSERVER_LOGSTREAM_H
#define WEBSERVER_LOGSTREAM_H

#include <iostream>
#include <cstring>
#include <string>
#include "base/Noncopyable.h"
#include "LogUtil.h"


namespace Tiny_muduo
{

    namespace log
    {
        class LogStream : public  Noncopyable
        {
        public:
            typedef LogBuffer<kSmallSize> Buffer;
            LogStream() = default;
            ~LogStream() = default;

            LogStream& operator<<(bool v);
            LogStream& operator<<(short);
            LogStream& operator<<(unsigned short);
            LogStream& operator<<(int);
            LogStream& operator<<(unsigned int);
            LogStream& operator<<(long);
            LogStream& operator<<(unsigned long);
            LogStream& operator<<(long long);
            LogStream& operator<<(unsigned long long);
            LogStream& operator<<(const void*);
            LogStream& operator<<(float);
            LogStream& operator<<(double);
            LogStream& operator<<(char);
            LogStream& operator<<(const char* str);
            LogStream& operator<<(const unsigned char*);
            LogStream& operator<<(const std::string&);
            LogStream& operator<<(const Buffer&);

            void append(const char* data, int len);
            const Buffer& getBuffer();
            void resetBuffer();

        private:
            template <typename T>
            void formatData(T num)
            {
                std::string res = std::to_string(num);
                _buffer.append(res.data(), res.size());
            }
        private:
            Buffer _buffer;
            static const int kMaxNumericSize = 48;
        };
    }
}

#endif //WEBSERVER_LOGSTREAM_H
