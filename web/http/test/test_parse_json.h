//
// Created by 37496 on 2024/2/26.
//

#ifndef WEBSERVER_TEST_PARSE_JSON_H
#define WEBSERVER_TEST_PARSE_JSON_H
#include "http/HttpRequest.h"
#include <iostream>
using namespace Tiny_muduo::Http;
using namespace Tiny_muduo;

int main()
{
    std::string str = "POST /json HTTP/1.1\r\n"
                      "Host: foo.com\r\n"
                      "Content-Type: application/json\r\n"
                      "Content-Length: 25\r\n"
                      "\r\n"
                      "{\"say\": \"hi\", \"to\":\"mom\"}";
    Buffer buf;
    buf.append(str);
    HttpRequest request(nullptr);
    request.parseRequest(&buf, TimeStamp::now());
    std::cout << request.getJson() << std::endl;

}
#endif //WEBSERVER_TEST_PARSE_JSON_H
