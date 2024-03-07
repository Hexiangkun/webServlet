//
// Created by 37496 on 2024/2/25.
//

#ifndef WEBSERVER_HTTPSERVER_H
#define WEBSERVER_HTTPSERVER_H


#include <functional>
#include <unordered_map>
#include <utility>
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "net/tcp/EventLoop.h"
#include "log/Log.h"
#include "net/tcp/InetAddress.h"
#include "net/tcp/Callback.h"
#include "net/tcp/TcpServer.h"
#include "net/tcp/TcpConnection.h"
#include "http/HttpServlet.h"

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

        void setServletDispatcher(ServletDispatcher::_ptr servletDispatcher) { m_servletDispatcher = std::move(servletDispatcher); }

        void addServlet(const std::string& uri, HttpServlet::servletFunc func) {
            if(m_servletDispatcher != nullptr) {
                m_servletDispatcher->addServlet(uri, std::move(func));
            }
        }
        void addServlet(const std::string& uri, HttpServlet::_ptr servlet) {
            if(m_servletDispatcher != nullptr) {
                m_servletDispatcher->addServlet(uri, std::move(servlet));
            }
        }

        void GET(const std::string& path, ContextHandler handler) {
            m_servletDispatcher->GET(path, std::move(handler));
        }

        void POST(const std::string& path, ContextHandler handler) {
            m_servletDispatcher->POST(path, std::move(handler));
        }

        void start();

    private:
        void onConnection(const net::TcpConnection::_ptr& conn);
        void onMessage(const net::TcpConnection::_ptr& conn, Buffer* buf, TimeStamp receiveTime);
        void onRequest(const net::TcpConnection::_ptr&, const HttpRequest&);
        void onIdle(std::weak_ptr<net::TcpConnection>& connection);

        void beforeServlet(HttpRequest request, HttpResponse* response);
        void afterServlet(const net::TcpConnection::_ptr&, HttpRequest request, HttpResponse* response);
        void handleSessionTimeout(HttpSession::_ptr session);


        net::TcpServer m_server;
        HttpCallback m_httpCallback;
        ServletDispatcher::_ptr m_servletDispatcher;
        HttpSessionManager* m_sessionManager;
        bool auto_close_idle_connection_;
        static const int kConnectionTimeout = 80;
    };
}


#endif //WEBSERVER_HTTPSERVER_H
