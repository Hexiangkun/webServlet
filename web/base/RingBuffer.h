//
// Created by 37496 on 2024/3/4.
//

#ifndef WEBSERVER_RINGBUFFER_H
#define WEBSERVER_RINGBUFFER_H

#include <algorithm>
#include <assert.h>
#include <string.h>
#include <string>
#include <vector>

namespace Tiny_muduo
{
    class RingBuffer
    {
    public:
        static const size_t kCheapPrepend = 1;
        static const size_t kInitialSize = 1024;

        RingBuffer();
        ~RingBuffer();

        size_t writableBytes() const ;        //可写字节数
        size_t readableBytes() const ;        //可读字节数
        size_t prependableBytes() const { return _readerIndex; }

        const char* peek() const { return _buffer + _readerIndex; }
        void retrieve(size_t len) {
            assert(len <= readableBytes());
            _readerIndex += len;
            _readerIndex %= _capacity;
        }

        void retrieveUntil(const char *end) {
            // !!! 没有做合法性检查
            if ((end >= begin()) && end < peek()) {
                retrieve(end - peek() + _capacity);
            }
            retrieve(end - peek());
        }

        void retrieveAll() {
            _readerIndex = kCheapPrepend;
            _writerIndex = kCheapPrepend;
        }

        std::string retrieveAsString() {
            if (_writerIndex < _readerIndex) {
                std::string str(peek(), _capacity - _readerIndex);
                std::string strbegin(_buffer, _writerIndex);
                str += strbegin;
                retrieveAll();
                return str;
            }

            std::string str(peek(), _writerIndex - _readerIndex);
            retrieveAll();
            return str;
        }

        std::string bufferToString() {
            if (_writerIndex < _readerIndex) {
                std::string str(peek(), _capacity - _readerIndex);
                std::string strbegin(_buffer, _writerIndex);
                str += strbegin;
                return str;
            }

            std::string str(peek(), _writerIndex - _readerIndex);
            return str;
        }

        void append(const std::string &str) { append(str.data(), str.length()); }

        // append 主要做了三点
        // 确保缓存拥有足够的空间存储新数据
        // 拷贝数据到可写位置
        // 更新writeIndex位置
        void append(const char * /*restrict*/ data, size_t len) {
            ensureWritableBytes(len);
            // 可以和std::copy对比下，看看哪个性能更高
            if (_writerIndex < _readerIndex) {
                memcpy(beginWrite(), data, len);
            } else {
                // 看看尾部预留的空间大小
                size_t reserve_tail = _capacity - _writerIndex;
                if (reserve_tail >= len) {
                    memcpy(beginWrite(), data, len);
                    _writerIndex += len;
                } else {
                    memcpy(beginWrite(), data, reserve_tail);
                    // _writerIndex 先一步更新完毕
                    _writerIndex = len - reserve_tail;
                    memcpy(_buffer, data + reserve_tail, _writerIndex);
                }
            }
        }

        void append(const void * /*restrict*/ data, size_t len) {
            append(static_cast<const char *>(data), len);
        }

        // 1. 当剩余可写空间小于写入数据时，系统要对缓存进行扩容，
        //     执行了makeSpace这个方法，参数len是要写入数据的长度
        // 2. 如果可写长度够用呢，就无需分配新空间了
        void ensureWritableBytes(size_t len) {
            if (writableBytes() < len) {
                makeSpace(len);
            }
            assert(writableBytes() >= len);
        }

        char *beginWrite() { return begin() + _writerIndex; }

        const char *beginWrite() const { return begin() + _writerIndex; }

        /// 将数据读入buffer, 适合LT
        ssize_t readFd(int fd, int *savedErrno);
        /// 将数据读入buffer, 适合ET
        ssize_t readFdET(int fd, int *savedErrno);

        /// 将数据读出buffer, 适合LT
        ssize_t writeFd(int fd, int *savedErrno);
        /// 将数据读出buffer, 适合ET
        ssize_t writeFdET(int fd, int *savedErrno);
    private:
        char *begin() { return _buffer; }

        const char *begin() const { return _buffer; }

        char *end() { return _buffer + _capacity; }
        const char *end() const { return _buffer + _capacity; }

        // makeSpace
        // 重新分配空间，然后修改readIndex和writerIndex
        void makeSpace(size_t len) {
            // 增长空间
            size_t n = (_capacity << 1) + len;
            char *newbuffer_ = new char[n];
            if (_writerIndex >= _readerIndex) {
                memcpy(newbuffer_ + kCheapPrepend, _buffer + _readerIndex,
                       (_writerIndex - _readerIndex));
            } else {
                // writerIndex_在前的时候需要考虑 可读数据分为后 前两段的情况
                memcpy(newbuffer_ + kCheapPrepend, _buffer + _readerIndex,
                       _capacity - _readerIndex);
                memcpy(newbuffer_ + kCheapPrepend + _capacity - _readerIndex, _buffer,
                       _writerIndex);
            }
            size_t readable_bytes = readableBytes();
            _readerIndex = kCheapPrepend;
            _writerIndex = _readerIndex + readable_bytes;
            _capacity = n;
            delete[] _buffer;
            _buffer = newbuffer_;
        }

        size_t _readerIndex;
        size_t _writerIndex;
        size_t _capacity;
        char* _buffer;
        static const char kCRLF[];
    };
}


#endif //WEBSERVER_RINGBUFFER_H
