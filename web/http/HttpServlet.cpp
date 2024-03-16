//
// Created by 37496 on 2024/2/23.
//

#include "HttpServlet.h"


namespace Tiny_muduo::Http
{
    const std::string ServletDispatcher::BASE_ROOT = config::GET_CONFIG<std::string>("resource.path", "/root/resource");

    std::string HttpServlet::basePage(int code, const std::string &content) {
        std::string text = std::to_string(code) + " " + HttpStatusCode2Str.at(code);
        return R"(<html><head><title>)" + text +
               R"(</title></head><body><center><h1>)" + text +
               R"(</h1></center><hr><center>)" + content +
               R"(</center></body></html>)";
    }

    ServletDispatcher::ServletDispatcher() :_router(std::make_unique<Router>())
    {
        _defaultServlet = std::make_shared<HttpServlet>([&](const HttpRequest& request, HttpResponse* response){

            std::string path(request.getPath());
            if(path == "/" || path == "/index") {
                path = "/index.html";
            }

            std::filesystem::path p(BASE_ROOT+path);
            bool exists = std::filesystem::exists(p);
            if(exists) {
                std::string ext;
                if(p.has_extension()) {
                    ext = std::string(p.extension().c_str());
                }

                std::unordered_set<std::string> accpetable_exts = {
                        ".html", ".htm", ".txt", ".jpg", ".png", ".css",
                        ".xml", ".xhtml", ".txt", ".rtf", ".pdf", ".word",
                        ".gif", ".jpeg", ".au", ".mpeg", ".mpg",  ".mp4",
                        ".avi", ".gz", ".tar", ".js"
                };

                if(accpetable_exts.find(ext) != accpetable_exts.end()) {
                    response->setStatusCode(HttpStatusCode::OK);
                    response->setContentType(Ext2HttpContentType.at(ext));
                    response->setFile(path);
                }
                else {
                    response->setStatusCode(HttpStatusCode::NOT_FOUND);
                    response->setContentType(HttpContentType::HTML);
                    response->setHtmlBody("/404.html");
                }
            }
            else {
                response->setStatusCode(HttpStatusCode::NOT_FOUND);
                response->setContentType(HttpContentType::HTML);
                response->setHtmlBody("/404.html");
                return;
            }
        });
    }

    void ServletDispatcher::addServlet(const std::string &uri, HttpServlet::servletFunc func){
        WriteLockGuard<ReadWriteLock> lock(_mutex);
        _dispatcher[uri] = std::make_shared<HttpServlet>(std::move(func));
    }

    void ServletDispatcher::addServlet(const std::string &uri, HttpServlet::_ptr servlet){
        WriteLockGuard<ReadWriteLock> lock(_mutex);
        _dispatcher[uri] = servlet;
    }

    HttpServlet::_ptr ServletDispatcher::getServlet(const std::string &uri){
        ReadLockGuard<ReadWriteLock> lock(_mutex);
        auto it = _dispatcher.find(uri);
        if(it == _dispatcher.end()){
            return _defaultServlet;
        }
        return it->second;
    }

    void ServletDispatcher::dispatch(const HttpRequest& request, HttpResponse* response,
                                     HttpServlet::servletFunc preprocess, HttpServlet::servletFunc postProcess) {
        if(preprocess != nullptr) {
            preprocess(request, response);
        }

        std::string path(request.getPath());
        HttpServlet::_ptr p = nullptr;

        auto it = find_match(path);
        if(it == _dispatcher.end()) {
            p = _defaultServlet;
        }
        else {
            p = it->second;
        }
        if(p == _defaultServlet) {
            auto [flag1, flag2, node] = _router->findRoute(request, response);
            if(flag1 && flag2) {
                node->handle(request, response);
            }
            else {
                p->handle(request, response);
            }
        }
        else {
            p->handle(request, response);
        }

        if(postProcess != nullptr) {
            postProcess(request, response);
        }
    }


    ServletDispatcher::Dispatcher::iterator ServletDispatcher::find_match(const std::string &uri) {
        ReadLockGuard<ReadWriteLock> lockGuard(_mutex);

        auto it = _dispatcher.find(uri);
        if(it != _dispatcher.end()) {
            return it;
        }
        for(Dispatcher::iterator it = _dispatcher.begin(); it != _dispatcher.end(); ++it) {
            if(!::fnmatch(it->first.c_str(), uri.c_str(), 0)) {
                return it;
            }
        }
        return _dispatcher.end();
    }


