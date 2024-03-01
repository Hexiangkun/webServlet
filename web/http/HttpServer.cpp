//
// Created by 37496 on 2024/2/25.
//

#include "HttpServer.h"
#include "HttpContext.h"
#include "log/Log.h"

namespace Tiny_muduo::Http
{
    namespace detail
    {
        void defaultHttpCallback(const HttpRequest&, HttpResponse* resp)
        {
//            resp->setStatusCode(HttpResponse::k404NotFound);
//            resp->setStatusMessage("Not Found");
            resp->setStatusCode(HttpStatusCode::NOT_FOUND);
            resp->setCloseConnection(true);
        }

    }



HttpServer::HttpServer(net::EventLoop* loop, const net::InetAddress& listenAddr, const std::string& name)
        : m_server(loop, listenAddr, name),
          m_httpCallback(detail::defaultHttpCallback),
          m_servletDispatcher(std::make_shared<ServletDispatcher>()),
          m_sessionManager(&HttpSessionManager::getInstance())
{
    m_server.setConnectionCallback(std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
    m_server.setMessageCallback(std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

HttpServer::~HttpServer()
{
}

void HttpServer::start()
{
#ifdef DEBUG
    LOG_WARN << "HttpServer[" << m_server.name() << "] starts listenning on " << m_server.ipPort();
#endif
    m_server.start();
}

void HttpServer::onConnection(const net::TcpConnection::_ptr& conn)
{
    if (conn->connected())
    {
        conn->setContext(std::make_shared<HttpContext>());
    }
}

    void HttpServer::onMessage(const net::TcpConnection::_ptr& conn, Buffer* buf, TimeStamp receiveTime)
    {
#ifndef DEBUG
        LOG_INFO << buf->toString() ;
#endif
        std::shared_ptr<HttpContext> context = std::static_pointer_cast<HttpContext>(conn->getContext());
        if (!context->parseRequest(buf, receiveTime))
        {
            conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
            conn->shutdown();
        }

        if (context->gotAll())
        {
            onRequest(conn, *context->request().get());
            context->reset();
        }
    }

void HttpServer::onRequest(const net::TcpConnection::_ptr& conn, const HttpRequest& req)
{
    auto tmp = req.getHeader().getConnection();
    std::string connection = tmp.exist() ? tmp.value() : "";

    bool close = (connection == "close") ||
                 (req.getVersion() == HttpVersion::HTTP_1_0 && connection != "Keep-Alive");
    HttpResponse response(close);
    if(m_servletDispatcher) {
        m_servletDispatcher->dispatch(req, &response, nullptr, nullptr);
    }
    //m_httpCallback(req, &response);
    Buffer buf;
    response.appendToBuffer(&buf);
#ifndef DEBUG
    LOG_INFO << buf.toString() ;
#endif
    conn->send(&buf);
    if (response.closeConnection())
    {
        conn->shutdown();
    }
}

    void HttpServer::beforeServlet(Tiny_muduo::Http::HttpRequest request, Tiny_muduo::Http::HttpResponse *response) {
        std::string req_ssid = request.getCookie().getSessionId();
        if(!req_ssid.empty()) {
            HttpCookie cookie;
            cookie.add("SESSIONID", req_ssid);
            response->setCookie(cookie);
        }
    }

    void HttpServer::afterServlet(const net::TcpConnection::_ptr& conn, Tiny_muduo::Http::HttpRequest request, Tiny_muduo::Http::HttpResponse *response) {
        HttpSession::_ptr session = request.getSession(false);
        if(session) {
            if(!session->isDestroy()) {
                if(session->isNew()) {
                    session->setStatus(HttpSession::Accessed);
                    net::TimerId timerId = conn->getLoop()->runAt(addTime(TimeStamp::now(), session->getMaxInactiveInterval()), [&](){
                        handleSessionTimeout(session);
                    });
                    session->setTimerId(timerId);
                }
                else {
                    auto it = session->getTimerId();
                    conn->getLoop()->cancel(it);
                    net::TimerId timerId = conn->getLoop()->runAt(addTime(TimeStamp::now(), session->getMaxInactiveInterval()), [&](){
                        handleSessionTimeout(session);
                    });
                    session->setTimerId(timerId);
                }
            }
        }
    }

    void HttpServer::handleSessionTimeout(HttpSession::_ptr session) {
        if(session) {
            session->setStatus(HttpSession::Destroy);
            m_sessionManager->delSession(session->getId());
        }
    }

}