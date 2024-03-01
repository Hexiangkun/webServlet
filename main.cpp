#include "http/HttpServer.h"
#include "http/HttpService.h"

#include <iostream>
#include <map>
namespace Tiny_muduo::net
{
#define DEBUG
}
using namespace Tiny_muduo;
using namespace Tiny_muduo::net;
using namespace Tiny_muduo::Http;

extern char favicon[555];

bool benchmark = false;


void onRequest(const HttpRequest& req, HttpResponse* resp)
{
    if (req.getPath()  == "/")
    {
        resp->setStatusCode(HttpStatusCode::OK);
        resp->setContentType(HttpContentType::HTML);

        std::string now = TimeStamp::now().toFormatString();
        resp->setBody("<html><head><title>This is title</title></head>"
                      "<body><h1>Hello</h1>Now is " + now +
                      "</body></html>");
    }
    else
    {
        resp->setStatusCode(HttpStatusCode::NOT_FOUND);
        resp->setCloseConnection(true);
    }
}

int main(int argc, char* argv[])
{
    log::Logger::setLogLevel(log::LogLevel::UNKONWN);
    int numThreads = 1;
    benchmark = true;
    EventLoop loop;
    HttpServer server(&loop, InetAddress(8000), "dummy");
    //server.setThreadNum(1);
    server.addServlet("/", [](HttpRequest request, HttpResponse* response) {
        Index(request, response);
    });
    server.addServlet("/login", [](HttpRequest request, HttpResponse* response) {
        Login(request, response);
    });
    server.addServlet("/register", [](HttpRequest request, HttpResponse* response) {
        Register(request, response);
    });

    server.setThreadNum(numThreads);
    server.start();
    loop.loop();
}

