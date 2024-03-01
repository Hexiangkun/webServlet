//
// Created by 37496 on 2024/2/22.
//

#ifndef WEBSERVER_TEST_HTTPCOOKIE_H
#define WEBSERVER_TEST_HTTPCOOKIE_H

#include <iostream>
#include "http/base/HttpCookie.h"

using namespace Tiny_muduo::Http;

void print_parseHttpCookies(HttpCookie &cookies){
    std::cout << cookies.toString() << std::endl;
}


void test_httpcookie()
{
    const char *cookies_str = "reg_fb_gate=deleted; Expires=Thu, 01 Jan 1970 00:00:01 GMT; Path=/; Domain=.example.com; HttpOnly; name=lfr";
    HttpCookie cookies(cookies_str);
    print_parseHttpCookies(cookies);
}

#endif //WEBSERVER_TEST_HTTPCOOKIE_H
