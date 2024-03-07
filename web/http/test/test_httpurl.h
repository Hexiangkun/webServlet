//
// Created by 37496 on 2024/2/23.
//


#include <iostream>
#include <memory>
#include "web/base/Buffer.h"
#include "http/base/HttpRequestLine.h"
using namespace Tiny_muduo::Http;

void print_parseHttpUrl(Tiny_muduo::Http::HttpRequestLine &url){
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
    const char *encodedUrl = "GET https://translate.google.cn/?sl=zh-CN&tl=en&text=%E9%92%A6%E4%BD%A9&op=translate HTTP/1.0\r\n";
    HttpRequestLine url;
    url.parseRequestLine(encodedUrl);
    print_parseHttpUrl(url);

    const char *encodedUrl1 = "GET https://static.kancloud.cn/digest/understandingnginx/202605 HTTP/1.0\r\n";
    HttpRequestLine url1;
    url1.parseRequestLine(encodedUrl1);
    print_parseHttpUrl(url1);

    const char *encodedUrl2 = "POST https://192.168.1.10:8888/digest/understandingnginx/202605 HTTP/1.1\r\n";
    HttpRequestLine url2;
    url2.parseRequestLine(encodedUrl2);
    print_parseHttpUrl(url2);
}

int main()
{
    test_httpurl();

}

