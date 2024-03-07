//
// Created by 37496 on 2024/2/23.
//

#ifndef WEBSERVER_TEST_HTTPURL_H
#define WEBSERVER_TEST_HTTPURL_H

#include <iostream>
#include <memory>
#include "web/base/Buffer.h"
#include "http/base/HttpRequestLine.h"
using namespace Tiny_muduo::Http;

void print_parseHttpUrl(Tiny_muduo::Http::HttpUrl &url){
    std::cout << "=============HttpUrl-query=============" << std::endl;
    std::cout << url.getUrl() << std::endl;
    std::cout << url.getScheme() << std::endl;
    std::cout << url.getHost() << std::endl;
    std::cout << url.getHostName() << ":" << url.getPort() << std::endl;
    std::cout << url.getPath() << std::endl;
    for(auto [k,v] : url.getQuery()){
        std::cout << k << ": " << v << std::endl;
    }
}

void test_httpurl()
{
    const char *encodedUrl = "https://translate.google.cn/?sl=zh-CN&tl=en&text=%E9%92%A6%E4%BD%A9&op=translate";
    HttpUrl url(encodedUrl);
    print_parseHttpUrl(url);

    const char *encodedUrl1 = "https://static.kancloud.cn/digest/understandingnginx/202605";
    HttpUrl url1(encodedUrl1);
    print_parseHttpUrl(url1);

    const char *encodedUrl2 = "https://192.168.1.10:8888/digest/understandingnginx/202605";
    HttpUrl url2(encodedUrl2);
    print_parseHttpUrl(url2);
}


#endif //WEBSERVER_TEST_HTTPURL_H
