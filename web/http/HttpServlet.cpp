//
// Created by 37496 on 2024/2/23.
//

#include "HttpServlet.h"


namespace Tiny_muduo::Http
{
    static const std::string BASE_ROOT = config::GET_CONFIG<std::string>("resource", "/root/resource/");
    static const uintmax_t small_file_limit = 4 *1024 * 1024; // 4M

    std::string HttpServlet::basePage(int code, const std::string &content) {
        std::string text = std::to_string(code) + " " + HttpStatusCode2Str.at(code);
        return R"(<html><head><title>)" + text +
               R"(</title></head><body><center><h1>)" + text +
               R"(</h1></center><hr><center>)" + content +
               R"(</center></body></html>)";
    }

    ServletDispatcher::ServletDispatcher() {
        _defaultServlet = std::make_shared<HttpServlet>([&](const HttpRequest& req, HttpResponse* resp){
            auto code = resp->getStatusCode();

            if(code >= 400) {
                resp->setContentType(HttpContentType::HTML);
                switch (code) {
                    case BAD_REQUEST:
                        resp->setHtmlBody("400.html");
                        break;
                    case UNAUTHORIZED:
                        resp->setHtmlBody("401.html");
                        break;
                    case FORBIDDEN:
                        resp->setHtmlBody("403.html");
                        break;
                    case NOT_FOUND:
                        resp->setHtmlBody("404.html");
                        break;
                    case METHOD_NOT_ALLOWED:
                        resp->setHtmlBody("405.html");
                        break;
                    case RANGE_NOT_SATISFIABLE:
                        resp->setHtmlBody("416.html");
                    default:
                        resp->appendBody(HttpServlet::basePage(code));
                        break;
                }
                return ;
            }

            auto url = req.getUrl();
            std::string path(url.getPath());
            if(path == "/") {
                path = "index.html";
            }

            std::filesystem::path p(BASE_ROOT+std::string(path));
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
                    resp->setContentType(Ext2HttpContentType.at(ext));
                    resp->setFileBody(p.c_str());
                }
                else {
                    resp->setStatusCode(HttpStatusCode::NOT_FOUND);
                    resp->setContentType(HttpContentType::HTML);
                    resp->setHtmlBody("404.html");
                }
            }
            else {
                resp->setStatusCode(HttpStatusCode::NOT_FOUND);
                resp->setContentType(HttpContentType::HTML);
                resp->setHtmlBody("404.html");
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

        std::string path(request.getUrl().getPath());
        HttpServlet::_ptr p = nullptr;

        auto it = find_match(path);
        if(it == _dispatcher.end()) {
            p = _defaultServlet;
        }
        else {
            p = it->second;
        }

        p->handle(request, response);

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

}