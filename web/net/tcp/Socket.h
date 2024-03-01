//
// Created by 37496 on 2024/2/10.
//

#ifndef WEBSERVER_SOCKET_H
#define WEBSERVER_SOCKET_H

#include <memory>
#include <sys/uio.h>
#include "net/tcp/InetAddress.h"
#include "net/tcp/Sockops.h"
#include "web/base/Buffer.h"

namespace Tiny_muduo
{
    namespace net
    {
        class Socket
        {
        public:
            using _ptr = std::shared_ptr<Socket>;

            explicit Socket(int fd);
            ~Socket();

            void bind(const InetAddress& localAddress);
            void listen();
            int accept(InetAddress *peeraddr);  // 默认返回读写非阻塞的socket
            virtual int accept();

            void connect(const InetAddress& address);
            void connect(const char* ip, uint16_t port);

            int sockfd() const noexcept { return _sockfd; }
            void setNonBlocking();

            InetAddress getLocalAddress() ;  //获取本地地址
            InetAddress getRemoteAddress(); //获取远端地址

            void ignoreSIGPIPE(bool on);    //忽略SIGPIPE信号

            void shutdownWrite();            // 设置半关闭

            void setTcpNoDelay(bool on);    // 设置Nagel算法
            void setReuseAddr(bool on);     // 设置地址复用
            void setReusePort(bool on);     // 设置端口复用
            void setKeepAlive(bool on);     // 设置长连接

        protected:
            const int _sockfd;
        };

    }
}


#endif //WEBSERVER_SOCKET_H
