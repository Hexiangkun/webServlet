////
//// Created by 37496 on 2024/2/26.
////
//
//#ifndef WEBSERVER_HTTPROUTER_H
//#define WEBSERVER_HTTPROUTER_H
//
//#include <string>
//#include <vector>
//#include <regex>
//#include <variant>
//#include <optional>
//
//namespace Tiny_muduo::Http
//{
//    class HttpRequest;
//    class HttpResponse;
//    typedef std::function<void(HttpRequest, HttpResponse)> RouteHandler;
//    typedef std::function<void(HttpRequest, HttpResponse, RouteHandler)> MiddleWareHandle;
//    typedef std::variant<RouteHandler , MiddleWareHandle > RouteFunction;
//
//    struct RouteDefinition {
//        std::string route_name;
//        std::string method;
//        std::vector<std::string> params_names;
//        std::regex route_regex;
//        RouteHandler handler;
//    };
//
//    RouteDefinition routeStr2Definition(std::string str);
//
//    std::optional<std::map<std::string, std::string>> matchRoute(RouteDefinition def, std::string str);
//
//    std::optional<std::map<std::string, std::string>> matchRequest(RouteDefinition def, HttpRequest req);
//}
//
//
//#endif //WEBSERVER_HTTPROUTER_H
