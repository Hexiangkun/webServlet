//
// Created by 37496 on 2024/2/4.
//

#ifndef WEBSERVER_YAMLUTIL_H
#define WEBSERVER_YAMLUTIL_H


#include <yaml-cpp/yaml.h>
#include <exception>
#include <chrono>
#include <unordered_set>
#include <iostream>
#include <optional>
#include <variant>
#include "json/nlohmann/json.h"

namespace yaml_struct
{
    inline nlohmann::json parse_scalar(const YAML::Node& node) {
        int i;
        double d;
        bool b;
        std::string s;
        if(YAML::convert<int>::decode(node, i)){
            return i;
        }
        else if(YAML::convert<double>::decode(node, d)) {
            return d;
        }
        else if(YAML::convert<bool>::decode(node, b)) {
            return b;
        }
        else if(YAML::convert<std::string>::decode(node, s)) {
            return s;
        }
        return nullptr;
    }

    inline nlohmann::json yaml2json(const YAML::Node& root) {
        nlohmann::json j;
        switch (root.Type()) {
            case YAML::NodeType::Null:
                break;
            case YAML::NodeType::Scalar:
                return parse_scalar(root);
            case YAML::NodeType::Sequence:
                for(auto&& node : root) {
                    j.emplace_back(yaml2json(node));
                }
                break;
            case YAML::NodeType::Map:
                for(auto&& it : root) {
                    j[it.first.as<std::string>()] = yaml2json(it.second);
                }
                break;
            default:
                break;
        }
        return j;
    }

    inline nlohmann::json yaml_to_json(const std::string& str) {
        YAML::Node root = YAML::Load(str);
        return yaml2json(root);
    }

    class Exception : public std::exception
    {
    public:
        explicit Exception(const char* what) noexcept :_what(what) {

        }
        explicit Exception(const std::string& what) noexcept :_what(what) {

        }

        const char* what() const noexcept override {
            return _what.c_str();
        }
    protected:
        Exception() noexcept { }
    private:
        std::string _what;
    };

    //将一组参数插入到输入的字符串格式中，并返回格式化后的字符串结果
    template<typename... Args>
    inline std::string string_format(const std::string& format, Args... args) {
        int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;
        if(size_s <= 0) {
            throw std::runtime_error("Error during formatting");
        }
        auto size = static_cast<size_t>(size_s);
        std::unique_ptr<char[]> buf(new char[size]);
        std::snprintf(buf.get(), size, format.c_str(), args...);
        return std::string(buf.get(), buf.get() + size - 1);
    }

    //用于判断类型是否为 std::optional 的模板结构体
    template<typename T, typename Enable = void>
    struct is_optional : std::false_type {};

    template<typename T>
    struct is_optional<std::optional<T>> : std::true_type {};

    template<typename T>
    inline T node_as(const YAML::Node& node) {
        //移除引用
        if constexpr (std::is_same_v<std::remove_reference_t<T>, uint8_t>) {
            return static_cast<uint8_t >(node.as<uint32_t>());
        }
        else {
            return node.as<T>();
        }
    }

    template<typename T>
    inline auto cast(const T& v) {
        if constexpr (std::is_same_v<T, uint8_t>) {
            return static_cast<uint32_t >(v);
        }
        else {
            return v;
        }
    }

    //typename... T，表示可传入任意数量的类型参数。 std::size_t... I，表示包含 std::size_t 类型的参数包
    template<typename... T, std::size_t... I>
    inline std::tuple<T...> yaml_node_to_tuple(const YAML::Node& node, std::index_sequence<I...>) {
        return std::make_tuple(yaml_struct::node_as<T>(node[I])...);
    }

    template<typename... T, std::size_t... I>
    inline YAML::Node tuple_to_yaml_node(const std::tuple<T...>& tup, std::index_sequence<I...>) {
        YAML::Node node(YAML::NodeType::Sequence);
        (node.push_back(yaml_struct::cast(std::get<I>(tup))), ...);
        return node;
    }

    template<typename T>
    inline std::tuple<std::optional<std::string>, std::string > to_yaml(const T& obj) {
        try {
            YAML::Emitter emitter;
            emitter << YAML::Node{yaml_struct::cast(obj)};
            return std::make_tuple(emitter.c_str(), "");
        }
        catch (std::exception& e){
            return std::make_tuple(std::nullopt, string_format("Emitter to string: %s", e.what()));
        }
    }

