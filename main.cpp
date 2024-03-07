#include "http/HttpServer.h"
#include "http/HttpService.h"

#include <iostream>


using namespace Tiny_muduo;
using namespace Tiny_muduo::net;
using namespace Tiny_muduo::Http;


int main()
{
    log::Logger::setLogLevel(log::LogLevel::UNKONWN);
    int numThreads = 1;
    EventLoop loop;
    HttpServer server(&loop, InetAddress(8000), "dummy");
    //server.setThreadNum(1);
    server.addServlet("/", [](HttpRequest request, HttpResponse* response) {
        Index(request, response);
    });

    server.addServlet("/get-comments", [](HttpRequest request, HttpResponse* response) {
        GetComments(request, response);
    });

    server.addServlet("/get-num-visits", [](HttpRequest request, HttpResponse* response) {
        GetVisitNum(request, response);
    });

    server.GET("/set-comments/:val", [](HttpRequest request, HttpResponse* response) {
        SetComments(request, response);
    });

    server.addServlet("/index", [](HttpRequest request, HttpResponse* response) {
        Index(request, response);
    });

    server.addServlet("/login", [](HttpRequest request, HttpResponse* response) {
        Login(request, response);
    });
    server.addServlet("/register", [](HttpRequest request, HttpResponse* response) {
        Register(request, response);
    });

    server.addServlet("/picture", [](HttpRequest request, HttpResponse* response) {
        Picture(request, response);
    });

    server.addServlet("/video", [](HttpRequest request, HttpResponse* response) {
        Video(request, response);
    });

    server.addServlet("/blogindex", [](HttpRequest request, HttpResponse* response) {
        BlogIndex(request, response);
    });

    server.addServlet("/fileupload", [](HttpRequest request, HttpResponse* response) {
        FileUpload(request, response);
    });

    server.addServlet("/filedownload", [](HttpRequest request, HttpResponse* response) {
        FileDownload(request, response);
    });

    server.addServlet("/filedownload-src/Everything-1.4.1.1009.x86-Setup.exe", [](HttpRequest request, HttpResponse* response) {
        FileDownload(request, response);
    });

    server.addServlet("/filedownload-src/resume.docx", [](HttpRequest request, HttpResponse* response) {
        FileDownload(request, response);
    });

    server.addServlet("/Aboutme", [](HttpRequest request, HttpResponse* response) {
        AboutMe(request, response);
    });

    server.addServlet("/WebDetails", [](HttpRequest request, HttpResponse* response) {
        WebDetails(request, response);
    });

    server.addServlet("/msgboard", [](HttpRequest request, HttpResponse* response) {
        MsgBoard(request, response);
    });

    server.setThreadNum(numThreads);
    server.start();
    loop.loop();
}

