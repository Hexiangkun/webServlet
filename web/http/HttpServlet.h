//
// Created by 37496 on 2024/2/23.
//

#ifndef WEBSERVER_HTTPSERVLET_H
#define WEBSERVER_HTTPSERVLET_H


#include <memory>
#include <functional>
#include <unordered_set>
#include <fnmatch.h>
#include <utility>
#include <vector>
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "http/FilterChain.h"
#include "config/Config.h"

namespace Tiny_muduo::Http
{
    class HttpServlet
    {
    public:
        typedef std::shared_ptr<HttpServlet> _ptr;
        typedef std::function<void(const HttpRequest&,  HttpResponse*)> servletFunc;

        HttpServlet(servletFunc func) : _func(std::move(func)) {}
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
        static const std::string BASE_ROOT;
    };

//    class Servlet
//    {
//    public:
//        void setPath(std::string path) {_paths.push_back(std::move(path)); }
//        const std::vector<std::string>& getPaths() const { return _paths; }
//
//        virtual void doGet(const HttpRequest& request, HttpResponse* response) = 0;
//        virtual void doPost(const HttpRequest& request, HttpResponse* response) = 0;
//
//    private:
//        std::vector<std::string> _paths;
//    };
//
//    class ServletManager
//    {
//    public:
//        ServletManager();
//
//        void init(){_filterManager.registerEnd(_endFilter);_filterManager.registerBegin(_beginFilter);}
//        void addServlet(std::shared_ptr<Servlet> servlet);
//
//        bool handleStaticResource(const HttpRequest& request, HttpResponse* response);
//        void handle404(const HttpRequest& request, HttpResponse* response);
//        bool handleDynamicResource(const HttpRequest& request, HttpResponse* response);
//        void handleRequest(const HttpRequest& request, HttpResponse* response);
//
//        void handle(const HttpRequest& request, HttpResponse* response);
//
//        void registerFilter(std::shared_ptr<Filter> filter){_filterManager.regist(filter);}
//    private:
//        std::map<std::string, std::shared_ptr<Servlet>> servlets;
//        const std::string staticResourcePath;
//        FilterChainManager _filterManager;
//        std::shared_ptr<Filter> _beginFilter;
//        std::shared_ptr<Filter> _endFilter;
//    };



}


#endif //WEBSERVER_HTTPSERVLET_H
