//
// Created by 37496 on 2024/2/9.
//

#ifndef WEBSERVER_LOGUTIL_H
#define WEBSERVER_LOGUTIL_H

#include <string>
#include <cstring>
#include <cassert>
#include "base/Noncopyable.h"

namespace Tiny_muduo
{
    namespace log
    {
        const int kSmallSize = 4000;
        const int kLargeSize = 4000 * 1000;

        template<int SIZE>
        class LogBuffer : public Noncopyable
        {
        public:
            LogBuffer():_cur(_data)
            {
            }

            ~LogBuffer() {

            }

            void append(const char* buf, size_t len) {
                if (static_cast<size_t>(available()) > len) {
                    memcpy(_cur, buf, len);
                    _cur += len;
                }
            }

            const char* data() const { return _data; }

            int length() const { return static_cast<int>(_cur - _data); }


            void add(size_t len) { _cur += len; }
            char* current() { return _cur; }
            int available() const { return static_cast<int>(end() - _cur); }

            void reset()
            {
                clear();
                _cur = _data;
            }

            void clear()
            {
                memset(_data, 0, sizeof(_data));
            }

            const char* debugString() {
                *_cur = '\0';
                return _data;
            }
            std::string toString() const { return std::string(_data); }


            void setCookie(void (*cookie)()) { _cookie = cookie; }

        private:
            const char* end() const
            {
                return _data + sizeof(_data);
            }


        private:
            char _data[SIZE];
            char* _cur;
            void (*_cookie)();
        };

        class SourceFile
        {
        public:
            template<int N>
            SourceFile(const char (&arr)[N]):_data(arr),_size(N-1)
            {
                const char* tmp = strrchr(_data, '/');
                if(tmp) {
                    _data = tmp + 1;
                }
                _size = static_cast<int>(strlen(_data));
            }

            explicit SourceFile(const char* filename):_data(filename)
            {
                const char* tmp = strrchr(filename, '/');
                if(tmp) {
                    _data = tmp + 1;
                }
                _size = static_cast<int>(strlen(_data));
            }
            ~SourceFile() = default;

        public:
            const char* _data;
            int _size;
        };

        class T
        {
        public:
            T(const char* str, unsigned len)
                    :str_(str),
                     len_(len)
            {
                assert(strlen(str) == len_);
            }

            const char* str_;
            const unsigned len_;
        };


    }
}

#endif //WEBSERVER_LOGUTIL_H
