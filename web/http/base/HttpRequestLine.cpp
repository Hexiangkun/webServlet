//
// Created by 37496 on 2024/2/23.
//

#include "HttpRequestLine.h"
#include "http/util/EncodeUtil.h"

namespace Tiny_muduo::Http
{
    HttpRequestLine::HttpRequestLine(bool decode)
                :_maxRequestLineLen(2048),
                _decode(decode),
                _method(HttpMethod::GET),
                _version(HttpVersion::HTTP_1_0)
    {

    }

    bool HttpRequestLine::parseRequestLine(const std::string& _requestLine) {
        std::string_view view(_requestLine);
        if(view.size() > _maxRequestLineLen){
            return false;
        }

        // 寻找行结束位置，没有就需要继续接收数据
        size_t end = view.find("\r\n");
        if(end == view.npos){
            if(view.size() && view[0] != 'G' && view[0] != 'P' && view[0] != 'H'){
                return false;
            }
            return false;
        }

        size_t i = 0;
        std::string_view line = view.substr(0, end);

        // 解析method
        if (line.compare(0, 3, "GET") == 0){
            _method = HttpMethod::GET;
            i += 4;
        }
        else if (line.compare(0, 4, "POST") == 0){
            _method = HttpMethod::POST;
            i += 5;
        }
        else if (line.compare(0, 4, "HEAD") == 0){
            _method = HttpMethod::HEAD;
            i += 5;
        }
        else{
            return false;
        }
        line.remove_prefix(i);

        // 解析url
        size_t pos = line.find_first_of(' ');
        if (pos == line.npos){
            return false;
        }
        std::string url = std::string(line.data(), pos);
        if(_decode) {
            _url = EncodeUtil::urlDecode(url);
        }
        else {
            _url = url;
        }
        if(_decode && !url.empty() && _url.empty()) {
            _state = PARSE_FATAL;
            return false;
        }
        else {
            _state = parseUrl();
        }
        if(_state == PARSE_FATAL) {
            return false;
        }

        line.remove_prefix(pos + 1);
        i += pos + 1;

        // 解析version
        if (line.compare("HTTP/1.0") == 0){
            _version = HttpVersion::HTTP_1_0;
        }
        else if (line.compare("HTTP/1.1") == 0){
            _version = HttpVersion::HTTP_1_1;
        }
        else if (line.compare("HTTP/2.0") == 0){
            _version = HttpVersion::HTTP_2_0;
        }
        else{
            return false;
        }
        return true;
    }

    HttpRequestLine::STATE HttpRequestLine::parseUrl() {
        std::string_view url_view(_url);
        size_t pos;

        if(url_view.compare(0, 4, "http") == 0) {
            pos = url_view.find("://");
            if(pos == url_view.npos) {
                return PARSE_FATAL;
            }
            std::string_view scheme_strview = url_view.substr(0, pos);
            if(scheme_strview == "http") {
                _scheme = HTTP;
            }
            else if(scheme_strview == "https") {
                _scheme = HTTPS;
            }
            else {
                return PARSE_FATAL;
            }

            url_view.remove_prefix(pos+3);
            if(url_view.size() < 2) {
                return PARSE_FATAL;
            }
            pos = url_view.find("/");
            if(pos == url_view.npos) {
                _host = std::string(url_view);
                _hostName = _host;
                _path = "/";
                return PARSE_SUCCESS;
            }
            else {
                _host = std::string(url_view.substr(0, pos));
                size_t pos1 = _host.find(":");
                if(pos1 != _host.npos) {
                    _hostName = _host.substr(0, pos1);
                    _port = _host.substr(pos1+1);
                }
                else {
                    _hostName = _host;
                }
            }
            url_view.remove_prefix(pos);

        }
        else {
            if(url_view.compare(0, 1, "/") != 0) {
                return PARSE_FATAL;
            }
        }

        pos = url_view.find("?");

        if(pos == url_view.npos) {
            _path = std::string(url_view);
            return PARSE_SUCCESS;
        }
        else {
            _path = std::string(url_view.substr(0, pos));
        }

        url_view.remove_prefix(pos+1);

        std::string_view query_view = url_view;
        std::string_view key, val;
        size_t i=0, j=0;

        for(i=0; i <query_view.size(); i++) {
            if(query_view[i] == '=') {
                key = query_view.substr(j, i-j);
                j = i+1;
            }
            else if(query_view[i] == '&') {
                val = query_view.substr(j, i-j);
                j = i+1;
                _query[std::string(key)] = std::string(val);
                key = "";
                val = "";
            }
        }
        if(!key.empty()) {
            val = query_view.substr(j, i-j);
            _query[std::string(key)] = std::string(val);
        }

        return PARSE_SUCCESS;
    }
}