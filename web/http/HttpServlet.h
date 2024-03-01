//
// Created by 37496 on 2024/2/23.
//

#ifndef WEBSERVER_HTTPSERVLET_H
#define WEBSERVER_HTTPSERVLET_H


#include <memory>
#include <functional>
#include <unordered_set>
#include <fnmatch.h>

#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "config/Config.h"

namespace Tiny_muduo::Http
{
    class HttpServlet
    {
    public:
        typedef std::shared_ptr<HttpServlet> _ptr;
        typedef std::function<void(const HttpRequest&,  HttpResponse*)> servletFunc;

        HttpServlet(servletFunc func) : _func(func) {}
        void handle(const HttpRequest& request, HttpResponse* response) { _func(request, response); }

        static std::string basePage(int code , const std::string& content = "WebServer");
    private:
        servletFunc _func;
    };


    class ServletDispatcher
    {
    public:
        typedef std::shared_ptr<ServletDispatcher> _ptr;
        typedef std::unordered_map<std::string , HttpServlet::_ptr > Dispatcher;

        ServletDispatcher();
        void addServlet(const std::string& uri, HttpServlet::_ptr httpServlet);
        void addServlet(const std::string& uri, HttpServlet::servletFunc func);

        HttpServlet::_ptr getServlet(const std::string& uri);

        void dispatch(const HttpRequest& request, HttpResponse* response,
                      HttpServlet::servletFunc preprocess, HttpServlet::servletFunc postProcess);

    private:
        Dispatcher::iterator find_match(const std::string& uri);

        Dispatcher _dispatcher;
        HttpServlet::_ptr _defaultServlet;
        ReadWriteLock _mutex;
    };
}


#endif //WEBSERVER_HTTPSERVLET_H
