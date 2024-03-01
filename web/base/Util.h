//
// Created by 37496 on 2024/1/31.
//

#ifndef WEBSERVER_UTIL_H
#define WEBSERVER_UTIL_H

#include <sys/syscall.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <string>

namespace Tiny_muduo
{
    thread_local static const pid_t PID = gettid();

    pid_t getThreadId();

    void ErrorIf(bool condition, const char *errmsg) ;

    unsigned char hex2dec(char);        //十六进制数转十进制数
    char dec2hex(char);                 //十进制数转十六进制数

    int hexString2Int(const std::string& hexStr);       //十六进制字符串转int,失败返回-1
    int hexString2Int(const std::string_view hexStr);

    std::string int2HexString(int dec);                 //int转十六进制字符串

    void toLowers(char*, size_t);
    void toUppers(char*, size_t);
    std::string toLowers(const std::string&);
    std::string toUppers(const std::string&);

    bool stringCaseCmp(const std::string&, const std::string&);
}

#endif //WEBSERVER_UTIL_H
