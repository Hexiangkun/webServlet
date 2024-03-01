//
// Created by 37496 on 2024/2/11.
//

#ifndef WEBSERVER_CONNECTOR_H
#define WEBSERVER_CONNECTOR_H

#include <memory>
#include <functional>
#include "base/Noncopyable.h"
#include "InetAddress.h"

namespace Tiny_muduo
{
    namespace net
    {
        class Channel;
        class EventLoop;

        // 主动发起连接，带有自动重连功能
        class Connector : public Noncopyable , public std::enable_shared_from_this<Connector>
        {
        public:
            using _ptr = std::shared_ptr<Connector>;
            typedef std::function<void (int sockfd)> NewConnectionCallback;

            Connector(EventLoop* loop, const InetAddress& serverAddr);
            ~Connector();

            void setNewConnectionCallback(const NewConnectionCallback& cb) { _newConnectionCallback = cb; }
            const InetAddress& serverAddress() const { return _serverAddr; }

            void start();  // can be called in any thread
            void restart();  // must be called in loop thread
            void stop();  // can be called in any thread

        private:
            enum States { kDisconnected, kConnecting, kConnected };
            static const int kMaxRetryDelayMs = 30*1000;    // 30秒，最大重连延迟时间
            static const int kInitRetryDelayMs = 500;       // 0.5秒，初始状态，连接不上，0.5秒后重连

            void setState(States s) { _state = s; }
            void startInLoop();
            void stopInLoop();
            void connect();
            void connecting(int sockfd);
            void handleWrite();
            void handleError();
            void retry(int sockfd);
            int removeAndResetChannel();
            void resetChannel();

            EventLoop* _loop;
            InetAddress _serverAddr;        //服务器端地址
            bool _connect; // atomic
            States _state;  // FIXME: use atomic variable
            std::unique_ptr<Channel> _channel;
            NewConnectionCallback _newConnectionCallback;
            int _retryDelayMs;              // 重连延迟时间（单位：毫秒）
        };
    }
}


#endif //WEBSERVER_CONNECTOR_H
