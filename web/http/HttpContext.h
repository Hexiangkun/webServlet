//
// Created by 37496 on 2024/2/25.
//

#ifndef WEBSERVER_HTTPCONTEXT_H
#define WEBSERVER_HTTPCONTEXT_H

#include "HttpRequest.h"

namespace Tiny_muduo::Http
{

    class HttpContext
    {
    public:
        HttpContext():m_request(std::make_shared<HttpRequest>(nullptr))
        {
        }

        const HttpRequest::_ptr request() const { return m_request; }
        HttpRequest::_ptr request() { return m_request; }

        bool parseRequest(Buffer* buf, TimeStamp receiveTime);

        bool gotAll() const { return m_request->complete(); }
        void reset() {
            HttpRequest::_ptr request1 = std::make_shared<HttpRequest>(nullptr);
            m_request.swap(request1);
            request1 = nullptr;
        }
    private:
        HttpRequest::_ptr m_request;
    };
}
#endif //WEBSERVER_HTTPCONTEXT_H
