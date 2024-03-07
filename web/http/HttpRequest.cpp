//
// Created by 37496 on 2024/2/23.
//

#include "web/http/HttpRequest.h"
#include "log/Log.h"
#include "http/util/HttpUtil.h"

namespace Tiny_muduo::Http
{
    HttpRequest::HttpRequest(HttpSessionManager *sm)
            :sessionManager_(sm)
            ,state_(CHECK_REQUESTLINE)
            ,reason_(NONE)
            ,max_requestHeader_len_(8192)
            ,chunked_(false)
            ,chunk_size_str_limit(5) // 16^5
            ,cur_chunkedState_(HttpChunkedState::NO_LEN)
            ,cur_chunk_size_(0)
            ,keepAlive_(false)
            ,compressed_(false)
            ,hasContentType_(false)
            ,hasContentLength_(false)
            ,hasCookies_(false)
            ,hasTransferEncoding_(false)
    {
    }

    HttpRequest::RET_STATE HttpRequest::parseRequest(Buffer* buffer, TimeStamp receiveTime){
        data_ = buffer;
        while(true){
            switch (state_)
            {
                case CHECK_REQUESTLINE:
                    retState_ = parseRequestLine();
                    if(retState_ == OK_REQUEST){
                        receiveTime_ = receiveTime;
                        state_ = CHECK_HEADER;
                    }
                    else{
                        reason_ = BAD_REQUESTLINE;
                    }
                    break;
                case CHECK_HEADER:
                    retState_ = parseRequestHeader();
                    if(retState_ == OK_REQUEST){
                        state_ = CHECK_CONTENT;
                    }
                    else {
                        reason_ = BAD_HEADER;
                    }
                    break;
                case CHECK_CONTENT:
                    retState_ = parseRequestContent();
                    if(retState_ == OK_REQUEST){
                        state_ = CHECK_DONE;
                    }
                    else {
                        reason_ = BAD_CONTENT;
                    }
                    break;
                default:
                    break;
            }

            if(retState_ != OK_REQUEST || state_ == CHECK_DONE){
                break;
            }
        }
        data_ = nullptr;
        return retState_;
    }

    HttpRequest::RET_STATE HttpRequest::parseRequestLine(){
        // 超出最大限制，结束
        const char* pos_crlf = data_->findCRLF(data_->peek());
        if(pos_crlf == nullptr) {
            return AGAIN_REQUEST;
        }

        std::string view(data_->peek(), pos_crlf-data_->peek()+2);
        data_->retrieve(pos_crlf-data_->peek()+2);
        bool res = requestLine_.parseRequestLine(view);
        if(res) {
            return OK_REQUEST;
        }
        return BAD_REQUEST;
    }


    HttpRequest::RET_STATE HttpRequest::parseRequestHeader(){
        const char* pos_2crlf = data_->find2CRLF(data_->peek());
        if(pos_2crlf== nullptr) {
            return AGAIN_REQUEST;
        }
        std::string_view header(data_->peek(), pos_2crlf-data_->peek()+4);
        data_->retrieve(pos_2crlf-data_->peek()+4);
        std::string_view line_header;
        while (header.size()) {
            // 碰到空行，结束
            if (header.size() >= 2 && header[0] == '\r' && header[1] == '\n') {
                /* 解析请求头的内容 */
                parseKeepAlive();
                parseAcceptEncoding();
                parseContentType();
                parseCookies();
                parseContentLength();
                parseTransferEncoding();
                parseSession();
                return OK_REQUEST;
            }
            // 超出最大限制，结束
            if(header_.size() > max_requestHeader_len_){
                reason_ |= EXCEED_HEADER;
                return BAD_REQUEST;
            }

            // 寻找行结束位置，没有就需要继续接收数据
            size_t crlf_pos = header.find("\r\n");
            if (crlf_pos == header.npos)
                break;

            line_header = header.substr(0, crlf_pos);

            size_t pos2 = line_header.find_first_of(":");
            if (pos2 == line_header.npos)
                return BAD_REQUEST;

            std::string_view key = line_header.substr(0, pos2);
            std::string_view value = "";

            /* 处理冒号后没有空格和有多个空格的情况 */
            size_t pos3 = line_header.find_first_of(" ", pos2);
            if(pos3 == line_header.npos){
                pos3 = pos2+1;
            }
            size_t pos4 = line_header.find_first_not_of(" ", pos3);
            if(pos4 != line_header.npos){
                value = line_header.substr(pos4);
            }

            // std::string_view value = line_header.substr(pos2 + 2, crlf_pos - pos2 - 2);
            header_.add(std::string(key.data(), key.size()),
                        std::string(value.data(), value.size()));

            header.remove_prefix(crlf_pos + 2);
        }

        return AGAIN_REQUEST;
    }



