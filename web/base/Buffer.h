//
// Created by 37496 on 2024/2/10.
//

#ifndef WEBSERVER_BUFFER_H
#define WEBSERVER_BUFFER_H

#include <vector>
#include <atomic>
#include <memory>
#include <string>
#include <cassert>
#include <algorithm>
#include "base/Type.h"

namespace Tiny_muduo
{

        /// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
        ///
        /// @code
        /// +-------------------+------------------+------------------+
        /// | prependable bytes |  readable bytes  |  writable bytes  |
        /// |                   |     (CONTENT)    |                  |
        /// +-------------------+------------------+------------------+
        /// |                   |                  |                  |
        /// 0      <=      readerIndex   <=   writerIndex    <=     size
        /// @endcode
        class Buffer
        {
        public:
            static const size_t kCheapPrepend;
            static const size_t kInitialSize;
            using _ptr = std::shared_ptr<Buffer>;

            explicit Buffer(size_t initSize = kInitialSize);

            void swap(Buffer& rhs);

            size_t writableBytes() const { return _buffer.size() - _writerIndex; }         //可写字节数
            size_t readableBytes() const { return _writerIndex - _readerIndex; }               //可读字节数
            size_t prependableBytes() const { return _readerIndex; }                        //头部预留字节数

            //返回读指针
            const char* peek() const { return Begin() + _readerIndex; }

            const char* findCRLF() const;                       // 查找 `\r\n`
            const char* findCRLF(const char* start) const;      // 从 start 开始查找 `\r\n`
            const char* findEOL() const;                        // 查找 `\n`
            const char* findEOL(const char* start) const;       // 从 start 开始查找 `\n`

            void retrieve(size_t len);                          //取len长度数据
            void retrieveUntil(const char* end);
            void retrieveAll();
            std::string retrieveAllToStr();
            std::string toString();
            std::string retrieveAsString(size_t len);

            void retrieveInt64() { retrieve(sizeof(int64_t)); }
            void retrieveInt32(){ retrieve(sizeof(int32_t)); }
            void retrieveInt16() { retrieve(sizeof(int16_t)); }
            void retrieveInt8() { retrieve(sizeof(int8_t)); }

            char* beginWrite() { return Begin() + _writerIndex; }
            const char* beginWrite() const { return Begin() + _writerIndex; }

            void hasWritten(size_t len) {  _writerIndex += len;}        //写len长度数据
            void unWrite(size_t len) { assert(len <= readableBytes()); _writerIndex -= len; }


            void append(const std::string& str);
            void append(const char* str, size_t len);
            void append(const void* str, size_t len);
            void append(const Buffer& buf);

            ssize_t readFd(int fd, int* Errno);
            ssize_t writeFd(int fd, int* Errno);
            ssize_t readFdET(int fd, int* Errno);
            ssize_t writeFdET(int fd, int* Errno);
            void ensureWritable(size_t len);    //确保足够长度可写
            void shrink(size_t reserve);
        private:
            char* Begin() { return &*_buffer.begin(); }
            const char* Begin() const { return &*_buffer.begin(); }

            void makeSpace(size_t len);

            std::size_t roundUp2Power(std::size_t size);

        private:
            std::vector<char> _buffer;
            std::size_t _readerIndex;
            std::size_t _writerIndex;

            static const char kCRLF[];

        };

}


#endif //WEBSERVER_BUFFER_H
