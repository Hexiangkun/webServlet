//
// Created by 37496 on 2024/2/13.
//

#ifndef WEBSERVER_SOCKOPS_H
#define WEBSERVER_SOCKOPS_H

#include <sys/uio.h>
#include <memory>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "net/tcp/Endian.h"
#include "net/tcp/InetAddress.h"

namespace Tiny_muduo
{
    namespace net
    {
        namespace sockops
        {
            int createNonBlockSocket();
            int createBlockSocket();

            int connect(int sockfd, const struct sockaddr_in addr);
            void bind(int sockfd, const struct sockaddr_in addr);
            void listen(int sockfd);
            int accept(int sockfd, struct sockaddr_in* addr);
            void close(int sockfd);

            ssize_t read(int sockfd, void *buf, size_t count);
            ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt);
            ssize_t write(int sockfd, const void *buf, size_t count);

            void shutdownWrite(int sockfd);

            void toIpPort(char* buf, size_t size, const struct sockaddr_in& addr);
            void toIp(char* buf, size_t size, const struct sockaddr_in& addr);
            void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);

            void setNonBlocking(int fd);

            void setNoDelay(int fd);  //禁用Nagle算法

            struct sockaddr_in getPeerAddr(int fd);    //获取socket对端地址
            struct sockaddr_in getLocalAddr(int fd);    //获取socket本端地址

            int getSocketError(int sockfd);
            bool isSelfConnect(int sockfd);

            std::shared_ptr<SSL_CTX> createSslCtx(const std::string& certfile, const std::string& pkfile, const std::string& password );
        }
    }
}


#endif //WEBSERVER_SOCKOPS_H