    HttpRequest::RET_STATE HttpRequest::parseRequestContent(){
        /* 分块传输要咋搞？卧槽 */
        if(hasTransferEncoding_){
            if(chunked_){
                auto ret = parseChunked();
                if(ret != OK_REQUEST){
                    return ret;
                }
            }
            else{
                // 不支持其他编码
                LOG_ERROR << "unsupport type other than chunked";
                return BAD_REQUEST;
            }
        }
            // 根据Content-Length判断数据是否读完
        else if(hasContentLength_){
            if(contentLength_ > body_.size() + data_->readableBytes()){
                return AGAIN_REQUEST;
            }
            else{
                size_t accept_size = contentLength_ - body_.size();
                body_.append(data_->peek(), accept_size);
                data_->retrieve(accept_size);
            }
        }
        else if(getMethod() == HttpMethod::GET || getMethod() == HttpMethod::HEAD){
            /* 空消息体，啥也不干 */
            return OK_REQUEST;
        }
            // 无法确定内容的末尾，终止交易！！！
        else{
            LOG_ERROR << "Unable to determine where the content ends";
            return BAD_REQUEST;
        }

        if(hasContentType_){
            if(contentType_ == HttpContentType::MULTIPART){
                std::string content_type = header_.getContentType().value();
                std::string find_str = "boundary=";
                std::size_t idx = content_type.find(find_str);
                if (idx == std::string::npos) return BAD_REQUEST; // 没找到boundary信息 报文有误
                std::string boundary(content_type.begin() + idx + find_str.size(), content_type.end());
                multipart_.setBoundary(boundary);
                std::string eob = "--" + multipart_.getBoundary() + "--\r\n";
                // equal to end_with
                if (body_.size() > eob.size() &&
                    !body_.compare(body_.size()-eob.size(), eob.size(), eob) == 0){
                    return AGAIN_REQUEST;
                }
                std::string_view view = body_;
                multipart_.parse(view);
            }
            else if(contentType_ == HttpContentType::URLENCODED){
                parseKeyValue(body_, "=", "&", form_, true);
            }
            else if(contentType_ == HttpContentType::PLAIN){
                parseKeyValue(body_, "=", "&", form_, false);
            }
            else if(contentType_ == HttpContentType::JSON){
                json_ = nlohmann::json::parse(body_);
            }
        }

        return OK_REQUEST;
    }




    void HttpRequest::parseKeepAlive(){
        // HTTP1.0以上的协议默认开启长连接
        if(getVersion() != HttpVersion::HTTP_1_0){
            keepAlive_ = true;
        }
        auto ret = header_.get("Connection");
        if(ret.exist()){
            std::string val = toLowers(ret.value());
            if(val == "keep-alive"){
                keepAlive_ = true;
            }
            else if(val == "close"){
                keepAlive_ = false;
            }
            else{
                keepAlive_ = false;
            }
        }
    }

    void HttpRequest::parseContentType(){
        auto ret = header_.get("Content-Type");
        if(ret.exist()){
            hasContentType_ = true;
            std::string val = toLowers(ret.value());
            if(val.compare(0, 33, "application/x-www-form-urlencoded") == 0){
                contentType_ = HttpContentType::URLENCODED;
            }
            else if(val.compare(0, 19, "multipart/form-data") == 0){
                contentType_ = HttpContentType::MULTIPART;
            }
            else if(val.compare(0, 10,"text/plain") == 0){
                contentType_ = HttpContentType::PLAIN;
            }
            else if(val.compare(0, 9, "text/html") == 0){
                contentType_ = HttpContentType::HTML;
            }
            else if(val.compare(0, 16, "application/json") == 0){
                contentType_ = HttpContentType::JSON;
            }
            else{
                contentType_ = HttpContentType::UNKNOW;
                LOG_ERROR << "unsupport Content-Type";
            }
        }
    }

