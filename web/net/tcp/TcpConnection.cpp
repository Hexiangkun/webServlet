//
// Created by 37496 on 2024/2/10.
//

#include <functional>
#include "net/tcp/TcpConnection.h"
#include "net/tcp/Socket.h"
#include "net/tcp/Channel.h"
#include "net/tcp/EventLoop.h"
#include "log/Log.h"

void Tiny_muduo::net::defaultConnectionCallback(const Tiny_muduo::net::TcpConnectionPtr& conn)
{
#ifdef DEBUG
    LOG_TRACE << conn->localAddress().toString() << " -> "
              << conn->peerAddress().toString() << " is "
              << (conn->connected() ? "UP" : "DOWN");
#endif
    // do not call conn->forceClose(), because some users want to register message callback only.
}

void Tiny_muduo::net::defaultMessageCallback(const TcpConnectionPtr&,
                            Buffer* buf,
                            TimeStamp)
{
    buf->retrieveAll();
}

namespace Tiny_muduo::net
{

    static EventLoop *CheckLoopNotNull(EventLoop *loop)
    {
        // 如果传入EventLoop没有指向有意义的地址则出错
        // 正常来说在 TcpServer::start 这里就生成了新线程和对应的EventLoop
        if (loop == nullptr)
        {
            LOG_FATAL << "mainLoop is null!";
        }
        return loop;
    }

    TcpConnection::TcpConnection(EventLoop *loop, const std::string &name, int sockfd,
                                 const Tiny_muduo::net::InetAddress &localAddr,
                                 const Tiny_muduo::net::InetAddress &peerAddr)
                                 :_loop(CheckLoopNotNull(loop)),
                                 _name(name),
                                 _state(kConnecting),
                                 _reading(true),
                                 _socket(std::make_unique<Socket>(sockfd)),
                                 _channel(std::make_unique<Channel>(loop, sockfd)),
                                 _inputBuffer(std::make_shared<Buffer>()),
                                 _outputBuffer(std::make_shared<Buffer>()),
                                 _localAddr(localAddr), _peerAddr(peerAddr),
                                 _highWaterMark(64 * 1024 * 1024) // 64M 避免发送太快对方接受太慢
    {
        _channel->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
        _channel->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
        _channel->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
        _channel->setErrorCallback(std::bind(&TcpConnection::handleError, this));
#ifdef DEBUG
        LOG_INFO << "TcpConnection::ctor[" << _name.c_str() << "] at fd =" << sockfd;
#endif
        _socket->setKeepAlive(true);

    }

    TcpConnection::~TcpConnection()
    {
#ifdef DEBUG
        LOG_INFO << "TcpConnection::dtor[" << _name.c_str() << "] at fd=" << _channel->fd() << " state=" << stateToString();
#endif
        assert(_state == kDisconnected);
    }

    void TcpConnection::send(const std::string &buf) {
        if(_state == kConnected) {
            if(_loop->isInLoopThread()) {
                sendInLoop(buf.c_str(), buf.size());
            }
            else {
                // 遇到重载函数的绑定，可以使用函数指针来指定确切的函数
                void(TcpConnection::*fp)(const void* data, size_t len) = &TcpConnection::sendInLoop;
                _loop->runInLoop(std::bind(fp, this, buf.c_str(), buf.size()));
            }
        }
    }

    void TcpConnection::send(Buffer *buf) {
        if(_state == kConnected) {
            if(_loop->isInLoopThread()) {
                sendInLoop(buf->peek(), buf->readableBytes());
                buf->retrieveAll();
            }
            else {
                void(TcpConnection::*fp)(const std::string& message) = &TcpConnection::send;
                _loop->runInLoop(std::bind(fp, this, buf->retrieveAllToStr()));
            }
        }
    }


    void TcpConnection::shutdown() {
        if(_state == kConnected) {
            setState(kDisconnecting);
            _loop->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
        }
    }

    void TcpConnection::setTcpNoDelay(bool on)
    {
        _socket->setTcpNoDelay(on);
    }

    void TcpConnection::startRead()
    {
        _loop->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
    }

    void TcpConnection::startReadInLoop()
    {
        _loop->assertInLoopThread();
        if (!_reading || !_channel->isReading())
        {
            _channel->enableReading();
            _reading = true;
        }
    }

