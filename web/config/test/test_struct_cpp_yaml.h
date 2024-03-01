//
// Created by 37496 on 2024/2/4.
//

#ifndef WEBSERVER_TEST_STRUCT_CPP_YAML_H
#define WEBSERVER_TEST_STRUCT_CPP_YAML_H

#include <iostream>
#include <cassert>
#include <tuple>
#include <vector>
#include <string>
#include <optional>
#include <variant>
#include <fstream>
#include "yaml/struct_cpp_yaml.h"
#include "yaml/tostring.h"

enum class AccountType : uint8_t {
    Personal = 1,
    Company = 2,
};
YCS_ADD_ENUM(AccountType, Personal, Company)

std::string to_string(const AccountType& type) {
    if (type == AccountType::Personal)
        return "Personal";
    if (type == AccountType::Company)
        return "Company";
    throw std::bad_cast();
}

struct AccountInfo {
    bool flag;
    std::string name;
    std::unordered_map<std::string, int> address;
    std::optional<std::string> num;
    std::chrono::milliseconds msec;
    std::tuple<std::string, uint8_t> tuple;
    std::unordered_map<std::string, std::tuple<std::string, int32_t>> map_tuple;
};
YCS_ADD_STRUCT(AccountInfo, flag, name, address, num, msec, tuple, map_tuple)

std::string to_string(const AccountInfo& aif) {
    std::stringstream ss;
    ss << "AccountInfo"
       << " name=" << aif.name << " address=" << hxk_muduo::to_string(aif.address) << " num=" << hxk_muduo::to_string(aif.num)
       << " msec=" << aif.msec.count() << " tuple=" << hxk_muduo::to_string(aif.tuple) << " map_tuple=" << hxk_muduo::to_string(aif.map_tuple)
       << " flag=" << hxk_muduo::to_string(aif.flag);
    return ss.str();
}

struct DefaultTest {
    bool flag{false};
    std::string name{"hello world"};
    std::unordered_map<std::string, int> address{{"127.0.0.1", 8856}};
    std::optional<std::string> num{"90"};
    std::chrono::milliseconds msec{56};
    std::tuple<std::string, uint8_t> tuple{std::make_tuple("tuple", 58)};
    std::unordered_map<std::string, std::tuple<std::string, int32_t>> map_tuple{{"test-default", std::make_tuple("001", 23)}};
    AccountType account_type{AccountType::Company};
};
YCS_ADD_STRUCT(DefaultTest, flag, name, address, num, msec, tuple, map_tuple, account_type)

std::string to_string(const DefaultTest& aif) {
    std::stringstream ss;
    ss << "DefaultTest"
       << " name=" << aif.name << " address=" << hxk_muduo::to_string(aif.address) << " num=" << hxk_muduo::to_string(aif.num)
       << " msec=" << aif.msec.count() << " tuple=" << hxk_muduo::to_string(aif.tuple) << " map_tuple=" << hxk_muduo::to_string(aif.map_tuple)
       << " flag=" << hxk_muduo::to_string(aif.flag);
    return ss.str();
}

struct Config {
    char ch;
    double price;
    int16_t count;
    std::string content;
    std::unordered_map<std::string, std::string> map;
    AccountInfo account_info;
    std::vector<std::string> vec;
    std::unordered_set<uint8_t> set_vec;
    AccountType account_type;
    std::variant<int, std::vector<int>> v1;
    DefaultTest default_test;
    std::string default_str{"hello default"};
    std::optional<std::string> default_opt{std::nullopt};
    std::vector<std::string> ips;
};
YCS_ADD_STRUCT(Config, ch, price, count, content, map, account_info, vec, set_vec, account_type, v1, default_test, default_str, default_opt, ips)

std::string to_string(const Config& cfg) {
    std::stringstream ss;
    ss << "Config"
       << " price=" << std::to_string(cfg.price) << " count=" << (int32_t)cfg.count
       << " content=" << cfg.content << " map=" << hxk_muduo::to_string(cfg.map) << " account_info=" << to_string(cfg.account_info)
       << " vec=" << hxk_muduo::to_string(cfg.vec) << " set_vec=" << hxk_muduo::to_string(cfg.set_vec) << " account_type=" << to_string(cfg.account_type)
       << " ch=" << hxk_muduo::to_string(cfg.ch) << " v1=" << hxk_muduo::to_string(std::get<1>(cfg.v1)) << " default_test=" << to_string(cfg.default_test)
       << " default_str=" << hxk_muduo::to_string(cfg.default_str) << " default_opt=" << hxk_muduo::to_string(cfg.default_opt);
    return ss.str();

}

namespace test
{
    void test_string_format()
    {
        // 测试用例1
        std::string result1 = ::yaml_struct::string_format("Hello, %s!", "World");
        assert(result1 == "Hello, World!");

        // 测试用例2
        int number = 42;
        std::string result2 = ::yaml_struct::string_format("The answer is %d.", number);
        assert(result2 == "The answer is 42.");

        // 测试用例3
        double pi = 3.14159;
        std::string result3 = ::yaml_struct::string_format("Pi is approximately %.2f.", pi);
        assert(result3 == "Pi is approximately 3.14.");

    }

    void test_yaml_node_to_tuple()
    {
        YAML::Node node(YAML::NodeType::Sequence);
        node.push_back(YAML::Node("hello"));
        node.push_back(YAML::Node(42));
        node.push_back(YAML::Node(true));

        auto tuple = ::yaml_struct::yaml_node_to_tuple<std::string, int, bool>(node, std::index_sequence_for<std::string, int, bool>());

        assert(std::get<0>(tuple) == "hello");
        assert(std::get<1>(tuple) == 42);
        assert(std::get<2>(tuple) == true);
    }

    void test_tuple_to_yaml_node()
    {
        std::tuple<int, double, std::string> tuple{1, 2.1, "hello world"};
        auto node = ::yaml_struct::tuple_to_yaml_node(tuple, std::index_sequence_for<int, double, std::string>{});
        assert(node.IsSequence() && node.size() == 3);
        assert(node[0].IsScalar() && node[0].as<int>() == 1);
        assert(node[1].IsScalar() && node[1].as<double>() == 2.1);
        assert(node[2].IsScalar() && node[2].as<std::string>() == "hello world");
    }


    void test_to_yaml()
    {
        std::string expected_yaml = R"(a: 1
                                        b: 2.0
                                        c: true
                                        d: foo
                                        e: ~
                                        )";
        YAML::Node node;
        node["a"] = 1;
        node["b"] = 2.0;
        node["c"] = true;
        node["d"] = "foo";
        node["e"] = YAML::Node(YAML::NodeType::Null);

        auto [yl, error] = ::yaml_struct::to_yaml(node);

        assert(expected_yaml == *yl);
        assert(""== error);
    }



    void test_from_yaml()
    {
        auto [cfg, error] = yaml_struct::from_yaml<Config>("/root/webserver/config/test/test_config.yaml");
        if(cfg) {
            std::cout << cfg->count << std::endl;
            std::cout << to_string(cfg.value()) << std::endl;
        }
        else {
            std::cout << error << std::endl;
        }
    }

}

#endif //WEBSERVER_TEST_STRUCT_CPP_YAML_H
