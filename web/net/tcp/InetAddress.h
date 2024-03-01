//
// Created by 37496 on 2024/2/10.
//

#ifndef WEBSERVER_INETADDRESS_H
#define WEBSERVER_INETADDRESS_H

#include <netinet/in.h>
#include <string>

namespace Tiny_muduo
{
    namespace net
    {
        class InetAddress
        {
        public:
            explicit InetAddress(uint16_t port);
            explicit InetAddress(sockaddr_in si) : _addr(si) {}
            explicit InetAddress(const char* ip, uint16_t port);
            ~InetAddress() = default;

            std::string getIp() const noexcept;
            uint16_t getPort() const noexcept;
            sa_family_t getFamily() const noexcept { return _addr.sin_family; }

            sockaddr_in getSockAddress() const noexcept { return _addr; }
            void setSockAddr(const sockaddr_in &addr) { _addr = addr; }
            std::string toString() const;
        private:
            sockaddr_in _addr;
        };
    }
}


#endif //WEBSERVER_INETADDRESS_H
