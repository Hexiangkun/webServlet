//
// Created by 37496 on 2024/2/10.
//

#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include "net/tcp/InetAddress.h"

namespace Tiny_muduo::net
{
    InetAddress::InetAddress(uint16_t port) {
        ::memset(&_addr, 0, sizeof(_addr));
        _addr.sin_port = ::htons(port);
        _addr.sin_family = AF_INET;
        _addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }


    InetAddress::InetAddress(const char *ip, uint16_t port) {
        ::memset(&_addr, 0, sizeof(_addr));
        _addr.sin_port = htons(port);
        _addr.sin_family = AF_INET;
        _addr.sin_addr.s_addr = ::inet_addr(ip);
    }

    std::string InetAddress::getIp() const noexcept {
        char buf[64];
        ::inet_ntop(AF_INET, &_addr.sin_addr, buf, sizeof(buf));
        return buf;
    }

    uint16_t InetAddress::getPort() const noexcept {
        return ::ntohs(_addr.sin_port);
    }

    std::string InetAddress::toString() const {
        return getIp() + std::string(":") + std::to_string(getPort());
    }
}