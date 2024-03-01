//
// Created by 37496 on 2024/2/4.
//

#ifndef WEBSERVER_TOSTRING_H
#define WEBSERVER_TOSTRING_H

#include <string>
#include <sstream>
#include <tuple>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <optional>

namespace hxk_muduo
{
    template<typename... Ts>
    std::string to_string(const Ts&... ts) {
        std::stringstream ss;
        const char* sep = "";
        auto func = [](auto& t) {
            if constexpr (std::is_same_v<std::decay_t<decltype(t)>, int8_t> ||
                    std::is_same_v<std::decay_t<decltype(t)>, uint8_t>) {
                return static_cast<int32_t >(t);
            }
            else {
                return t;
            }
        };
        ((static_cast<void>(ss << sep << func(ts)), sep=" "), ...);
        return ss.str();
    }

    template<typename... Args>
    std::string to_string(const std::tuple<Args...>& t) {
        return std::apply([](const auto&... ts) { return to_string(ts...); }, t);
    }


    template<typename T>
    inline std::enable_if_t<std::is_arithmetic<T>::value && !std::is_same<T, bool>::value, std::string> to_string(T& value) {
        return std::to_string(value);
    }

    inline std::string to_string(bool value) {
        return value ? "true" : "false";
    }

    inline std::string to_string(const std::string& value) {
        return value;
    }

    template<typename T>
    inline std::string to_string(const std::vector<T>& vec) {
        std::string res = "[";
        auto it = vec.begin();
        if (it != vec.end()) {
            res += to_string(*it);
            ++it;
        }
        for (; it != vec.end(); ++it) {
            res += ",";
            res += to_string(*it);
        }
        res += "]";
        return res;
    }

    template<typename T>
    inline std::string to_string(const std::unordered_set<T>& set) {
        std::string res = "[";
        auto it = set.begin();
        if (it != set.end()) {
            res += to_string(*it);
            ++it;
        }
        for (; it != set.end(); ++it) {
            res += ",";
            res += to_string(*it);
        }
        res += "]";
        return res;
    }

    template<typename K, typename V>
    inline std::string to_string(const std::unordered_map<K, V>& map) {
        std::string res = "{";
        auto it = map.begin();
        if (it != map.end()) {
            res += to_string(it->first);
            res += "->";
            res += to_string(it->second);
            ++it;
        }
        for (; it != map.end(); ++it) {
            res += ",";
            res += to_string(it->first);
            res += "->";
            res += to_string(it->second);
        }
        res += "}";
        return res;
    }

    template<typename T>
    inline std::string to_string(const std::optional<T>& opt) {
        std::stringstream ss;
        if(opt) {
            if constexpr (std::is_fundamental_v<T> || std::is_same_v<T, std::string >) {
                ss << opt.value();
            }
            else {
                ss << to_string(opt.value());
            }
        }
        else {
            ss << "(nullopt)";
        }
        return ss.str();
    }

}
#endif //WEBSERVER_TOSTRING_H
