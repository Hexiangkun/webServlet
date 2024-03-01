//
// Created by 37496 on 2024/2/22.
//

#ifndef WEBSERVER_HTTPTYPEUTIL_H
#define WEBSERVER_HTTPTYPEUTIL_H


#include <unordered_map>
#include <string>

namespace Tiny_muduo::Http
{
    enum HttpStatusCode
    {
        CONTINUE = 100,
        OK = 200,
        PARTIAL_CONTENT = 206,
        MOVED_PERMANENTLY = 301,
        FOUND = 302,
        NOT_MODIFIED = 304,
        TEMPORARY_REDIRECT = 307,
        BAD_REQUEST = 400,
        UNAUTHORIZED = 401,
        FORBIDDEN = 403,
        NOT_FOUND = 404,
        METHOD_NOT_ALLOWED = 405,
        RANGE_NOT_SATISFIABLE = 416,
        INTERNAL_SERVER_ERROR = 500,
        SERVICE_UNAVAILABLE = 503,
        HTTP_VERSION_NOT_SUPPORTED = 505
    };

    inline const std::unordered_map<short, std::string> HttpStatusCode2Str = {
            {HttpStatusCode::CONTINUE, "Continue"},
            {HttpStatusCode::OK, "OK"},
            {HttpStatusCode::PARTIAL_CONTENT, "Partial Content"},
            {HttpStatusCode::MOVED_PERMANENTLY, "Moved Permanently"},
            {HttpStatusCode::FOUND, "Found"},
            {HttpStatusCode::NOT_MODIFIED, "Not Modified"},
            {HttpStatusCode::TEMPORARY_REDIRECT, "Temporary Redirect"},
            {HttpStatusCode::BAD_REQUEST, "Bad Request"},
            {HttpStatusCode::UNAUTHORIZED, "Unauthorized"},
            {HttpStatusCode::FORBIDDEN, "Forbidden"},
            {HttpStatusCode::NOT_FOUND, "Not Found"},
            {HttpStatusCode::METHOD_NOT_ALLOWED, "Method Not Allowed"},
            {HttpStatusCode::RANGE_NOT_SATISFIABLE, "Range Not Satisfiable"},
            {HttpStatusCode::INTERNAL_SERVER_ERROR, "Internal Server Error"},
            {HttpStatusCode::SERVICE_UNAVAILABLE, "Service Unavailable"},
            {HttpStatusCode::HTTP_VERSION_NOT_SUPPORTED, "HTTP Version Not Suppported"}
    };

    enum HttpChunkedState {
        NO_LEN,
        NO_DATA,
        DONE,
        BAD,
    };

    enum class HttpContentType {
        URLENCODED,
        MULTIPART,
        JSON,
        PLAIN,
        HTML,
        XML,
        XHTML,
        TXT,
        RTF,
        PDF,
        WORD,
        JPG,
        JPEG,
        PNG,
        GIF,
        BMP,
        AVI,
        MP4,
        WEBM,
        CSS,
        JS,
        UNKNOW,
    };

    inline const std::unordered_map<HttpContentType, std::string> HttpContentType2Str = {
            {HttpContentType::URLENCODED, "application/x-www-form-urlencoded"},
            {HttpContentType::MULTIPART, "multipart/form-data"},
            {HttpContentType::PLAIN, "text/plain"},
            {HttpContentType::HTML, "text/html; charset=utf-8"},
            {HttpContentType::XML, "text/xml"},
            {HttpContentType::XHTML, "application/xhtml+xml"},
            {HttpContentType::TXT, "text/plain"},
            {HttpContentType::RTF, "application/rtf"},
            {HttpContentType::PDF, "application/pdf"},
            {HttpContentType::WORD, "application/nsword"},
            {HttpContentType::JSON, "application/json"},
            {HttpContentType::JPEG, "image/jpeg"},
            {HttpContentType::JPG, "image/jpg"},
            {HttpContentType::PNG, "image/png"},
            {HttpContentType::GIF, "image/gif"},
            {HttpContentType::BMP, "image/bmp"},
            {HttpContentType::AVI, "video/x-msvideo"},
            {HttpContentType::MP4, "video/mp4"},
            {HttpContentType::WEBM, "video/webm"},
            {HttpContentType::CSS, "text/css"},
            {HttpContentType::JS, "text/javascript"},
            {HttpContentType::JSON, "application/json"},
    };

