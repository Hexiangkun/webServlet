////
//// Created by 37496 on 2024/2/26.
////
//
//#include "HttpRouter.h"
//#include "http/HttpRequest.h"
//
//
//namespace Tiny_muduo::Http
//{
//    RouteDefinition routeStr2Definition(std::string str) {
//        RouteDefinition def{.route_name = str};
//        std::regex slash("\\/");
//        str = std::regex_replace(str, slash, R"(\/)");
//        str = "^" + str;
//        std::regex url_param(R"((\[[^ \t\r\n\/\\]+\]))",
//                             std::regex_constants::ECMAScript);
//        do {
//            std::smatch params_sm;
//            std::regex_search(str, params_sm, url_param);
//            if (params_sm.size() <= 1)
//                break;
//
//            for (int i = 1; i < params_sm.size(); i++) {
//                std::string param_name = params_sm[i].str();
//                param_name = std::string(param_name.begin() + 1, param_name.end() - 1);
//                str.replace(params_sm.position(i), params_sm.length(i),
//                            R"(([^ \t\r\n\/\\]+))");
//                def.params_names.push_back(param_name);
//            }
//        } while (true);
//        str += R"(\/?$)";
//        def.route_regex = std::regex(str, std::regex_constants::ECMAScript);
//        return def;
//    }
//
//
//    std::optional<std::map<std::string, std::string>> matchRoute(RouteDefinition def, std::string str) {
//        std::smatch matches;
//        if (!std::regex_search(str, matches, def.route_regex)) {
//            return {};
//        }
//
//        std::map<std::string, std::string> params;
//        for (int i = 0; i < def.params_names.size(); i++) {
//            params[def.params_names[i]] = matches[i + 1];
//        }
//
//        return params;
//    }
//
//    std::optional<std::map<std::string, std::string>> matchRequest(RouteDefinition def, HttpRequest req) {
//        if (def.method != req.getMethodStr())
//            return {};
//        return matchRoute(def, std::string(req.getUrl().getPath()));
//    }
//}