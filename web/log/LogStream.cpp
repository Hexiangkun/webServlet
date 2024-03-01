//
// Created by 37496 on 2024/1/30.
//
#include "LogStream.h"

namespace Tiny_muduo
{
    namespace log
    {
        LogStream& LogStream::operator<<(bool v)
        {
            if(v) {
                _buffer.append("true", 4);
            }
            else{
                _buffer.append("false", 5);
            }
            return *this;
        }
        LogStream& LogStream::operator<<(short v)
        {
            formatData(v);
            return *this;
        }
        LogStream& LogStream::operator<<(unsigned short v)
        {
            formatData(v);
            return *this;
        }
        LogStream& LogStream::operator<<(int v)
        {
            formatData(v);
            return *this;
        }
        LogStream& LogStream::operator<<(unsigned int v)
        {
            formatData(v);
            return *this;
        }
        LogStream& LogStream::operator<<(long v)
        {
            formatData(v);
            return *this;
        }

        LogStream& LogStream::operator<<(unsigned long v)
        {
            formatData(v);
            return *this;
        }
        LogStream& LogStream::operator<<(long long v)
        {
            formatData(v);
            return *this;
        }
        LogStream& LogStream::operator<<(unsigned long long v)
        {
            formatData(v);
            return *this;
        }

        LogStream& LogStream::operator<<(float v)
        {
            formatData(v);
            return *this;
        }

        LogStream& LogStream::operator<<(double v)
        {
            formatData(v);
            return *this;
        }

        LogStream& LogStream::operator<<(const void* ptr)
        {
            char buffer[20];
            sprintf(buffer, "%p", ptr);
            *this << std::string(buffer);
            return *this;
        }


        LogStream& LogStream::operator<<(char v)
        {
            _buffer.append(&v, 1);
            return *this;
        }

        LogStream& LogStream::operator<<(const char* str)
        {
            if(str) {
                _buffer.append(str, strlen(str));
            }
            else{
                _buffer.append("(nullptr)", 9);
            }
            return *this;
        }

        LogStream& LogStream::operator<<(const unsigned char* str)
        {
            *this << reinterpret_cast<const char*>(str);
            return *this;
        }

        LogStream& LogStream::operator<<(const std::string& str)
        {
            _buffer.append(str.c_str(), str.size());
            return *this;
        }

        LogStream& LogStream::operator<<(const Buffer& buf)
        {
            _buffer.append(buf.data(), buf.length());
            return *this;
        }

        void LogStream::append(const char* data, int len)
        {
            _buffer.append(data, len);
        }


        const LogStream::Buffer& LogStream::getBuffer()
        {
            return _buffer;
        }

        void LogStream::resetBuffer()
        {
            _buffer.reset();
        }
    }


}