//
// Created by 37496 on 2024/2/23.
//

#ifndef WEBSERVER_HTTPREQUEST_H
#define WEBSERVER_HTTPREQUEST_H

#include "nlohmann/json.h"
#include "base/TimeStamp.h"
#include "base/Buffer.h"
#include "http/util/HttpTypeUtil.h"
#include "http/base/HttpRequestLine.h"
#include "http/base/HttpHeader.h"
#include "http/base/HttpCookie.h"
#include "http/base/HttpMultiPart.h"
#include "http/base/HttpSession.h"


namespace Tiny_muduo::Http
{
    class HttpRequest
    {
    public:
        typedef std::shared_ptr<HttpRequest> _ptr;
        enum CHECK_STATE
        {
            CHECK_REQUESTLINE,
            CHECK_HEADER,
            CHECK_CONTENT,
            CHECK_DONE,
        };
        enum RET_STATE
        {
            OK_REQUEST,
            AGAIN_REQUEST,
            BAD_REQUEST,
        };
        enum REASON{
            BAD_REQUESTLINE = 1,
            BAD_HEADER = 2,
            BAD_CONTENT = 4,
            EXCEED_REQUESTLINE = 8,
            EXCEED_HEADER = 16,
            NONE = 0,
        };

        HttpRequest();
        RET_STATE parseRequest(Buffer* buffer, TimeStamp receiveTime);

        const HttpRequestLine& getRequestLine() const { return requestLine_; }

        HttpMethod getMethod() const { return requestLine_.getMethod(); }
        const std::string getMethodStr() const { return requestLine_.getMethodStr(); }
        const std::string& getPath() const { return requestLine_.getPath(); }
        HttpVersion getVersion() const { return requestLine_.getVersion(); }

        const HttpHeader& getHeader() const { return header_; }
        const std::string& getBody() const { return body_; }

        bool keepAlive() const { return keepAlive_; }
        bool hasCookies() const { return hasCookies_; }
        HttpContentType getContentType() const { return contentType_; }
        const std::string& getAcceptEncoding() const { return acceptEncoding_; }
        const HttpCookie& getCookie() const { return cookie_; }
        size_t getContentLength() const { return contentLength_; }
        const std::string& getTransferEncoding() const { return transferEncoding_; }

        CHECK_STATE getState() const { return state_; }
        RET_STATE getRetState() const { return retState_; }

        void setReceiveTime(TimeStamp t) { receiveTime_ = t; }
        TimeStamp receiveTime() const { return receiveTime_; }
        void setHttpSessionManager(HttpSessionManager* sm) { sessionManager_ = sm; }

        bool complete() const { return state_ == CHECK_DONE; }
        bool okRequest() const { return retState_ == OK_REQUEST; }

        // 获取session
        HttpSession::_ptr getSession(bool autoCreate = true);

        nlohmann::json getJson() const { return json_; }

        const HttpForm& getForm() const { return form_; }
        const HttpMultiPart& getMultiPart() const { return multipart_; }

    private:
        RET_STATE parseRequestLine();
        RET_STATE parseRequestHeader();
        RET_STATE parseRequestContent();

        void parseKeepAlive();
        void parseContentType();
        void parseAcceptEncoding();
        void parseCookies();
        void parseContentLength();
        void parseTransferEncoding();
        void parseSession();

        RET_STATE parseChunked();
    public:
        HttpSessionManager *sessionManager_;
        Buffer* data_;

        CHECK_STATE state_;
        RET_STATE retState_;
        int reason_;

        TimeStamp receiveTime_;
        const size_t max_requestHeader_len_;

        HttpRequestLine requestLine_;

        HttpHeader header_;
        HttpCookie cookie_;


        HttpMultiPart multipart_;
        std::string body_;
        HttpForm form_;
        nlohmann::json json_;

        HttpSession::_ptr session_;

        HttpContentType contentType_;
        size_t contentLength_{};
        std::string acceptEncoding_;
        std::string transferEncoding_;
        bool chunked_;

        const size_t chunk_size_str_limit;
        HttpChunkedState cur_chunkedState_;
        int cur_chunk_size_;

        bool keepAlive_;
        bool compressed_;
        bool hasContentType_;
        bool hasContentLength_;
        bool hasCookies_;
        bool hasTransferEncoding_;

    };
}


#endif //WEBSERVER_HTTPREQUEST_H
