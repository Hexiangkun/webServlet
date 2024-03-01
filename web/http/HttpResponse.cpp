//
// Created by 37496 on 2024/2/25.
//

#include "HttpResponse.h"
#include "config/Config.h"

namespace Tiny_muduo::Http
{
    static const std::string BASE_ROOT = config::GET_CONFIG<std::string>("resource", "/root/webserver/web/resource/");

    void HttpResponse::setHttpResponseLine(Tiny_muduo::Http::HttpVersion version, Tiny_muduo::Http::HttpStatusCode code)  {
        setStatusCode(code);
        setHttpVersion(version);
    }


    void HttpResponse::setKeepAlive(bool on) {
        if(on && !closeConnection_){
            header_.setConnection("Keep-Alive");
        }
        else{
            header_.setConnection("Close");
        }
    }

    void HttpResponse::setContentType(HttpContentType contentType) {
        header_.setContentType(HttpContentType2Str.at(contentType));
    }

    void HttpResponse::setCookie(const HttpCookie &cookie)  {
        header_.add("Set-Cookie", cookie.toString());
    }

    void HttpResponse::setCookie(const std::string &cookieStr){
        header_.add("Set-Cookie", cookieStr);
    }

    void HttpResponse::addAtrribute(const std::string &key, const std::string &val) {
        header_.add(key, val);
    }

    void HttpResponse::setContentLength(size_t contentLength) {
        contentLength_ = contentLength;
        header_.setContentLength(std::to_string(contentLength));
    }

    void HttpResponse::setRedirect(const std::string &url) {   // 重定向
        statusCode_ = HttpStatusCode::FOUND;
        header_.add("Location", url);
    }

    void HttpResponse::setFileBody(const std::string &filepath) {
        body_.clear();
        isFileBody_ = true;
        file_path_ = filepath;
        std::string content;
        readSmallFile(filepath.c_str(), 64*1024*1024,  content, nullptr, nullptr, nullptr);
        setBody(content);
    }

    void HttpResponse::setHtmlBody(const std::string &filepath) {
        header_.setContentType(HttpContentType2Str.at(HttpContentType::HTML));
        setFileBody(BASE_ROOT+filepath);
    }

    void HttpResponse::setPlainText(const std::string &content) {
        header_.add("Content-Type", HttpContentType2Str.at(HttpContentType::PLAIN));
        setBody(content);
    }

    void HttpResponse::setJsonBody(nlohmann::json node) {
        header_.add("Content-Type", HttpContentType2Str.at(HttpContentType::JSON));
        auto str = node.dump();
        setBody(str);
    }


    void HttpResponse::appendToBuffer(Buffer* output)
    {
        output->append(HttpVersion2Str.at(version_) + " ");
        output->append(std::to_string(statusCode_) + " ");
        output->append(HttpStatusCode2Str.at(statusCode_));
        output->append("\r\n");

        if (closeConnection_)
        {
            header_.setConnection("close");
        }
        else
        {
            if(contentLength_ == -1) {
                if(isFileBody_) {
                    contentLength_ = std::filesystem::file_size(file_path_);
                }
                else {
                    contentLength_ = body_.size();
                }
            }
            header_.setContentLength(std::to_string(contentLength_));
            setKeepAlive(true);
        }

        for(const auto& header : header_.getHeaders()) {
            output->append(header.first);
            output->append(": ");
            output->append(header.second);
            output->append("\r\n");
        }

        output->append("\r\n");
        output->append(body_);
    }


}