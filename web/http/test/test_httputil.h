//
// Created by 37496 on 2024/2/22.
//

#ifndef WEBSERVER_TEST_HTTPUTIL_H
#define WEBSERVER_TEST_HTTPUTIL_H


#include <iostream>
#include "web/http/util/HttpUtil.h"
#include "web/http/util/HttpMap.h"
#include "http/util/EncodeUtil.h"
using namespace Tiny_muduo::Http;

void print_parseHttpForm(HttpForm &form){
    std::cout << "=============HttpForm=============" << std::endl;
    for(auto [k,v] : form){
        std::cout << k << ": " << v << std::endl;
    }
}

void test_parsekeyvalue_urldecode()
{
    const char *url = "key1=你好&key2=@nihao";

    std::string decoded_url = EncodeUtil::urlDecode(url);
    HttpForm form;
    parseKeyValue(decoded_url, "=", "&", form);
    print_parseHttpForm(form);
}

#endif //WEBSERVER_TEST_HTTPUTIL_H
