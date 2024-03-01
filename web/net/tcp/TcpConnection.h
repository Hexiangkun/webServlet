//
// Created by 37496 on 2024/2/10.
//

#ifndef WEBSERVER_TCPCONNECTION_H
#define WEBSERVER_TCPCONNECTION_H


#include <string>
#include <atomic>
#include <memory>
#include <any>
#include "net/tcp/InetAddress.h"
#include "net/tcp/Callback.h"
#include "web/base/Buffer.h"
#include "base/TimeStamp.h"
#include "base/Noncopyable.h"

namespace Tiny_muduo
{
    namespace net
    {
        class EventLoop;
        class Socket;
        class Channel;

        /**
         * TcpServer => Acceptor => 有一个新用户连接，通过accept函数拿到connfd
         * => TcpConnection设置回调 => 设置到Channel => Poller => Channel回调
         **/
    class TcpConnection : public Noncopyable, public std::enable_shared_from_this<TcpConnection>
        {
        public:
            using _ptr = std::shared_ptr<TcpConnection>;

            TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
                          const InetAddress& localAddr, const InetAddress& peerAddr);

            ~TcpConnection();

            void send(const std::string& buf);
            void send(Buffer* buf);

            void shutdown();                //关闭连接
            void forceClose();
            void startRead();
            void stopRead();

            void setTcpNoDelay(bool on);

            //TcpServer调用
            void connectEstablished();      //建立连接
            void connectDestroyed();        //销毁连接

            EventLoop* getLoop() const { return _loop; }
            const std::string& name() const { return _name; }
            const InetAddress& localAddress() const { return _localAddr; }
            const InetAddress& peerAddress() const { return _peerAddr; }

            bool isReading() const { return _reading; };

            bool connected() const { return _state == kConnected; }
            bool disconnected() const { return _state == kDisconnected; }


            void setConnectionCallback(const ConnectionCallback& cb) { _connectionCallback = cb; }
            void setMessageCallback(const MessageCallback& cb) { _messageCallback = cb; }
            void setWriteCompleteCallback(const WriteCompleteCallback& cb) { _writeCompleteCallback = cb; }
            void setCloseCallback(const CloseCallback& cb) { _closeCallback = cb; }
            void setHighWaterMarkCallback(const HighWaterMarkCallback& cb) { _highWaterMarkCallback = cb; }

            Buffer::_ptr getReadBuffer() const { return _inputBuffer; }
            Buffer::_ptr getWriteBuffer() const { return _outputBuffer; }

            void setContext(std::shared_ptr<void> context) { shared_context = context; }
            std::shared_ptr<void> getContext() const { return shared_context; }
            void clearContext() { shared_context = nullptr; }

            void setAnyContext(const std::any& context) { _context = context; }
            const std::any& getAnyContext() const { return _context;}
            std::any* getMutableAnyContext() { return &_context; }

            TimeStamp last_message() const { return _lastMessage; }
            void set_last_message(TimeStamp now) { _lastMessage = now; }

        private:
            enum StateE
            {
                kDisconnected,  //已经断开连接
                kConnecting,    //正在连接
                kConnected,     //已连接
                kDisconnecting  //正在断开连接
            };
            void setState(StateE stateE) { _state = stateE; }

            void handleRead(TimeStamp receiveTime);
            void handleWrite();
            void handleClose();
            void handleError();

            void sendInLoop(const void* message, size_t len);
            void sendInLoop(const std::string& message);
            void shutdownInLoop();
            void forceCloseInLoop();
            void startReadInLoop();
            void stopReadInLoop();

            const char* stateToString() const;

    private:
        // 这里是baseloop还是subloop由TcpServer中创建的线程数决定 若为多Reactor 该loop_指向subloop 若为单Reactor 该loop_指向baseloop
            EventLoop* _loop;               //属于那个subLoop
            const std::string _name;
            std::atomic_int _state;         //连接状态
            bool _reading;
            TimeStamp _lastMessage;
            std::unique_ptr<Socket> _socket;            //fd的生命周期由Socket负责
            std::unique_ptr<Channel> _channel;          //fd的事件由Channel负责

            const InetAddress _localAddr;
            const InetAddress _peerAddr;

            ConnectionCallback _connectionCallback;          //连接建立/关闭后的回调函数
            MessageCallback _messageCallback;                //收到消息后的回调
            WriteCompleteCallback _writeCompleteCallback;    //消息发送完毕后的回调
            CloseCallback  _closeCallback;                   //客户端关闭连接的回调
            HighWaterMarkCallback _highWaterMarkCallback;    //超出水位实现的回调
            size_t _highWaterMark;

            Buffer::_ptr _inputBuffer;
            Buffer::_ptr _outputBuffer;
            std::shared_ptr<void> shared_context;
            std::any _context;

        };
    }
}


#endif //WEBSERVER_TCPCONNECTION_H
