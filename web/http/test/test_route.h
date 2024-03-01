//
// Created by 37496 on 2024/2/26.
//

#ifndef WEBSERVER_TEST_ROUTE_H
#define WEBSERVER_TEST_ROUTE_H

#include <regex>
#include <string>
#include <iostream>
#include <gtest/gtest.h>
#include "http/base/HttpRouter.h"
#include "http/HttpRequest.h"

using namespace Tiny_muduo::Http;

void test_regex_search()
{
    std::string route = "/say/[text]/to/[person]";
    auto def = routeStr2Definition(route);
    std::regex rr = def.route_regex;

    auto r1 = "/say/hi/to/mom";
    auto r2 = "/say/hi/to/mom/";
    auto r3 = "/say/83uu4/to/me";
    auto r4 = "/say/hi/to/";

    std::cout << std::regex_search(r1, rr) << std::endl;
    std::cout << std::regex_search(r2, rr) << std::endl;
    std::cout << std::regex_search(r3, rr) << std::endl;
    std::cout << std::regex_search(r4, rr) << std::endl;
}

void test_regex_search1()
{
    std::string route = "/say/[text]/to/[person]";
    auto def = routeStr2Definition(route);

    ASSERT_TRUE(def.params_names[0] == "text");
    ASSERT_TRUE(def.params_names[1] == "person");

    std::regex rr = def.route_regex;

    std::string r1 = "/say/hi/to/mom";
    std::smatch s;

    ASSERT_TRUE(std::regex_search(r1, s, rr));
    ASSERT_TRUE(s[1] == "hi");
    ASSERT_TRUE(s[2] == "mom");
}

void test_matchRoute()
{
    std::string route = "/";
    auto def = routeStr2Definition(route);
    std::string r1 = "/";
    std::string r2 = "/hi";

    ASSERT_TRUE(matchRoute(def, r1).has_value());
    ASSERT_TRUE(!matchRoute(def, r2).has_value());
}

void test_matchRoute1()
{
    std::string route = "/say/[text]/to/[person]";
    auto def = routeStr2Definition(route);
    std::string r1 = "/say/hi/to/mom";

    auto match = matchRoute(def, r1);
    ASSERT_TRUE(match.has_value());
    auto value = match.value();
    EXPECT_EQ(value["text"] , "hi");
    EXPECT_EQ(value["person"] , "mom");
}

void test_matchRequest()
{
    std::string route = "/say/[text]/to/[person]";
    auto def = routeStr2Definition(route);
    def.method = "GET";

    Tiny_muduo::Buffer buf;
    buf.append("GET /say/hi/to/mom?from=ahmed HTTP/1.0\r\n"
               "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7\r\n"
               "Accept-Encoding: gzip, deflate\r\n"
               "Accept-Language: zh-CN,zh;q=0.9\r\n"
               "Host: 8.140.240.244:8000\r\n"
               "Upgrade-Insecure-Requests: 1\r\n"
               "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/121.0.0.0 Safari/537.36\r\n"
               "\r\n");
    HttpRequest req(nullptr);
    req.parseRequest(&buf, Tiny_muduo::TimeStamp::now());

    auto match = matchRequest(def, req);
    ASSERT_TRUE(match.has_value());

    auto value = match.value();
    std::cout << value["text"] << std::endl << value["person"] << std::endl;
    EXPECT_EQ(value["text"] , "hi");
    EXPECT_EQ(value["person"] , "mom");
}

int main()
{
    test_matchRequest();
}

#endif //WEBSERVER_TEST_ROUTE_H