    template<typename T, bool is_file = true>
    inline std::tuple<std::optional<T>, std::string > from_yaml(const std::string& str) {
        try {
            if constexpr (is_file) {
                const auto node = YAML::LoadFile(str);
                return std::make_tuple(yaml_struct::node_as<T>(node), "");
            }
            else {
                const auto node = YAML::Load(str);
                return std::make_tuple(yaml_struct::node_as<T>(node), "");
            }
        }
        catch (const YAML::BadFile& e) {
            return std::make_tuple(std::nullopt, string_format("%s not found or broken", str.c_str()));
        }
        catch (const std::exception& e) {
            return std::make_tuple(std::nullopt, string_format("on parsing %s -> %s", str.c_str(), e.what()));
        }
    }

    template<typename T>
    inline std::tuple<std::optional<T>, std::string > from_yaml_env(const std::string& str, const std::string& prefix) {
        YAML::Node node = YAML::LoadFile(str);
        for(YAML::iterator it = node.begin(); it != node.end(); it++) {
            std::string node_name = it->first.as<std::string>();
            std::transform(node_name.begin(), node_name.end(), node_name.begin(), ::toupper);

            auto environment_name = prefix + node_name;
            auto environment_content_ptr = std::getenv(environment_name.data());
            if(!environment_content_ptr) {
                continue;
            }
            std::string value{environment_content_ptr};
            it->second = YAML::Load(value);
        }

        return from_yaml<T, false>(YAML::Dump(node));
    }
}


namespace YAML
{
    template<typename K, typename V>
    struct convert<std::unordered_map<K,V>>
    {
        static bool decode(const Node& node, std::unordered_map<K,V>& rhs) {
            if(!node.IsMap()) {
                return false;
            }
            rhs.clear();
            for(auto& it : node) {
                rhs.emplace(it.first.template as<K>(), it.second.template as<V>());
            }
            return true;
        }

        static Node encode(const std::unordered_map<K,V>& rhs) {
            Node node(NodeType::Map);
            for(auto& [k, v] : rhs) {
                node.force_insert(::yaml_struct::cast(k), ::yaml_struct::cast(v));
            }
            return node;
        }
    };

    template<typename R, typename P>
    struct convert<std::chrono::duration<R, P>>
    {
        static bool decode(const Node& node, std::chrono::duration<R,P>& rhs) {
            if(!node.IsScalar()) {
                return false;
            }
            rhs = std::chrono::duration<R,P>(node.template as<R>());
            return true;
        }

        static Node encode(const std::chrono::duration<R,P>& rhs) {
            return Node(::yaml_struct::cast(rhs.count()));
        }
    };


    template <typename V>
    struct convert<std::unordered_set<V>> {
        static bool decode(const Node& node, std::unordered_set<V>& rhs) {
            if (!node.IsSequence())
                return false;
            rhs.clear();
            for (auto& it : node)
                rhs.emplace(it.template as<V>());
            return true;
        }

        static Node encode(const std::unordered_set<V>& rhs) {
            Node node(NodeType::Sequence);
            for (auto& it : rhs)
                node.push_back(Node(::yaml_struct::cast(it)));
            return node;
        }
    };

    template<typename T>
    struct convert<std::optional<T>>
    {
        static bool decode(const Node& node, std::optional<T>& rhs) {
            if(node.IsNull()) {
                rhs = std::nullopt;
            }
            else {
                rhs = ::yaml_struct::node_as<T>(node);
            }
            return true;
        }

        static Node encode(const std::optional<T>& rhs) {
            if(rhs.has_value()) {
                return Node(::yaml_struct::cast(rhs.value()));
            }
            else {
                return Node{};
            }
        }
    };


    template<typename... T>
    struct convert<std::tuple<T...>>
    {
        static bool decode(const Node& node, std::tuple<T...>& rhs) {
            if(!node.IsSequence() || (node.size() != sizeof...(T))) {
                return false;
            }
            rhs = ::yaml_struct::yaml_node_to_tuple<T...>(node, std::index_sequence_for<T...>{});
            return true;
        }

        static Node encode(const std::tuple<T...>& rhs) {
            return ::yaml_struct::tuple_to_yaml_node(rhs, std::index_sequence_for<T...>{});
        }
    };

    template<typename... T>
    struct convert<std::variant<T...>>
    {
        static bool decode(const Node& node, std::variant<T...>& rhs) {
            bool flag = false;
            ([&]() {
                if (flag)
                    return;
                try {
                    rhs = ::yaml_struct::node_as<T>(node);
                    flag = true;
                }
                catch (...) {

                }
            }(), ...);
            return flag;
        }

        static Node encode(const std::variant<T...>& rhs) {
            Node node;
            std::visit([&](auto& v) {
                node = Node(::yaml_struct::cast(v));
            }, rhs);
            return node;
        }
    };
}


#endif //WEBSERVER_YAMLUTIL_H
