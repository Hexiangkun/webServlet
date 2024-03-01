//
// Created by 37496 on 2024/2/5.
//

#include <algorithm>
#include "Util.h"

namespace Tiny_muduo
{
    pid_t getThreadId() {
        return PID;
    }

    void ErrorIf(bool condition, const char *errmsg) {
        if (condition) {
            perror(errmsg);
            exit(EXIT_FAILURE);
        }
    }

    unsigned char hex2dec(char ch) {       //十六进制数转十进制数
        if(ch >= '0' && ch <= '9') {
            return (static_cast<unsigned char>(ch - '0'));
        }
        else if(ch >= 'a' && ch <= 'z') {
            return (static_cast<unsigned char>(ch - 'a' + 10));     //a-f[10-15]
        }
        else if(ch >= 'A' && ch <= 'Z') {
            return (static_cast<unsigned char>(ch - 'A' + 10));     //A-F[10-15]
        }
        return static_cast<unsigned char>(ch);
    }

    char dec2hex(char ch) {         //将ASCII中前0-15个数据转换为十六进制
        if(ch >= 0 && ch <= 9) {
            return (ch + '0');
        }
        else if(ch >= 10 && ch <= 15) {
            return ch - 10 + 'A';
        }
        return ch;
    }


    int hexString2Int(const std::string& hexStr) {
        int res = 0;
        for(const auto& c:hexStr) {
            if(!isdigit(c)) {
                return -1;
            }
            res = res * 10 + hex2dec(c);
            if(res < 0) {
                return -1;
            }
        }
        return res;
    }

    int hexString2Int(const std::string_view hexStr) {
        int res = 0;
        for(const auto& c:hexStr) {
            if(!isdigit(c)) {
                return -1;
            }
            res = res * 10 + hex2dec(c);
            if(res < 0) {
                return -1;
            }
        }
        return res;
    }

    std::string int2HexString(int dec) {
        std::string hex;
        while(dec) {
            hex += dec2hex(dec % 10);
            dec /= 10;
        }
        std::reverse(hex.begin(), hex.end());
        return hex;
    }

    void toLowers(char* s, size_t size)
    {
        for(size_t i = 0; i < size; i++) {
            if(::isupper(s[i])) {
                s[i] = static_cast<char>(::tolower(s[i]));
            }
        }
    }

    void toUppers(char* s, size_t size) {
        for(size_t i = 0; i < size; i++) {
            if(::islower(s[i])) {
                s[i] = static_cast<char>(::toupper(s[i]));
            }
        }
    }

    std::string toLowers(const std::string& str) {
        std::string ls(str);
        toLowers(ls.data(), ls.size());
        return ls;
    }

    std::string toUppers(const std::string& str) {
        std::string ls(str);
        toUppers(ls.data(), ls.size());
        return ls;
    }

    bool stringCaseCmp(const std::string& a, const std::string& b) {
        return toLowers(a) == toLowers(b);
    }
}