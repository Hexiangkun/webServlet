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
            resp->setStatusCode(HttpResponse::k404NotFound);
            resp->setStatusMessage("Not Found");
            resp->setCloseConnection(true);
        }

    }



HttpServer::HttpServer(net::EventLoop* loop, const net::InetAddress& listenAddr, const std::string& name)
        : m_server(loop, listenAddr, name),
          m_httpCallback(detail::defaultHttpCallback)
{
    m_server.setConnectionCallback(std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
    m_server.setMessageCallback(std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

HttpServer::~HttpServer()
{
}

void HttpServer::start()
{
    m_server.start();
}

void HttpServer::onConnection(const net::TcpConnection::_ptr& conn)
{
    if (conn->connected())
    {
        //conn->setContext(std::make_shared<HttpContext>());
        conn->setAnyContext(HttpContext());
    }
}

void HttpServer::onMessage(const net::TcpConnection::_ptr& conn, Buffer* buf, TimeStamp receiveTime)
{
#ifdef USE_DEBUG
        LOG_INFO << buf->toString() ;
#endif
    //HttpContext* context = std::any_cast<HttpContext>(conn->getMutableAnyContext());
//    std::shared_ptr<HttpContext> context = std::static_pointer_cast<HttpContext>(conn->getContext());
//    if (!context->parseRequest(buf, receiveTime))
//    {
//        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
//        conn->shutdown();
//    }
//
//    if (context->gotAll())
//    {
//        onRequest(conn, *context->request().get());
//        context->reset();
//    }
    HttpContext* context = std::any_cast<HttpContext>(conn->getMutableAnyContext());

    if (!context->parseRequest(buf, receiveTime))
    {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }

    if (context->gotAll())
    {
        onRequest(conn, context->request());
        context->reset();
    }
}

void HttpServer::onRequest(const net::TcpConnection::_ptr& conn, const HttpRequest& req)
{
    //ReturnOption<std::string> con = req.getHeader().getConnection();
    //bool close = true;
//    if(connection.exist()) {
//        close = (connection.value() == "close") ||
//                (req.getVersion() == HttpVersion::HTTP_1_0 && connection.value() != "Keep-Alive");
//    }
    //const std::string connection = con.value();
    //bool close = connection == "close" ||
      //  (req.getVersion() == HttpVersion::HTTP_1_0 && connection != "Keep-Alive");
    const string& connection = req.getHeader("Connection");
    bool close = connection == "close" ||
                 (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
    HttpResponse response(close);
    m_httpCallback(req, &response);
    Buffer buf;
    response.appendToBuffer(&buf);
#ifdef USE_DEBUG
    LOG_INFO << buf.toString() ;
#endif
    conn->send(&buf);
    if (response.closeConnection())
    {
        conn->shutdown();
    }
}

}