    void TcpConnection::stopRead()
    {
        _loop->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
    }

    void TcpConnection::stopReadInLoop()
    {
        _loop->assertInLoopThread();
        if (_reading || _channel->isReading())
        {
            _channel->disableReading();
            _reading = false;
        }
    }

    //在TcpServer::newConnection中调用，主要时注册EPOLLIN事件，顺便执行用户自定义的回调函数
    void TcpConnection::connectEstablished() {
        _loop->assertInLoopThread();
        assert(_state == kConnecting);
        setState(kConnected);
        /**
         * TODO:tie
         * channel_->tie(shared_from_this());
         * tie相当于在底层有一个强引用指针记录着，防止析构
         * 为了防止TcpConnection这个资源被误删掉，而这个时候还有许多事件要处理
         * channel->tie 会进行一次判断，是否将弱引用指针变成强引用，变成得话就防止了计数为0而被析构得可能
         */
        _channel->tie(shared_from_this());
        _channel->enableReading();      // 向poller注册channel的EPOLLIN读事件

        _connectionCallback(shared_from_this());            // 新连接建立 执行回调
    }

    void TcpConnection::connectDestroyed() {
        _loop->assertInLoopThread();
        if(_state == kConnected) {
            setState(kDisconnected);

            _channel->disableAll();
            _connectionCallback(shared_from_this());
        }
        _channel->removeChannel();
    }

    /**
     * 其实该函数就是把TCP可读缓冲区的数据读入到inputBuffer_中，以腾出TCP可读缓冲区，
     * 避免反复触发EPOLLIN事件（可读事件），同时执行用户自定义的消息到来时候的回调函数
     * */
    void TcpConnection::handleRead(Tiny_muduo::TimeStamp receiveTime) {
        int saveErrno = 0;
        ssize_t n = _inputBuffer->readFd(_channel->fd(), &saveErrno);
        if(n > 0) {
            // 已建立连接的用户，有可读事件发生，调用用户传入的回调操作
            _messageCallback(shared_from_this(), _inputBuffer.get(), receiveTime);
        }
        else if(n == 0) {
            handleClose();
        }
        else {
            errno = saveErrno;
            LOG_ERROR << "TcpConnection::handleRead() failed";
            handleError();
        }
    }

    void TcpConnection::handleWrite() {
        _loop->assertInLoopThread();
        if(_channel->isWriting()) {
            int saveErrno = 0;
            ssize_t n = sockops::write(_channel->fd(), _outputBuffer->peek(), _outputBuffer->readableBytes());

            if(n > 0) {
                _outputBuffer->retrieve(static_cast<size_t>(n));
                // 说明buffer可读数据都被TcpConnection读取完毕并写入给了客户端
                // 此时就可以关闭连接，否则还需继续提醒写事件
                if(_outputBuffer->readableBytes() == 0) {

                    _channel->disableWriting();

                    if(_writeCompleteCallback) {
                        _loop->queueInLoop(std::bind(_writeCompleteCallback, shared_from_this()));
                    }

                    if(_state == kDisconnecting) {
                        shutdownInLoop();
                    }
                }
            }
            else {
                LOG_ERROR << "TcpConnection::handleWrite() failed";
            }
        }
        else {          // state_不为写状态
            LOG_ERROR << "TcpConnection fd=" << _channel->fd() << " is down, no more writing";
        }
    }

    /**
     * 对方主动关闭连接、或者对方意外关机，我方通过心跳机制给对方发送探测报文时，会触发了EPOLLHUP事件，此时该连接对应的Channel会调用TcpConnection::handleClose函数
     * TcpConnection::handleRead中读取到0字节数据也表示对方主动关闭连接，也会调用TcpConnection::handleClose函数
     * */
    void TcpConnection::handleClose() {
        _loop->assertInLoopThread();
#ifdef DEBUG
        LOG_TRACE << "fd = " << _channel->fd() << " state = " << stateToString();
#endif
        assert(_state == kConnected || _state == kDisconnecting);
        setState(kDisconnected);
        _channel->disableAll();

        TcpConnectionPtr connPtr(shared_from_this());
        _connectionCallback(connPtr);

        // 执行关闭连接的回调 执行的是TcpServer::removeConnection回调方法
        // TcpServer::removeConnection又会调用TcpConnection::connectDestroyed
        _closeCallback(connPtr);
    }