    //-------------------------------------------------------------------------------------------------------------//

//    ServletManager::ServletManager()
//                :staticResourcePath(config::GET_CONFIG<std::string>("resource.path", "/root/resources")),
//                _beginFilter(std::make_shared<Filter>()),
//                _endFilter(std::make_shared<Filter>())
//    {
//        init();
//    }
//
//    void ServletManager::addServlet(std::shared_ptr<Servlet> servlet) {
//        for(const auto& path : servlet->getPaths()) {
//            servlets[path] = servlet;
//        }
//    }
//
//    bool ServletManager::handleStaticResource(const Tiny_muduo::Http::HttpRequest &request,
//                                              Tiny_muduo::Http::HttpResponse *response) {
//        std::string path = request.getPath();
//        std::filesystem::path p(staticResourcePath+path);
//        bool exists = std::filesystem::exists(p);
//        if(exists) {
//            std::string ext;
//            if(p.has_extension()) {
//                ext = std::string(p.extension().c_str());
//            }
//
//            std::unordered_set<std::string> accpetable_exts = {
//                    ".html", ".htm", ".txt", ".jpg", ".png", ".css",
//                    ".xml", ".xhtml", ".txt", ".rtf", ".pdf", ".word",
//                    ".gif", ".jpeg", ".au", ".mpeg", ".mpg",  ".mp4",
//                    ".avi", ".gz", ".tar", ".js"
//            };
//
//            if(accpetable_exts.find(ext) != accpetable_exts.end()) {
//                response->setStatusCode(HttpStatusCode::OK);
//                response->setContentType(Ext2HttpContentType.at(ext));
//                response->setFile(path);
//            }
//            else {
//                response->setStatusCode(HttpStatusCode::NOT_FOUND);
//                response->setContentType(HttpContentType::HTML);
//                response->setHtmlBody("/404.html");
//            }
//        }
//        else {
//            response->setStatusCode(HttpStatusCode::NOT_FOUND);
//            response->setContentType(HttpContentType::HTML);
//            response->setHtmlBody("/404.html");
//        }
//    }
//
//    void ServletManager::handle404(const Tiny_muduo::Http::HttpRequest &request, Tiny_muduo::Http::HttpResponse *response) {
//
//        response->setHttpVersion(request.getVersion());
//        response->setStatusCode(HttpStatusCode::NOT_FOUND);
//        response->setBody("<html><head><title>404 Not Found</title></head>"
//                      "<body><h1>404 Not Found</h1></body></html>");
//    }
//
//    bool ServletManager::handleDynamicResource(const Tiny_muduo::Http::HttpRequest &request, Tiny_muduo::Http::HttpResponse *response){
//        if(servlets.count(request.getPath()) == 0) {
//            return false;
//        }
//        response->setHttpVersion(request.getVersion());
//        response->setStatusCode(HttpStatusCode::OK);
//
//        switch (request.getMethod())
//        {
//            case HttpMethod::GET:
//                servlets[request.getPath()]->doGet(request, response);
//                break;
//            case HttpMethod::POST:
//                servlets[request.getPath()]->doPost(request, response);
//                break;
//            default:
//                break;
//        }
//        return true;
//    }
//
//    void ServletManager::handleRequest(const Tiny_muduo::Http::HttpRequest &request,
//                                       Tiny_muduo::Http::HttpResponse *response) {
//
//        if(handleDynamicResource(request, response)) return;
//        if(handleStaticResource(request, response)) return;
//        handle404(request, response);
//    }
//
//    void
//    ServletManager::handle(const Tiny_muduo::Http::HttpRequest &request, Tiny_muduo::Http::HttpResponse *response) {
//        if(request.getCookie().get("SESSION_ID").exist()){
//            request.getSession()->setValue("SESSION_ID", request.getCookie().get("SESSION_ID").value());
//        }
//        else{
////            std::string sessionID = base::TimeStamp().setNow().toCookieString();
////            //std::cout<<"set SESSIONID: "<<sessionID<<std::endl;
////            resp->addCookie(Cookie("SESSION_ID", sessionID).setMaxAge(1000).setPath("/"));
////            request->getSession()->setSessionID(sessionID);
//        }
//        _filterManager.startChain(request, response);
//    }
}