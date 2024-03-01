//
// Created by 37496 on 2024/2/11.
//

#include "Connector.h"
#include "log/Log.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"

namespace Tiny_muduo::net
{
    const int Connector::kMaxRetryDelayMs;

    Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
            : _loop(loop),
              _serverAddr(serverAddr),
              _connect(false),
              _state(kDisconnected),
              _retryDelayMs(kInitRetryDelayMs)
    {
#ifdef DEBUG
        LOG_DEBUG << "ctor[" << this << "]";
#endif
    }

    Connector::~Connector()
    {
#ifdef DEBUG
        LOG_DEBUG << "dtor[" << this << "]";
#endif
        assert(!_channel);
    }

    void Connector::start()
    {
        _connect = true;
        _loop->runInLoop(std::bind(&Connector::startInLoop, this)); // FIXME: unsafe
    }

    void Connector::startInLoop()
    {
        _loop->assertInLoopThread();
        assert(_state == kDisconnected);
        if (_connect)
        {
            connect();
        }
        else
        {
#ifdef DEBUG
            LOG_DEBUG << "do not connect";
#endif
        }
    }

    void Connector::stop()
    {
        _connect = false;
        _loop->queueInLoop(std::bind(&Connector::stopInLoop, this)); // FIXME: unsafe
        // FIXME: cancel timer
    }

    void Connector::stopInLoop()
    {
        _loop->assertInLoopThread();
        if (_state == kConnecting)
        {
            setState(kDisconnected);
            int sockfd = removeAndResetChannel();
            retry(sockfd);
        }
    }

    void Connector::connect()
    {
        int sockfd = sockops::createNonBlockSocket();
        int ret = sockops::connect(sockfd, _serverAddr.getSockAddress());
        int savedErrno = (ret == 0) ? 0 : errno;
        switch (savedErrno)
        {
            case 0:
            case EINPROGRESS:
            case EINTR:
            case EISCONN:
                connecting(sockfd);
                break;

            case EAGAIN:
            case EADDRINUSE:
            case EADDRNOTAVAIL:
            case ECONNREFUSED:
            case ENETUNREACH:
                retry(sockfd);
                break;

            case EACCES:
            case EPERM:
            case EAFNOSUPPORT:
            case EALREADY:
            case EBADF:
            case EFAULT:
            case ENOTSOCK:
                LOG_ERROR << "connect error in Connector::startInLoop " << savedErrno;
                sockops::close(sockfd);
                break;

            default:
                LOG_ERROR << "Unexpected error in Connector::startInLoop " << savedErrno;
                sockops::close(sockfd);
                // connectErrorCallback_();
                break;
        }
    }

    void Connector::restart()
    {
        _loop->assertInLoopThread();
        setState(kDisconnected);
        _retryDelayMs = kInitRetryDelayMs;
        _connect = true;
        startInLoop();
    }

    void Connector::connecting(int sockfd)
    {
        setState(kConnecting);
        assert(!_channel);
        _channel.reset(new Channel(_loop, sockfd));
        _channel->setWriteCallback(
                std::bind(&Connector::handleWrite, this)); // FIXME: unsafe
        _channel->setErrorCallback(
                std::bind(&Connector::handleError, this)); // FIXME: unsafe

        // _channel->tie(shared_from_this()); is not working,
        // as _channel is not managed by shared_ptr
        _channel->enableWriting();
    }

    int Connector::removeAndResetChannel()
    {
        _channel->disableAll();
        _channel->removeChannel();
        int sockfd = _channel->fd();
        // Can't reset _channel here, because we are inside Channel::handleEvent
        _loop->queueInLoop(std::bind(&Connector::resetChannel, this)); // FIXME: unsafe
        return sockfd;
    }

    void Connector::resetChannel()
    {
        _channel.reset();
    }

    void Connector::handleWrite()
    {
#ifdef DEBUG
        LOG_TRACE << "Connector::handleWrite " << _state;
#endif
        if (_state == kConnecting)
        {
            int sockfd = removeAndResetChannel();
            int err = sockops::getSocketError(sockfd);
            if (err)
            {
#ifdef DEBUG
                LOG_WARN << "Connector::handleWrite - SO_ERROR = "
                         << err << " " << (err);
#endif
                retry(sockfd);
            }
            else if (sockops::isSelfConnect(sockfd))
            {
#ifdef DEBUG
                LOG_WARN << "Connector::handleWrite - Self connect";
#endif
                retry(sockfd);
            }
            else
            {
                setState(kConnected);
                if (_connect)
                {
                    _newConnectionCallback(sockfd);
                }
                else
                {
                    sockops::close(sockfd);
                }
            }
        }
        else
        {
            // what happened?
            assert(_state == kDisconnected);
        }
    }

    void Connector::handleError()
    {

        LOG_ERROR << "Connector::handleError state=" << _state;
        if (_state == kConnecting)
        {
            int sockfd = removeAndResetChannel();
            int err = sockops::getSocketError(sockfd);

            LOG_TRACE << "SO_ERROR = " << err << " " << (err);
            retry(sockfd);
        }
    }

    // 采用back-off策略重连，即重连时间逐渐延长，0.5s, 1s, 2s, ...直至30s
    void Connector::retry(int sockfd)
    {
        sockops::close(sockfd);
        setState(kDisconnected);
        if (_connect)
        {
#ifdef DEBUG
            LOG_INFO << "Connector::retry - Retry connecting to " << _serverAddr.toString()
                     << " in " << _retryDelayMs << " milliseconds. ";
#endif
            // 注册一个定时操作，重连
            _loop->runAfter(_retryDelayMs/1000.0,
                            std::bind(&Connector::startInLoop, shared_from_this()));
            _retryDelayMs = std::min(_retryDelayMs * 2, kMaxRetryDelayMs);
        }
        else
        {
#ifdef DEBUG
            LOG_DEBUG << "do not connect";
#endif
        }
    }


}