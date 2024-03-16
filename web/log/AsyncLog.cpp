//
// Created by 37496 on 2024/1/30.
//

#include <functional>
#include <cassert>
#include "AsyncLog.h"
#include "LogFile.h"
#include "TimeStamp.h"


namespace HLog
{
    AsyncLog::AsyncLog(const std::string& filepath, const std::string& basename, int flush_interval, off_t roll_size)
            : _filepath(filepath),
            _flush_interval(flush_interval),
            _roll_size(roll_size),
            _running(true),
            _thread(std::bind(&AsyncLog::writeThread, this), "AsyncLogThread"),
            _basename(basename),
            _cur(new Buffer),
            _next(new Buffer),
            _buffers()
    {
        _cur->clear();
        _next->clear();
        _buffers.reserve(16);
    }

    AsyncLog::~AsyncLog() {
        if(_running){
            stop();
        }
    }

    void AsyncLog::stop() {
        _running = false;
        _thread.join();     //主线程阻塞，等待其他线程终止
    }

    void AsyncLog::append(const char *buf, int len) {
        std::unique_lock<std::mutex> lock(_mutex);
        if(_cur->available() > len) {
            _cur->append(buf, len);
        }
        else {
            _buffers.emplace_back(std::move(_cur));
            if(_next) {
                _cur = std::move(_next);
            }
            else {
                _cur.reset(new Buffer);
//                _next.reset(new Buffer);
            }
            _cur->append(buf, len);
            _cond.notify_one();
        }
    }

    void AsyncLog::writeThread() {
        BufferPtr buf1(new Buffer);
        BufferPtr buf2(new Buffer);
        buf1->clear();
        buf2->clear();

        BufferVector buf_to_write;
        buf_to_write.reserve(16);

        LogFile output(_filepath, _basename, _roll_size);

        while (_running) {
            {
                std::unique_lock<std::mutex> lock(_mutex);
                if(_buffers.empty()) {
                    _cond.wait_for(lock, std::chrono::milliseconds(_flush_interval));
                }

                _buffers.emplace_back(std::move(_cur));
                _cur = std::move(buf1);
                buf_to_write.swap(_buffers);

                if (!_next) {
                    _next = std::move(buf2);
                }
            }

            assert(!buf_to_write.empty());

            if (buf_to_write.size() > 25) {
                char buf[256];
                snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
                         TimeStamp::now().toFormatString().c_str(),
                         buf_to_write.size()-2);
                fputs(buf, stderr);
                output.append(buf, static_cast<int>(strlen(buf)));
                buf_to_write.erase(buf_to_write.begin()+2, buf_to_write.end());
            }

            for (const auto &buf: buf_to_write) {
                output.append(buf->data(), (size_t) buf->length());
            }

            if (buf_to_write.size() > 2) {
                buf_to_write.resize(2);
            }

            if (!buf1) {
                assert(!buf_to_write.empty());
                buf1 = std::move(buf_to_write.back());
                buf_to_write.pop_back();
                buf1->reset();
            }
            if (!buf2) {
                assert(!buf_to_write.empty());
                buf2 = std::move(buf_to_write.back());
                buf_to_write.pop_back();
                buf2->reset();
            }

            buf_to_write.clear();
//            BufferVector().swap(buf_to_write);
//            buf_to_write.reserve(8);
            output.flush();
        }
        output.flush();
    }
}

