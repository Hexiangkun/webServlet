//
// Created by 37496 on 2024/2/28.
//

#ifndef WEBSERVER_HTTPSERVICE_H
#define WEBSERVER_HTTPSERVICE_H

#include "http/HttpRequest.h"
#include "http/HttpResponse.h"

namespace Tiny_muduo::Http
{
    void Index(const HttpRequest& req, HttpResponse* resp);

    void Login(const HttpRequest& request, HttpResponse* response);

    void Register(const HttpRequest& request, HttpResponse* response);
}


#endif //WEBSERVER_HTTPSERVICE_H
