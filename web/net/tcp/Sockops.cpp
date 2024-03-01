//
// Created by 37496 on 2024/2/13.
//

#include <unistd.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "net/tcp/Sockops.h"
#include "log/Log.h"
#include "base/Type.h"

namespace Tiny_muduo::net::sockops
{
    const sockaddr* sockaddr_cast(const struct sockaddr_in* addr)
    {
        return static_cast<const sockaddr*>(implicit_cast<const void*>(addr));
    }

    sockaddr* sockaddr_cast(struct sockaddr_in* addr)
    {
        return static_cast<sockaddr*>(implicit_cast<void*>(addr));
    }
    int createBlockSocket() {
        int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
        if(sockfd < 0) {
            LOG_ERROR << "sockets::createBlocking";
        }
        return sockfd;
    }

    int createNonBlockSocket() {
        int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, IPPROTO_TCP);
        if(sockfd < 0) {
            LOG_ERROR << "sockets::createNonblocking";
        }
        return sockfd;
    }

    void setNonBlocking(int fd) {
        int old = ::fcntl(fd, F_GETFL, 0);
        old |= O_NONBLOCK;
        ::fcntl(fd, F_SETFL, old);
    }

    int connect(int sockfd, const struct sockaddr_in addr) {
        return ::connect(sockfd, sockaddr_cast(&addr), sizeof(addr));
    }

    void bind(int sockfd, const struct sockaddr_in addr) {
        int ret = ::bind(sockfd, sockaddr_cast(&addr), sizeof(addr));
        if(ret < 0)
        {
            LOG_FATAL << "sockets::bindOrDie";
        }
    }

    void listen(int sockfd)
    {
        int ret = ::listen(sockfd, SOMAXCONN);
        if(ret < 0)
        {
            LOG_FATAL << "sockets::listenOrDie";
        }
    }

    int  accept(int sockfd, struct sockaddr_in* addr) {
        socklen_t addrlen = sizeof(*addr);
        int connfd = ::accept4(sockfd, sockaddr_cast(addr), &addrlen, SOCK_NONBLOCK|SOCK_CLOEXEC);

        if(connfd < 0)
        {
            int savedErrno = errno;
            LOG_ERROR << "Socket::accept";
            switch(savedErrno)
            {
                case EAGAIN:
                case ECONNABORTED:
                case EINTR:
                case EPROTO: // ???
                case EPERM:
                case EMFILE: // per-process lmit of open file desctiptor ???
                    // expected errors
                    errno = savedErrno;
                    break;
                case EBADF:
                case EFAULT:
                case EINVAL:
                case ENFILE:
                case ENOBUFS:
                case ENOMEM:
                case ENOTSOCK:
                case EOPNOTSUPP:
                    // unexpected errors
                    LOG_FATAL << "unexpected error of ::accept " << savedErrno;
                    break;
                default:
                    LOG_FATAL << "unknown error of ::accept " << savedErrno;
                    break;
            }
        }
        return connfd;
    }

    void close(int sockfd)
    {
        if (::close(sockfd) < 0)
        {
            LOG_ERROR << "sockets::close";
        }
    }

    ssize_t read(int sockfd, void *buf, size_t count)
    {
        return ::read(sockfd, buf, count);
    }

    ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt)
    {
        return ::readv(sockfd, iov, iovcnt);
    }

    ssize_t write(int sockfd, const void *buf, size_t count)
    {
        return ::write(sockfd, buf, count);
    }

    void shutdownWrite(int sockfd)
    {
        if(::shutdown(sockfd, SHUT_WR) < 0)
        {
            LOG_ERROR << "sockets::shutdownWrite";
        }
    }

    void toIpPort(char* buf, size_t size, const struct sockaddr_in& addr)
    {
        char host[INET_ADDRSTRLEN] = "INVALID";
        toIp(host, sizeof(host), addr);
        uint16_t port = networkToHost16(addr.sin_port);
        snprintf(buf, size, "%s:%u", host, port);
    }

    void toIp(char* buf, size_t size, const struct sockaddr_in& addr)
    {
        assert(size >= INET_ADDRSTRLEN);
        ::inet_ntop(AF_INET, &addr.sin_addr, buf, static_cast<socklen_t>(size));
    }

    void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr)
    {
        addr->sin_family = AF_INET;
        addr->sin_port = hostToNetwork16(port);
        if(::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
        {
            LOG_ERROR << "sockets::fromIpPort";
        }
    }

    int getSocketError(int sockfd)
    {
        int optval;
        socklen_t optlen = static_cast<socklen_t>(sizeof optval);

        if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
        {
            return errno;
        }
        else
        {
            return optval;
        }
    }

    void setNoDelay(int fd) {
        int opt = 1;
        ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
    }


    struct sockaddr_in getPeerAddr(int sockfd)
    {
        struct sockaddr_in peeraddr;
        bzero(&peeraddr, sizeof(peeraddr));
        socklen_t addrlen = sizeof(peeraddr);
        if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0)
        {
            LOG_ERROR << "sockets::getPeerAddr";
        }
        return peeraddr;
    }

    struct sockaddr_in getLocalAddr(int sockfd)
    {
        struct sockaddr_in localaddr;
        bzero(&localaddr, sizeof(localaddr));
        socklen_t addrlen = sizeof(localaddr);
        if(::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0)
        {
            LOG_ERROR << "sockets::getLocalAddr";
        }
        return localaddr;
    }


    bool isSelfConnect(int sockfd)
    {
        struct sockaddr_in localaddr = getLocalAddr(sockfd);
        struct sockaddr_in peeraddr = getPeerAddr(sockfd);
        return localaddr.sin_port == peeraddr.sin_port && localaddr.sin_addr.s_addr == peeraddr.sin_addr.s_addr;
    }

    std::shared_ptr<SSL_CTX> createSslCtx(const std::string& certfile, const std::string& pkfile, const std::string& password )
    {
        std::shared_ptr<SSL_CTX > ctx = nullptr;
        ctx.reset(::SSL_CTX_new(TLS_server_method()), SSL_CTX_free);

        if(!ctx) {
            LOG_ERROR << "createSslCtx failed !!!";
            ::ERR_print_errors_fp(stderr);
            ::exit(-1);
        }
        ::SSL_CTX_set_options(ctx.get(),
                              SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
                              SSL_OP_NO_COMPRESSION |
                              SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION);

        ::SSL_CTX_set_default_passwd_cb_userdata(
                ctx.get(), password.empty() ? nullptr : static_cast<void*>(const_cast<char*>(password.c_str())));

        if (::SSL_CTX_use_certificate_file(ctx.get(), certfile.c_str(),
                                           SSL_FILETYPE_PEM) <= 0) {
            ::ERR_print_errors_fp(stderr);
            ::exit(-1);
        }
        if (::SSL_CTX_use_PrivateKey_file(ctx.get(), pkfile.c_str(), SSL_FILETYPE_PEM) <=
            0) {
            ::ERR_print_errors_fp(stderr);
            ::exit(-1);
        }
        if (::SSL_CTX_check_private_key(ctx.get()) <= 0) {
            ::ERR_print_errors_fp(stderr);
            ::exit(-1);
        }

        return ctx;
    }

}