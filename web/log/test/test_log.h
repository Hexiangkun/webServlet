//
// Created by 37496 on 2024/2/2.
//

#ifndef WEBSERVER_TEST_LOG_H
#define WEBSERVER_TEST_LOG_H

#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include "log/Log.h"
#include "log/LogFile.h"
#include "base/TimeStamp.h"

namespace test
{

    int64_t get_current_millis(void) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
    }


    int g_total;
    FILE* g_file;
    std::unique_ptr<Tiny_muduo::log::LogFile> g_logFile;

    void dummyOutput(const Tiny_muduo::log::LogStream::Buffer& buf)
    {
        g_total += buf.length();
        if (g_file)
        {
            fwrite(buf.data(), 1, buf.length(), g_file);
        }
        else if (g_logFile)
        {
            g_logFile->append(buf.data(), buf.length());
        }
    }


    void test_log_func()
    {
        using namespace Tiny_muduo::log;
        Tiny_muduo::log::Logger::setOutputFunc(dummyOutput);
        auto batch = 1e6;
        g_total = 0;

        const bool kLongLog = false;
        std::string empty = " ";
        std::string longStr(3000, 'X');
        longStr += " ";
        int n = 1000*1000;
        Tiny_muduo::TimeStamp start(Tiny_muduo::TimeStamp::now());
        for (int i = 0;i < n; ++i)
        {
            LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz"
                     << (kLongLog ? longStr : empty)
                     << i;
        }
        Tiny_muduo::TimeStamp end(Tiny_muduo::TimeStamp::now());

        double seconds = timeDifference(end, start);
        printf("%f seconds, %d bytes, %10.2f msg/s, %.2f MiB/s\n",
               seconds, g_total, n / seconds, g_total / seconds / (1024 * 1024));

    }

    void test_log()
    {
        g_file = NULL;
        g_logFile.reset(new Tiny_muduo::log::LogFile("test_log_st", 500*1000*1000));
        test_log_func();
    }

    void full_test()
    {
        Tiny_muduo::log::Logger::setLogLevel(Tiny_muduo::log::LogLevel::TRACE);
        LOG_TRACE << "trace";
        LOG_DEBUG << "debug";
        LOG_INFO << "info";
        LOG_WARN << "warn";
        LOG_ERROR << "error";
        LOG_FATAL << "fatal";
    }

}
#endif //WEBSERVER_TEST_LOG_H
