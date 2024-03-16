//
// Created by 37496 on 2024/2/25.
//
#include "HttpContext.h"

namespace Tiny_muduo::Http
{
    bool HttpContext::parseRequest(Tiny_muduo::Buffer *buf, Tiny_muduo::TimeStamp receiveTime) {
        auto ret = m_request->parseRequest(buf, receiveTime);
        return m_request->complete();
    }
}