//
// Created by 37496 on 2024/2/10.
//

#include <limits>
#include <cassert>
#include <sys/uio.h>
#include <cstring>
#include <unistd.h>
#include "Buffer.h"


namespace Tiny_muduo
{
    const size_t Buffer::kCheapPrepend = 8;
    const std::size_t Buffer::kInitialSize = 1024;
    const char Buffer::kCRLF[] = "\r\n";

    Buffer::Buffer(size_t initSize)
        : _buffer(kCheapPrepend + initSize),
        _readerIndex(kCheapPrepend),
        _writerIndex(kCheapPrepend)
    {
        assert(readableBytes() == 0);
        assert(writableBytes() == kInitialSize);
        assert(prependableBytes() == kCheapPrepend);
    }

    void Buffer::swap(Tiny_muduo::Buffer &rhs) {
        _buffer.swap(rhs._buffer);
        std::swap(_readerIndex, rhs._readerIndex);
        std::swap(_writerIndex, rhs._writerIndex);
    }

    const char *Buffer::findCRLF() const {
        const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF+2);
        return crlf == beginWrite() ? nullptr : crlf;
    }

    const char *Buffer::findCRLF(const char *start) const {
        assert(peek() <= start);
        assert(start <= beginWrite());
        const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF+2);
        return crlf == beginWrite() ? nullptr : crlf;
        //数据可读处
    }

    const char* Buffer::findEOL() const
    {
        const void* eol = memchr(peek(), '\n', readableBytes());
        return static_cast<const char*>(eol);
    }

    const char* Buffer::findEOL(const char* start) const
    {
        assert(peek() <= start);
        assert(start <= beginWrite());
        const void* eol = memchr(start, '\n', beginWrite() - start);
        return static_cast<const char*>(eol);
    }

    //取回数据
    void Buffer::retrieve(size_t len) {
        assert(len <= readableBytes());
        if(len < readableBytes()) {
            _readerIndex += len;
        }
        else {
            retrieveAll();
        }
    }

    void Buffer::retrieveUntil(const char *end) {
        assert(peek() <= end);
        assert(end <= beginWrite());
        retrieve(static_cast<size_t>(end - peek()));
    }

    void Buffer::retrieveAll() {
        _readerIndex = kCheapPrepend;
        _writerIndex = kCheapPrepend;
    }



    std::string Buffer::retrieveAllToStr() {
        std::string str(peek(), readableBytes());
        retrieveAll();
        return str;
    }

    std::string Buffer::toString() {
        std::string str(peek(), readableBytes());
        return str;
    }

    std::string Buffer::retrieveAsString(size_t len)
    {
        assert(len <= readableBytes());
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    void Buffer::shrink(size_t reserve) {
        std::vector<char> buf(kCheapPrepend + readableBytes() + reserve);
        std::copy(peek(), peek() + readableBytes(), buf.begin() + kCheapPrepend);
        buf.swap(_buffer);
    }

    void Buffer::ensureWritable(size_t len) {
        if(writableBytes() < len) {
            makeSpace(len);
        }
        assert(writableBytes() >= len);
    }

    void Buffer::append(const std::string &str) {
        append(str.data(), str.size());
    }

    void Buffer::append(const char *str, size_t len) {
        assert(str);
        ensureWritable(len);
        std::copy(str, str+len, beginWrite());
        hasWritten(len);
    }

    void Buffer::append(const void *str, size_t len) {
        assert(str);
        append(static_cast<const char*>(str), len);
    }

    void Buffer::append(const Buffer &buf) {
        append(buf.peek(), buf.readableBytes());
    }

    void Buffer::makeSpace(size_t len) {

        size_t dataSize = readableBytes();

        if(writableBytes() + prependableBytes() < len + kCheapPrepend) {
            _buffer.resize(_writerIndex + len);
        }
        else {
            std::copy(Begin()+_readerIndex, Begin()+_writerIndex, Begin() + kCheapPrepend);
            _readerIndex = kCheapPrepend;
            _writerIndex = _readerIndex + dataSize;
        }
        assert(dataSize == readableBytes());
    }

    std::size_t Buffer::roundUp2Power(std::size_t size) {
        if (size == 0)
            return 0;

        std::size_t roundUp = 1;
        while (roundUp < size)
            roundUp *= 2;
        return roundUp;
    }

    ssize_t Buffer::readFd(int fd, int *saveErrno) {
        char buf[65536];
        struct iovec iov[2];
        const size_t writable = writableBytes();
        iov[0].iov_base = Begin() + _writerIndex;  //读到_buffer
        iov[0].iov_len = writable;
        iov[1].iov_base = buf;          //读到buf
        iov[1].iov_len = sizeof(buf);

        const int iovcnt = (writable < sizeof(buf)) ? 2 : 1;

        const ssize_t len = ::readv(fd, iov, iovcnt);
        if(len < 0) {
            *saveErrno = errno;
        }
        else if(implicit_cast<size_t>(len) <= writable) {
            _writerIndex += implicit_cast<size_t>(len);
        }
        else {
            _writerIndex = _buffer.size();
            append(buf, static_cast<size_t>(len) - writable);
        }
        return len;
    }

    ssize_t Buffer::writeFd(int fd, int *Errno) {
        ssize_t n = ::write(fd, peek(), readableBytes());
        if(n>0){
            retrieve(n);
        }
        return 0;
    }

    ssize_t Buffer::readFdET(int fd, int *savedErrno) {
        char extrabuf[65536];
        struct iovec vec[2];

        size_t writable = writableBytes();
        vec[0].iov_base = Begin() + _writerIndex;
        vec[0].iov_len = writable;

        vec[1].iov_base = extrabuf;
        vec[1].iov_len = sizeof extrabuf;

        ssize_t readLen = 0;
        // 不断调用read读取数据
        for (;;) {
            ssize_t n = readv(fd, vec, 2);
            if (n < 0) {
                if (errno == EAGAIN) {
                    *savedErrno = errno;
                    break;
                }
                return -1;
            }
            else if (n == 0) {
                // 没有读取到数据，认为对端已经关闭
                return 0;
            }
            else if (static_cast<size_t>(n) <= writable) {
                // 还没有写满缓冲区
                _writerIndex += n;
            }
            else {
                // 已经写满缓冲区, 则需要把剩余的buf写进去
                _writerIndex = _buffer.size();
                append(extrabuf, n - writable);
            }

            // 写完后需要更新 vec[0] 便于下一次读入
            writable = writableBytes();
            vec[0].iov_base = Begin() + _writerIndex;
            vec[0].iov_len = writable;
            readLen += n;
        }
        return readLen;
    }

    ssize_t Buffer::writeFdET(int fd, int *savedErrno) {
        ssize_t writesum = 0;

        for (;;) {
            ssize_t n = ::write(fd, peek(), readableBytes());
            if (n > 0) {
                writesum += n;
                retrieve(n); // 更新可读索引
                if (readableBytes() == 0) {
                    return writesum;
                }
            }
            else if (n < 0) {
                if (errno == EAGAIN) //系统缓冲区满，非阻塞返回
                {
                    break;
                }
                    // 暂未考虑其他错误
                else {
                    return -1;
                }
            }
            else {
                // 返回0的情况，查看write的man，可以发现，一般是不会返回0的
                return 0;
            }
        }
        return writesum;
    }

}