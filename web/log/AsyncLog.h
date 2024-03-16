//
// Created by 37496 on 2024/1/30.
//

#ifndef WEBSERVER_ASYNCLOG_H
#define WEBSERVER_ASYNCLOG_H

#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "LogStream.h"


namespace HLog
{
    class AsyncLog : public Noncopyable
    {
        typedef LogBuffer<kLargeSize> Buffer;
        typedef std::vector<std::unique_ptr<Buffer >> BufferVector;
        typedef  BufferVector::value_type BufferPtr;

    public:
        AsyncLog(const std::string& filepath, const std::string& basename = "basename", int flush_interval=3, off_t roll_size = 20*1024*1024);
        ~AsyncLog();

        void append(const char* buf, int len);

        void stop();

    private:
        void writeThread();

    private:
        const int _flush_interval;
        const off_t _roll_size;
        std::string _filepath;
        std::string _basename;
        std::atomic<bool> _running;
        std::thread _thread;
        std::mutex _mutex;
        std::condition_variable _cond;
        BufferPtr _cur;
        BufferPtr _next;
        BufferVector _buffers;
    };
}



#endif //WEBSERVER_ASYNCLOG_H
