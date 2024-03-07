//
// Created by 37496 on 2024/3/7.
//

#include "HttpRoute.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include <utility>

namespace Tiny_muduo::Http
{
    void Router::AddRouter(const std::string &method, const std::string &pattern, ContextHandler handler) {
        std::vector<std::string> parts;
        Trie::ParsePattern(pattern, parts);

        if(roots_.find(method) == roots_.end()) {
            roots_[method] = new Trie();
        }

        roots_[method]->Insert(pattern, std::move(handler));
    }

    std::tuple<bool, bool, Node*> Router::findRoute(const HttpRequest& request, HttpResponse* response) {
        if(roots_.find(request.getMethodStr()) == roots_.end()) return std::make_tuple(false, false, nullptr);

        std::string paths = request.getPath();
        std::vector<std::string> parts;
        Trie::ParsePattern(paths, parts);

        Node* node = roots_[request.getMethodStr()]->Search(parts);

        if(node != nullptr) {
            std::vector<std::string> node_parts;
            Trie::ParsePattern(node->pattern_, node_parts);
            bool flag = false;
            for(int i = 0; i < node_parts.size(); ++i) {
                if(node_parts[i][0] == ':') {
                    flag = true;
                    response->getParams()[node_parts[i].substr(1)] = parts[i];
                }
            }

            return std::make_tuple(true, flag, node);
        }

        return std::make_tuple(false, false, nullptr);
    }

//    void Router::Handle(const HttpRequest& request, HttpResponse* response) {
//        auto [flag, node] = findRoute(request, response);
//        if(flag) {
//            node->handler_(request, response);
//        }else {
//
//        }
//    }
}