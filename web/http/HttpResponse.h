//
// Created by 37496 on 2024/2/25.
//

#ifndef WEBSERVER_HTTPRESPONSE_H
#define WEBSERVER_HTTPRESPONSE_H

#include <string>
#include <unordered_map>
#include <string_view>
#include <map>
#include <filesystem>
#include "http/util/HttpTypeUtil.h"
#include "http/base/HttpHeader.h"
#include "http/base/HttpCookie.h"
#include "base/Noncopyable.h"
#include "base/Buffer.h"
#include "base/FileUtil.h"
#include "nlohmann/json.h"


namespace Tiny_muduo::Http
{
    class HttpResponse
    {
    public:

        explicit HttpResponse(bool close)
                : statusCode_(HttpStatusCode::OK),
                  closeConnection_(close),
                  version_(HttpVersion::HTTP_1_0),
                  contentLength_(-1),
                  isFileBody_(false),
                  keepAlive_(false),//todo
                  hasSession_(false)//todo
        {
        }

        void setStatusCode(HttpStatusCode code) { statusCode_ = code; }
        HttpStatusCode getStatusCode() const { return statusCode_; }
        void setHttpVersion(HttpVersion version) { version_ = version; }
        void setHttpResponseLine(HttpVersion version, HttpStatusCode code);

        bool closeConnection() const { return closeConnection_; }
        void setCloseConnection(bool on) { closeConnection_ = on; }

        void setKeepAlive(bool on);
        void setContentType(HttpContentType contentType);
        void setContentLength(size_t contentLength) ;
        void setCookie(const HttpCookie &cookie);
        void setCookie(const std::string &cookieStr);
        void setRedirect(const std::string &url);

        void addAtrribute(const std::string &key, const std::string &val);
        HttpHeader& getHttpHeader() { return header_; }

        void appendBody(const std::string& data) { body_.append(data); }
        void setBody(const std::string& body) { body_ = body; }

        void setFileBody(const std::string& filepath);
        void setHtmlBody(const std::string& filepath);
        void setPlainText(const std::string& content);
        void setJsonBody(nlohmann::json node);

        void appendToBuffer(Buffer* output) ;

    private:
        HttpStatusCode statusCode_;
        HttpVersion version_;
        HttpHeader header_;

        size_t contentLength_;

        bool isFileBody_;
        std::string file_path_;
        bool closeConnection_;
        std::string body_;

        bool keepAlive_;
        bool hasSession_;

    };

}


#endif //WEBSERVER_HTTPRESPONSE_H
