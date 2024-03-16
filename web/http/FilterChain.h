//
// Created by 37496 on 2024/3/7.
//

#ifndef WEBSERVER_FILTERCHAIN_H
#define WEBSERVER_FILTERCHAIN_H

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "http/HttpRequest.h"

namespace Tiny_muduo::Http
{
    class HttpRequest;
    class HttpResponse;
    class FilterChain;
    class Filter
    {
    public:
        virtual void doFilter(const HttpRequest& request, HttpResponse* response, std::shared_ptr<FilterChain>);
        void setPath(const std::string& path) { path_ = path; }
        const std::string& getPath() { return path_; }
    private:
        std::string path_;
    };

    class FilterChain
    {
    public:
        void doFilter(const HttpRequest& request, HttpResponse* response) {
            auto trigeredUrl = request.getRequestLine().getUrl();
            auto path = curFilter_->getPath();
            //std::cout<<"here is chain: "<<path<<std::endl;
            if(path == trigeredUrl.substr(0, std::min(path.length(), trigeredUrl.length()))){
                curFilter_->doFilter(request, response, nextChain_);
            }
            else nextChain_->doFilter(request, response);
        }

        void setFilter(std::shared_ptr<Filter> filter) { curFilter_ = std::move(filter); }
        void setNextChain(std::shared_ptr<FilterChain> filterChain) { nextChain_ = std::move(filterChain); }
    private:
        std::shared_ptr<Filter> curFilter_;
        std::shared_ptr<FilterChain> nextChain_;
    };

    class FilterChainManager
    {
    public:
        void regist(std::shared_ptr<Filter> filter) {
            std::shared_ptr<FilterChain> chain = std::make_shared<FilterChain>();
            chain->setFilter(std::move(filter));
            chain->setNextChain(end_);
            if(!filterChains.empty()) {
                begin_->setNextChain(filterChains.front());
                filterChains.back()->setNextChain(chain);
            }
            else {
                begin_->setNextChain(chain);
            }
            filterChains.push_back(chain);
        }

        void registerBegin(std::shared_ptr<Filter> filter) {
            begin_ = std::make_shared<FilterChain>();
            begin_->setFilter(std::move(filter));
            begin_->setNextChain(end_);
        }
        void registerEnd(std::shared_ptr<Filter> filter) {
            end_ = std::make_shared<FilterChain>();
            end_->setFilter(std::move(filter));
        }

        void startChain(const HttpRequest& request, HttpResponse* response) {
            begin_->doFilter(request, response);
        }
    private:
        std::vector<std::shared_ptr<FilterChain>> filterChains;
        std::shared_ptr<FilterChain> begin_;
        std::shared_ptr<FilterChain> end_;
    };
}
#endif //WEBSERVER_FILTERCHAIN_H
