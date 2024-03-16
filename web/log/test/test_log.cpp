#include <unistd.h>
#include <sys/time.h>
#include <iostream>
#include "../Log.h"
#include "../LogFile.h"


int64_t get_current_millis(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}


int g_total;
FILE* g_file;
std::unique_ptr<HLog::LogFile> g_logFile;

void dummyOutput(const HLog::LogStream::Buffer& buf)
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

    //log::Logger::setOutputFunc(dummyOutput);
    SET_LOG_ASYNC(1);
    auto batch = 1e6;
    g_total = 0;

    const bool kLongLog = false;
    std::string empty = " ";
    std::string longStr(3000, 'X');
    longStr += " ";
    int n = 1000*1000;
    HLog::TimeStamp start(HLog::TimeStamp::now());
    std::cout << start.toFormatString() << std::endl;
    for (int i = 0;i < n; ++i)
    {
        LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz"
                 << (kLongLog ? longStr : empty)
                 << i;
    }
    HLog::TimeStamp end(HLog::TimeStamp::now());
    std::cout << end.toFormatString() << std::endl;
    double seconds = timeDifference(end, start);
    printf("%0.6f seconds, %d bytes, %10.2f msg/s, %.2f MiB/s\n",
           seconds, g_total, n / seconds, g_total / seconds / (1024 * 1024));

}

//void test_log()
//{
//    g_file = NULL;
//    g_logFile.reset(new log::LogFile("test_log_st", 500*1000*1000));
//    test_log_func();
//}

void full_test()
{
    HLog::Logger::setLogLevel(HLog::LogLevel::TRACE);
    LOG_TRACE << "trace";
    LOG_DEBUG << "debug";
    LOG_INFO << "info";
    LOG_WARN << "warn";
    LOG_ERROR << "error";
    LOG_FATAL << "fatal";
}

int main()
{
    test_log_func();
}
