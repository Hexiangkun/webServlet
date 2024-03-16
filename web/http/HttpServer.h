//
// Created by 37496 on 2024/2/25.
//

#ifndef WEBSERVER_HTTPSERVER_H
#define WEBSERVER_HTTPSERVER_H


#include <functional>
#include <unordered_map>
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "net/tcp/EventLoop.h"
#include "log/Log.h"
#include "net/tcp/InetAddress.h"
#include "net/tcp/Callback.h"
#include "net/tcp/TcpServer.h"
#include "net/tcp/TcpConnection.h"

namespace Tiny_muduo::Http
{
    class HttpServer
    {
    public:
        typedef std::function<void (const HttpRequest&, HttpResponse*)> HttpCallback;
        HttpServer(net::EventLoop* loop, const net::InetAddress& listenAddr, const std::string& name);

        ~HttpServer();  // force out-line dtor, for scoped_ptr members.

        /// Not thread safe, callback be registered before calling start().
        void setHttpCallback(const HttpCallback& cb)
        {
            m_httpCallback = cb;
        }

        void setThreadNum(int numThreads)
        {
            m_server.setThreadNum(numThreads);
        }

        void start();

    private:
        void onConnection(const net::TcpConnection::_ptr& conn);
        void onMessage(const net::TcpConnection::_ptr& conn, Buffer* buf, TimeStamp receiveTime);
        void onRequest(const net::TcpConnection::_ptr&, const HttpRequest&);

        net::TcpServer m_server;
        HttpCallback m_httpCallback;
    };
}


#endif //WEBSERVER_HTTPSERVER_H