    inline const std::unordered_map<std::string, HttpContentType> Str2HttpContentType (
            [](){
                std::unordered_map<std::string, HttpContentType> tmp;
                for(auto& [k,v]:HttpContentType2Str) {
                    tmp[v]=k;
                }
                return tmp;
            }()
    );

    inline const std::unordered_map<std::string, std::string> Ext2HttpContentTypeStr {
            {".html", "text/html"},
            {".htm", "text/html"},
            {".css", "text/css"},
            {".csv", "text/csv"},
            {".txt", "text/plain"},

            {".woff", "font/woff"},
            {".woff2", "font/woff2"},
            {".ttf", "font/ttf"},
            {".otf", "font/otf"},

            {".bmp", "image/x-ms-bmp"},
            {".svg", "image/svg+xml"},
            {".svgz", "image/svg+xml"},
            {".gif", "image/gif"},
            {".jpg", "image/jpeg"},
            {".jpeg", "image/jpeg"},
            {".webp", "image/webp"},
            {".png", "image/png"},
            {".ico", "image/x-icon"},

            {".wav", "audio/wav"},
            {".weba", "audio/webm"},
            {".mp3", "audio/mpeg"},
            {".oga", "audio/ogg"},

            {".ogv", "video/ogg"},
            {".mpeg", "video/mpeg"},
            {".mp4", "video/mp4"},
            {".avi", "video/x-msvideo"},
            {".wmv", "video/x-ms-wmv"},
            {".webm", "video/webm"},
            {".flv", "video/x-flv"},
            {".m4v", "video/x-m4v"},

            {".7z", "application/x-7z-compressed"},
            {".rar", "application/x-rar-compressed"},
            {".sh", "application/x-sh"},
            {".rss", "application/rss+xml"},
            {".tar", "application/x-tar"},
            {".jar", "application/java-archive"},
            {".gz", "application/gzip"},
            {".zip", "application/zip"},
            {".xhtml", "application/xhtml+xml"},
            {".xml", "application/xml"},
            {".js", "text/javascript"},
            {".pdf", "application/pdf"},
            {".doc", "application/msword"},
            {".xls", "application/vnd.ms-excel"},
            {".ppt", "application/vnd.ms-powerpoint"},
            {".eot", "application/vnd.ms-fontobject"},
            {".json", "application/json"},
            {".bin", "application/octet-stream"},
            {".exe", "application/octet-stream"},
            {".dll", "application/octet-stream"},
            {".deb", "application/octet-stream"},
            {".iso", "application/octet-stream"},
            {".img", "application/octet-stream"},
            {".dmg", "application/octet-stream"},
            {".docx","application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
            {".xlsx","application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
            {".pptx","application/""vnd.openxmlformats-officedocument.presentationml.presentation"}
    };

    inline const std::unordered_map<std::string, HttpContentType> Ext2HttpContentType(
            [](){
                std::unordered_map<std::string, HttpContentType> tmp;
                for(auto& [k, v] : Ext2HttpContentTypeStr) {
                    auto it = Str2HttpContentType.find(v);
                    if(it != Str2HttpContentType.end()) {
                        tmp[k] = it->second;
                    }
                }
                return tmp;
            }()
    );

    inline const std::unordered_map<HttpContentType, std::string> HttpContentType2Ext(
            [](){
                std::unordered_map<HttpContentType, std::string> tmp;
                for(auto& [k, v] : Ext2HttpContentType) {
                    tmp[v] = k;
                }
                tmp[HttpContentType::HTML] = "html";
                return tmp;
            }()
    );

    enum class HttpMethod { GET = 0x01, POST = 0x02, HEAD = 0x03 };


    enum class HttpVersion {
        HTTP_1_0 = 0x10,
        HTTP_1_1 = 0x11,
        HTTP_2_0 = 0x20 /*not support HTTP 2.0*/
    };

    inline const std::unordered_map<HttpMethod, std::string> HttpMethod2Str{
            {HttpMethod::GET, "GET"},
            {HttpMethod::POST, "POST"},
            {HttpMethod::HEAD, "HEAD"},
    };

    inline const std::unordered_map<HttpVersion, std::string> HttpVersion2Str{
            {HttpVersion::HTTP_1_0, "HTTP/1.0"},
            {HttpVersion::HTTP_1_1, "HTTP/1.1"},
            {HttpVersion::HTTP_2_0, "HTTP/2.0"},
    };
}

#endif //WEBSERVER_HTTPTYPEUTIL_H
