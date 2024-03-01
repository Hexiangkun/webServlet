//
// Created by 37496 on 2024/2/22.
//

#ifndef WEBSERVER_TEST_HTTPMULTIPART_H
#define WEBSERVER_TEST_HTTPMULTIPART_H

#include <string>
#include <iostream>
#include "http/base/HttpMultiPart.h"

void test_httpmultipart()
{
    std::string data = "------WebKitFormBoundaryKPjN0GYtWEjAni5F\r\n"
                       "Content-Disposition: form-data; name=\"username\"\r\n"
                       "Content-Type: multipart/form-data\r\n"
                       "\r\n"
                       "130533193203240022\r\n"
                       "------WebKitFormBoundaryKPjN0GYtWEjAni5F\r\n"
                       "Content-Disposition: form-data; name=\"password\"\r\n"
                       "Content-Type: multipart/form-data\r\n"
                       "\r\n"
                       "qwerqwer\r\n"
                       "------WebKitFormBoundaryKPjN0GYtWEjAni5F\r\n"
                       "Content-Disposition: form-data; name=\"captchaId\"\r\n"
                       "Content-Type: multipart/form-data\r\n"
                       "\r\n"
                       "img_captcha_7d96b3cd-f873-4c36-8986-584952e38f20\r\n"
                       "------WebKitFormBoundaryKPjN0GYtWEjAni5F\r\n"
                       "Content-Disposition: form-data; name=\"captchaWord\"\r\n"
                       "Content-Type: multipart/form-data\r\n"
                       "\r\n"
                       "rdh5\r\n"
                       "------WebKitFormBoundaryKPjN0GYtWEjAni5F\r\n"
                       "Content-Disposition: form-data; name=\"_csrf\"\r\n"
                       "Content-Type: multipart/form-data\r\n"
                       "\r\n"
                       "200ea95d-90e9-4789-9e0b-435a6dd8b57b\r\n"
                       "------WebKitFormBoundaryKPjN0GYtWEjAni5F--\r\n";

    std::cout << data << std::endl;

    Tiny_muduo::Http::HttpMultiPart httpMultiPart;
    httpMultiPart.setBoundary("----WebKitFormBoundaryKPjN0GYtWEjAni5F");
    httpMultiPart.parse(std::string_view(data));
}

#endif //WEBSERVER_TEST_HTTPMULTIPART_H