    void HttpRequest::parseAcceptEncoding(){
        auto ret = header_.get("Accept-Encoding");
        if(ret.exist()){
            acceptEncoding_ = ret.value();
            compressed_ = ret.value().find("gzip") != std::string::npos;
        }
    }

    void HttpRequest::parseCookies(){
        auto ret = header_.get("Cookie");
        if(ret.exist()){
            hasCookies_ = true;
            cookie_ = HttpCookie(ret.value());
        }
    }

    void HttpRequest::parseContentLength(){
        auto ret = header_.get("Content-Length");
        if(ret.exist()){
            hasContentLength_ = true;
            contentLength_ = stoul(ret.value());
        }
    }

    void HttpRequest::parseTransferEncoding(){
        auto ret = header_.get("Transfer-Encoding");
        if(ret.exist()){
            hasTransferEncoding_ = true;
            transferEncoding_ = toLowers(ret.value());
            if(transferEncoding_ == "chunked"){
                chunked_ = true;
            }
        }
    }

    HttpRequest::RET_STATE HttpRequest::parseChunked(){
        std::string_view view(data_->peek(), data_->readableBytes());
        while(true){
            // 读取长度
            if(cur_chunkedState_ == HttpChunkedState::NO_LEN){
                size_t pos = view.substr(0, chunk_size_str_limit+2).find("\r\n");
                if(pos == view.npos){
                    return AGAIN_REQUEST;
                }
                // 数据长度超限
                if(pos > chunk_size_str_limit){
                    return BAD_REQUEST;
                }
                cur_chunk_size_ = hexString2Int(view.substr(0,pos));
                // 读到最后的空块，完毕
                if(cur_chunk_size_ == 0){
                    data_->retrieve(pos+2);
                    view.remove_prefix(pos+2);
                    cur_chunkedState_ = HttpChunkedState::DONE;
                    continue;
                }
                // 错误的数字
                if(cur_chunk_size_ == -1){
                    return BAD_REQUEST;
                }
                // 读取完长度，转为读取数据
                data_->retrieve(pos+2);
                view.remove_prefix(pos+2);
                cur_chunkedState_ = HttpChunkedState::NO_DATA;
            }
                // 读取数据
            else if(cur_chunkedState_ == HttpChunkedState::NO_DATA){
                if((int)view.size() < cur_chunk_size_ + 2){
                    return AGAIN_REQUEST;
                }
                    // 读取数据成功，开始准备下一个chunk
                else if(view[cur_chunk_size_] == '\r' && view[cur_chunk_size_+1] == '\n'){
                    body_.append(data_->peek(), cur_chunk_size_);
                    data_->retrieve(cur_chunk_size_+2);
                    view.remove_prefix(cur_chunk_size_+2);
                    cur_chunkedState_ = HttpChunkedState::NO_LEN;
                }
                else{
                    return BAD_REQUEST;
                }
            }
                // 所有chunk都接受完毕
            else if(cur_chunkedState_ == HttpChunkedState::DONE){
                if(view.size() < 2){
                    return AGAIN_REQUEST;
                }
                else if(view[0] == '\r' && view[1] == '\n'){
                    data_->retrieve(2);
                    view.remove_prefix(2);
                    return OK_REQUEST;
                }
                else{
                    return BAD_REQUEST;
                }
            }
        }
        return AGAIN_REQUEST;
    }

    void HttpRequest::parseSession(){
        auto ssid = cookie_.getSessionId();
        if(!ssid.empty()){
            if(sessionManager_) {
                auto ret = sessionManager_->getSession(ssid);
                if(ret.exist()){
                    session_ = ret.value();
                }
                else{
                    session_ = nullptr;
                }
            }
            else {
                session_ = nullptr;
            }
        }
        else{
            session_ = nullptr;
        }
    }

    HttpSession::_ptr HttpRequest::getSession(bool autoCreate){
        if(session_){
            return session_;
        }
        else if(autoCreate){
            if(sessionManager_) {
                return sessionManager_->newSession();
            }
        }
        return nullptr;
    }


}