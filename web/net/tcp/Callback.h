//
// Created by 37496 on 2024/2/10.
//

#ifndef WEBSERVER_CALLBACK_H
#define WEBSERVER_CALLBACK_H

#include <memory>
#include <functional>

namespace Tiny_muduo
{
    class TimeStamp;
    class Buffer;
    namespace net
    {
        class TcpConnection;

        using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

        using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
        using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
        using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
        using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr&, size_t)>;

        using MessageCallback = std::function<void(const TcpConnectionPtr&, Buffer*, TimeStamp)>;

        void defaultConnectionCallback(const TcpConnectionPtr& conn);
        void defaultMessageCallback(const TcpConnectionPtr& conn,
                                    Buffer* buffer,
                                    TimeStamp receiveTime);
    }
}
#endif //WEBSERVER_CALLBACK_H
