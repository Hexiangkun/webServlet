//
// Created by 37496 on 2024/2/23.
//

#ifndef WEBSERVER_HTTPREQUEST_H
#define WEBSERVER_HTTPREQUEST_H



#include "base/Buffer.h"
#include "http/util/HttpTypeUtil.h"
#include "nlohmann/json.h"
#include "base/TimeStamp.h"

namespace Tiny_muduo::Http
{

    class HttpRequest
            {
            public:
                enum Method
                {
                    kInvalid, kGet, kPost, kHead, kPut, kDelete
                };
                enum Version
                {
                    kUnknown, kHttp10, kHttp11
                };

                HttpRequest()
                        : method_(kInvalid),
                          version_(kUnknown)
                {
                }

                void setVersion(Version v)
                {
                    version_ = v;
                }

                Version getVersion() const
                { return version_; }

                bool setMethod(const char* start, const char* end)
                {
                    assert(method_ == kInvalid);
                    string m(start, end);
                    if (m == "GET")
                    {
                        method_ = kGet;
                    }
                    else if (m == "POST")
                    {
                        method_ = kPost;
                    }
                    else if (m == "HEAD")
                    {
                        method_ = kHead;
                    }
                    else if (m == "PUT")
                    {
                        method_ = kPut;
                    }
                    else if (m == "DELETE")
                    {
                        method_ = kDelete;
                    }
                    else
                    {
                        method_ = kInvalid;
                    }
                    return method_ != kInvalid;
                }

                Method method() const
                { return method_; }

                const char* methodString() const
                {
                    const char* result = "UNKNOWN";
                    switch(method_)
                    {
                        case kGet:
                            result = "GET";
                            break;
                        case kPost:
                            result = "POST";
                            break;
                        case kHead:
                            result = "HEAD";
                            break;
                        case kPut:
                            result = "PUT";
                            break;
                        case kDelete:
                            result = "DELETE";
                            break;
                        default:
                            break;
                    }
                    return result;
                }

                void setPath(const char* start, const char* end)
                {
                    path_.assign(start, end);
                }

                const string& path() const
                { return path_; }

                void setQuery(const char* start, const char* end)
                {
                    query_.assign(start, end);
                }

                const string& query() const
                { return query_; }

                void setReceiveTime(TimeStamp t)
                { receiveTime_ = t; }

                TimeStamp receiveTime() const
                { return receiveTime_; }

                void addHeader(const char* start, const char* colon, const char* end)
                {
                    string field(start, colon);
                    ++colon;
                    while (colon < end && isspace(*colon))
                    {
                        ++colon;
                    }
                    string value(colon, end);
                    while (!value.empty() && isspace(value[value.size()-1]))
                    {
                        value.resize(value.size()-1);
                    }
                    headers_[field] = value;
                }

                string getHeader(const string& field) const
                {
                    string result;
                    std::map<string, string>::const_iterator it = headers_.find(field);
                    if (it != headers_.end())
                    {
                        result = it->second;
                    }
                    return result;
                }

                const std::map<string, string>& headers() const
                { return headers_; }

                void swap(HttpRequest& that)
                {
                    std::swap(method_, that.method_);
                    std::swap(version_, that.version_);
                    path_.swap(that.path_);
                    query_.swap(that.query_);
                    receiveTime_.swap(that.receiveTime_);
                    headers_.swap(that.headers_);
                }

            private:
                Method method_;
                Version version_;
                string path_;
                string query_;
                TimeStamp receiveTime_;
                std::map<string, string> headers_;
            };


//    class HttpRequest
//    {
//    public:
//        typedef std::shared_ptr<HttpRequest> _ptr;
//        enum CHECK_STATE
//        {
//            CHECK_REQUESTLINE,
//            CHECK_HEADER,
//            CHECK_CONTENT,
//            CHECK_DONE,
//        };
//        enum RET_STATE
//        {
//            OK_REQUEST,
//            AGAIN_REQUEST,
//            BAD_REQUEST,
//        };
//        enum REASON{
//            BAD_REQUESTLINE = 1,
//            BAD_HEADER = 2,
//            BAD_CONTENT = 4,
//            EXCEED_REQUESTLINE = 8,
//            EXCEED_HEADER = 16,
//            NONE = 0,
//        };
//
//        HttpRequest(HttpSessionManager*);
//        RET_STATE parseRequest(Buffer* buffer, TimeStamp receiveTime);
//
//        HttpMethod getMethod() const { return method_; }
//        std::string getMethodStr() const { return HttpMethod2Str.at(method_); }
//
//        const HttpUrl& getUrl() const { return reqUrl_; }
//        const std::string getPath() const { return std::string(reqUrl_.getPath());}
//        HttpVersion getVersion() const { return version_; }
//        std::string getVersionStr() const { return HttpVersion2Str.at(version_); }
//
//        const HttpHeader& getHeader() const { return header_; }
//        const std::string& getBody() const { return body_; }
//
//        bool keepAlive() const { return keepAlive_; }
//        bool hasCookies() const { return hasCookies_; }
//        HttpContentType getContentType() const { return contentType_; }
//        const std::string& getAcceptEncoding() const { return acceptEncoding_; }
//        const HttpCookie& getCookie() const { return cookie_; }
//        size_t getContentLength() const { return contentLength_; }
//        const std::string& getTransferEncoding() const { return transferEncoding_; }
//
//        CHECK_STATE getState() const { return state_; }
//        RET_STATE getRetState() const { return retState_; }
//
//        void setReceiveTime(TimeStamp t) { receiveTime_ = t; }
//        TimeStamp receiveTime() const { return receiveTime_; }
//
//        bool complete() const { return state_ == CHECK_DONE; }
//        bool okRequest() const { return retState_ == OK_REQUEST; }
//
//        // 获取session
//        HttpSession::_ptr getSession(bool autoCreate = true);
//
//        nlohmann::json getJson() const { return json_; }
//
//    private:
//        RET_STATE parseRequestLine();
//        RET_STATE parseRequestHeader();
//        RET_STATE parseRequestContent();
//
//        void parseKeepAlive();
//        void parseContentType();
//        void parseAcceptEncoding();
//        void parseCookies();
//        void parseContentLength();
//        void parseTransferEncoding();
//        void parseSession();
//
//        RET_STATE parseChunked();
//    public:
//        HttpSessionManager *sessionManager_;
//        Buffer* data_;
//
//        CHECK_STATE state_;
//        RET_STATE retState_;
//        int reason_;
//
//        // 记录上一次数据解析结束的位置
//        size_t lastEnd_;
//        TimeStamp receiveTime_;
//        const size_t max_requestLine_len_;
//        const size_t max_requestHeader_len_;
//
//        HttpMethod method_;
//        HttpUrl reqUrl_;
//        HttpVersion version_;
//
//        HttpHeader header_;
//        HttpCookie cookie_;
//        HttpMultiPart multipart_;
//        std::string body_;
//        HttpForm form_;
//        nlohmann::json json_;
//
//        HttpSession::_ptr session_;
//
//        HttpContentType contentType_;
//        size_t contentLength_{};
//        std::string acceptEncoding_;
//        std::string transferEncoding_;
//        bool chunked_;
//
//        const size_t chunk_size_str_limit;
//        HttpChunkedState cur_chunkedState_;
//        int cur_chunk_size_;
//
//        bool keepAlive_;
//        bool compressed_;
//        bool hasContentType_;
//        bool hasContentLength_;
//        bool hasCookies_;
//        bool hasTransferEncoding_;
//    };

}




#endif //WEBSERVER_HTTPREQUEST_H
