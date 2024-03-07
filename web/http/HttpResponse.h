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
#include "config/Config.h"

namespace Tiny_muduo::Http
{
    class HttpResponse
    {
    public:

        explicit HttpResponse(bool close, HttpVersion version)
                : statusCode_(HttpStatusCode::OK),
                 version_(version),
                 closeConnection_(close),
                 root_path_(config::GET_CONFIG<std::string>("resource.path", "/root/resources")),
                 contentLength_(-1),
                 isFileBody_(false),
                 keepAlive_(false),//todo
                 hasSession_(false),//todo
                 fileFd_(-1),
                 fileLen_(0)
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

        void setFile(const std::string& filepath);
        void setFileBody(const std::string& filepath);
        void setHtmlBody(const std::string& filepath);
        void setPlainText(const std::string& content);
        void setJsonBody(nlohmann::json node);

        bool needSendFile() const { return fileFd_ != -1; }
        int getFileFd() const { return fileFd_; }
        void setFileFd(const int fd) { fileFd_ = fd; }
        off64_t getFileLen() const { return fileLen_; }
        void setFileLen(const off64_t len) { fileLen_ = len; }

        const std::string& getRootPath() const { return root_path_; }

        void appendToBuffer(Buffer* output) ;

    private:
        HttpStatusCode statusCode_;
        HttpVersion version_;
        HttpHeader header_;

        size_t contentLength_;

        bool isFileBody_;
        std::string root_path_;         //资源目录

        bool closeConnection_;          //是否关闭连接

        std::string body_;              //消息体

        bool keepAlive_;                //todo
        bool hasSession_;               //todo

        int fileFd_;        //传输文件时，文件描述符
        off64_t fileLen_;   //文件大小
    };

}


#endif //WEBSERVER_HTTPRESPONSE_H
