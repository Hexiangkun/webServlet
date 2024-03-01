//
// Created by 37496 on 2024/2/10.
//

#include <netinet/tcp.h>
#include "net/tcp/Sockops.h"
#include "net/tcp/Socket.h"

namespace Tiny_muduo::net
{


    Socket::Socket(int fd) : _sockfd(fd) {

    }

    Socket::~Socket() {
        sockops::close(_sockfd);
    }

    void Socket::bind(const InetAddress &address) {
        sockops::bind(_sockfd, address.getSockAddress());
    }

    void Socket::listen() {
        sockops::listen(_sockfd);
    }

    int Socket::accept(InetAddress *peeraddr) {
        int clt_sockFd = -1;
        struct sockaddr_in addr {};
        ::bzero(&addr, sizeof(addr));

        clt_sockFd = sockops::accept(_sockfd, &addr);

        if(clt_sockFd >= 0) {
            peeraddr->setSockAddr(addr);
        }
        return clt_sockFd;
    }

    int Socket::accept() {
        int clt_sockFd = -1;
        struct sockaddr_in addr {};
        ::bzero(&addr, sizeof(addr));

        clt_sockFd = sockops::accept(_sockfd, &addr);
        return clt_sockFd;
    }

    void Socket::connect(const InetAddress &address) {
        struct sockaddr_in tmp = address.getSockAddress();
        sockops::connect(_sockfd, tmp);
    }

    void Socket::connect(const char *ip, uint16_t port) {
        InetAddress address(ip, port);
        connect(address);
    }


    InetAddress Socket::getLocalAddress()  {
        return InetAddress(sockops::getLocalAddr(_sockfd));
    }

    InetAddress Socket::getRemoteAddress() {
        return InetAddress(sockops::getPeerAddr(_sockfd));
    }

    void Socket::setNonBlocking() {
        sockops::setNonBlocking(_sockfd);
    }

    void Socket::shutdownWrite() {
        sockops::shutdownWrite(_sockfd);
    }

    void Socket::setTcpNoDelay(bool on) {
        int optval = on ? 1 : 0;
        // TCP_NODELAY包含头文件 <netinet/tcp.h>
        ::setsockopt(_sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
    }

    void Socket::setReuseAddr(bool on) {
        int optval = on ? 1 : 0;
        ::setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    }

    void Socket::setReusePort(bool on) {
        int optval = on ? 1 : 0;
        ::setsockopt(_sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    }

    void Socket::setKeepAlive(bool on) {
        int optval = on ? 1 : 0;
        ::setsockopt(_sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
    }

    void Socket::ignoreSIGPIPE(bool on) {
        int optval = on ? 1 : 0;
        ::setsockopt(_sockfd, SOL_SOCKET, MSG_NOSIGNAL, &optval, sizeof(optval));
    }

}