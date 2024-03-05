//
// Created by 37496 on 2024/3/4.
//

#include "RingBuffer.h"
#include <errno.h>
#include <memory.h>
#include <sys/uio.h>
#include <unistd.h>

namespace Tiny_muduo
{
    const char RingBuffer::kCRLF[] = "\r\n"; // 回车换行

    RingBuffer::RingBuffer()
            :_writerIndex(kCheapPrepend),
            _readerIndex(kCheapPrepend),
            _capacity(kCheapPrepend+kInitialSize),
            _buffer(new char[_capacity])
    {
        assert(readableBytes() == 0);
        assert(writableBytes() == kInitialSize);
    }

    RingBuffer::~RingBuffer() {
        delete _buffer;
        _buffer = nullptr;
        _capacity = 0;
    }

    size_t RingBuffer::readableBytes() const {
        if(_writerIndex>=_readerIndex) {
            return _writerIndex - _readerIndex;
        }
    }

    size_t RingBuffer::writableBytes() const {
        if(_writerIndex >= _readerIndex) {
            return _capacity - _writerIndex + _readerIndex -kCheapPrepend;
        }
        return _readerIndex - _writerIndex - kCheapPrepend;
    }

    ssize_t RingBuffer::readFd(int fd, int *savedErrno) {
        char extrabuf[65536]; // 64*1024
        struct iovec vec[3];
        const size_t writable = writableBytes();

        vec[0].iov_base = begin() + _writerIndex;

        if (_writerIndex < _readerIndex) {
            vec[0].iov_len = writable;
            // 如果可写数据只在头端，则不使用第二块缓冲区
            vec[1].iov_base = begin();
            vec[1].iov_len = 0;
        } else // 如果可写数据横跨缓冲区尾端和头端
        {
            vec[0].iov_len = _capacity - _writerIndex;
            vec[1].iov_base = begin();
            vec[1].iov_len = writable - vec[0].iov_len;
        }
        vec[2].iov_base = extrabuf;
        vec[2].iov_len = sizeof extrabuf;
        // readv()代表分散读， 即将数据从文件描述符读到分散的内存块中
        const ssize_t n = readv(fd, vec, 3);
        if (n < 0) {
            *savedErrno = errno;
        } else if (static_cast<size_t>(n) <= writable) {
            _writerIndex += n;
            _writerIndex %= _capacity;
        } else {
            _writerIndex += writable;
            _writerIndex %= _capacity;
            append(extrabuf, n - writable);
        }
        return n;
    }


// 支持ET模式下缓冲区的数据读取
    ssize_t RingBuffer::readFdET(int fd, int *savedErrno) {
        char extrabuf[65536];
        struct iovec vec[3];
        vec[2].iov_base = extrabuf;
        vec[2].iov_len = sizeof extrabuf;
        size_t writable;
        ssize_t readLen = 0;
        // 不断调用read读取数据
        for (;;) {
            // 写完后需要更新 vec[0] 便于下一次读入
            writable = writableBytes();
            if (_writerIndex < _readerIndex) {
                vec[0].iov_base = begin() + _writerIndex;
                vec[0].iov_len = writable;
                // 不使用第二块缓冲区
                vec[1].iov_base = begin();
                vec[1].iov_len = 0;
            } else {
                vec[0].iov_base = begin() + _writerIndex;
                vec[0].iov_len = _capacity - _writerIndex;
                vec[1].iov_base = begin();
                vec[1].iov_len = writable - vec[0].iov_len;
            }

            ssize_t n = readv(fd, vec, 3);
            if (n < 0) {
                if (errno == EAGAIN) {
                    *savedErrno = errno;
                    break;
                }
                return -1;
            } else if (n == 0) {
                // 没有读取到数据，认为对端已经关闭
                return 0;
            } else if (static_cast<size_t>(n) <= writable) {
                // 还没有写满缓冲区
                _writerIndex += n;
                _writerIndex %= _capacity;
            } else {
                // 已经写满缓冲区, 则需要把剩余的buf写进去
                _writerIndex += writable;
                _writerIndex %= _capacity;
                append(extrabuf, n - writable);
            }

            readLen += n;
        }
        return readLen;
    }


    ssize_t RingBuffer::writeFd(int fd, int *savedErrno) {
        // 从可读位置开始读取
        struct iovec vec[2];
        if (_writerIndex < _readerIndex) {
            vec[0].iov_base = _buffer + _readerIndex;
            vec[0].iov_len = _capacity - _readerIndex;
            vec[1].iov_base = _buffer;
            vec[1].iov_len = _writerIndex;
        } else {
            vec[0].iov_base = _buffer + _readerIndex;
            // vec[0].iov_len = readableBytes();
            vec[0].iov_len = (_writerIndex - _readerIndex);
            vec[1].iov_base = _buffer;
            vec[1].iov_len = 0;
        }
        ssize_t n = ::writev(fd, vec, 2);
        if (n > 0) {
            retrieve(n);
        }
        return 0;
    }


// ET 模式下处理写事件
ssize_t RingBuffer::writeFdET(int fd, int *savedErrno) {
  ssize_t writesum = 0;
  // 从可读位置开始读取
  struct iovec vec[2];
  for (;;) {
    if (_writerIndex < _readerIndex) {
      vec[0].iov_base = _buffer + _readerIndex;
      vec[0].iov_len = _capacity - _readerIndex;
      vec[1].iov_base = _buffer;
      vec[1].iov_len = _writerIndex;
    } else {
      vec[0].iov_base = _buffer + _readerIndex;
      // vec[0].iov_len = readableBytes();
      vec[0].iov_len = (_writerIndex - _readerIndex);
      vec[1].iov_base = _buffer;
      vec[1].iov_len = 0;
    }
    ssize_t n = ::writev(fd, vec, 2);
    if (n > 0) {
      writesum += n;
      retrieve(n); // 更新可读索引
      if (readableBytes() == 0) {
        return writesum;
      }
    } else if (n < 0) {
      if (errno == EAGAIN) //系统缓冲区满，非阻塞返回
      {
        break;
      }
      // 暂未考虑其他错误
      else {
        return -1;
      }
    } else {
      // 返回0的情况，查看write的man，可以发现，一般是不会返回0的
      return 0;
    }
  }
  return writesum;
}


}