    void TcpConnection::handleError() {
        int err = sockops::getSocketError(_channel->fd());
        LOG_ERROR << "TcpConnection::handleError [" << _name
                  << "] - SO_ERROR = " << err << " " << strerror(err);
    }
    /**
    * 发送数据 应用写的快 而内核发送数据慢 需要把待发送数据写入缓冲区，而且设置了水位回调
     * 在sendInLoop函数中，如果数据没发送完毕，会在EPoller中注册该连接的可写事件，
     * 这样，当TCP可写缓冲区空闲的时候，就会触发该连接对应Channel上的可写事件，即，
     * 最终调用TcpConnection::handleWrite函数。该函数就是负责把剩余未发送的数据（即outputBuffer_中的数据）发送出去.
    **/
    void TcpConnection::sendInLoop(const void *message, size_t len) {
        _loop->assertInLoopThread();
        ssize_t nwrote = 0;
        size_t remaining = len;
        bool faultError = false;

        if(_state == kDisconnected) {
            LOG_ERROR << "disconnected, give up writing";
            return;
        }
        // channel第一次写数据，且缓冲区没有待发送数据
        if(!_channel->isWriting() && _outputBuffer->readableBytes() == 0) {
            nwrote = sockops::write(_channel->fd(), message, len);

            if(nwrote >= 0) {
                remaining = len - static_cast<size_t>(nwrote);       // 判断有没有一次性写完
                // 既然一次性发送完事件就不用让channel对epollout事件感兴趣了
                if(remaining == 0 && _writeCompleteCallback) {
                    _loop->queueInLoop(std::bind(_writeCompleteCallback, shared_from_this()));
                }
            }
            else {
                nwrote = 0;
                // EWOULDBLOCK表示非阻塞情况下没有数据后的正常返回 等同于EAGAIN
                if(errno != EWOULDBLOCK) {
                    LOG_ERROR << "TcpConnection::sendInLoop";
                    if (errno == EPIPE || errno == ECONNRESET) // SIGPIPE
                    {
                        faultError = true;
                    }
                }
            }

        }

        /**
     * 说明当前这一次write并没有把数据全部发送出去 剩余的数据需要保存到缓冲区当中
     * 然后给channel注册EPOLLOUT事件，Poller发现tcp的发送缓冲区有空间后会通知
     * 相应的sock->channel，调用channel对应注册的writeCallback_回调方法，
     * channel的writeCallback_实际上就是TcpConnection设置的handleWrite回调，
     * 把发送缓冲区outputBuffer_的内容全部发送完成
     **/
        assert(remaining <= len);
        // 说明一次性并没有发送完数据，剩余数据需要保存到缓冲区中，且需要改channel注册写事件
        if(!faultError && remaining > 0) {
            size_t oldLen = _outputBuffer->readableBytes();
            if((oldLen + remaining >= _highWaterMark) && oldLen < _highWaterMark && _highWaterMarkCallback) {
                _loop->queueInLoop(std::bind(_highWaterMarkCallback, shared_from_this(), oldLen + remaining));
            }

            _outputBuffer->append(static_cast<const char*>(message) + nwrote, remaining);
            if(!_channel->isWriting()) {
                _channel->enableWriting();
            }
        }
    }

    void TcpConnection::sendInLoop(const std::string &message) {
        sendInLoop(message.data(), message.size());
    }

    void TcpConnection::shutdownInLoop() {
        _loop->assertInLoopThread();
        if(!_channel->isWriting()) {
            _socket->shutdownWrite();
        }
    }

    void TcpConnection::forceClose()
    {
        // FIXME: use compare and swap
        if (_state == kConnected || _state == kDisconnecting)
        {
            setState(kDisconnecting);
            _loop->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
        }
    }


    void TcpConnection::forceCloseInLoop()
    {
        _loop->assertInLoopThread();
        if (_state == kConnected || _state == kDisconnecting)
        {
            // as if we received 0 byte in handleRead();
            handleClose();
        }
    }

    const char* TcpConnection::stateToString() const
    {
        switch (_state)
        {
            case kDisconnected:
                return "kDisconnected";
            case kConnecting:
                return "kConnecting";
            case kConnected:
                return "kConnected";
            case kDisconnecting:
                return "kDisconnecting";
            default:
                return "unknown state";
        }
    }
}