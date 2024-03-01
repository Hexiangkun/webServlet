//
// Created by 37496 on 2024/2/25.
//

#ifndef WEBSERVER_TEST_PARSEREQUESTHEADER_H
#define WEBSERVER_TEST_PARSEREQUESTHEADER_H

#include <string>
#include "http/HttpRequest.h"
#include "base/Buffer.h"
using namespace Tiny_muduo::Http;
using namespace Tiny_muduo;
int main()
{
    std::string str = "Host: 8.140.240.244:8000\r\n"
                      "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/121.0.0.0 Safari/537.36\r\n"
                      "Accept: image/avif,image/webp,image/apng,image/svg+xml,image/*,*/*;q=0.8\r\n"
                      "Accept-Encoding: gzip, deflate\r\n"
                      "Accept-Language: zh-CN,zh;q=0.9\r\n"
                      "Referer: http://8.140.240.244:8000/favicon.ico\r\n";
    Buffer::_ptr buf = std::make_shared<Buffer>();
    buf->append(str);
    HttpRequest request(nullptr);
    request.setBuffer(buf);
    request.parseRequestHeader();
}

#endif //WEBSERVER_TEST_PARSEREQUESTHEADER_H
