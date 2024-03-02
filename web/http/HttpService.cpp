//
// Created by 37496 on 2024/2/28.
//

#include "HttpService.h"
#include "sqlconn/MySqlConnectionPool.h"

namespace Tiny_muduo::Http
{
    namespace detail
    {
        std::string DefaultPage(int code, const std::string &content) {
            std::string text = std::to_string(code) + " " + HttpStatusCode2Str.at(code);
            return R"(<html><head><title>)" + text +
                   R"(</title></head><body><center><h1>)" + text +
                   R"(</h1></center><hr><center>)" + content +
                   R"(</center></body></html>)";
        }
    }

    static const std::string STORE_ROOT = "/root/webserver/store/";

    void DefaultHandle(const HttpRequest& request, HttpResponse* response) {
        auto code = response->getStatusCode();

        if(code >= 400) {
            response->setContentType(HttpContentType::HTML);
            switch (code) {
                case BAD_REQUEST:
                    response->setHtmlBody("400.html");
                    break;
                case UNAUTHORIZED:
                    response->setHtmlBody("401.html");
                    break;
                case FORBIDDEN:
                    response->setHtmlBody("403.html");
                    break;
                case NOT_FOUND:
                    response->setHtmlBody("404.html");
                    break;
                case METHOD_NOT_ALLOWED:
                    response->setHtmlBody("405.html");
                    break;
                case RANGE_NOT_SATISFIABLE:
                    response->setHtmlBody("416.html");
                default:
                    response->setHtmlBody("400.html");
                    break;
            }
            return ;
        }
    }

    void Index(const HttpRequest& request, HttpResponse* response)
    {
        if(request.getMethod() == HttpMethod::GET) {
            response->setStatusCode(HttpStatusCode::OK);
            response->setContentType(HttpContentType::HTML);
            response->setHtmlBody("index.html");
        }
        else {
            response->setStatusCode(HttpStatusCode::BAD_REQUEST);
            DefaultHandle(request, response);
        }
    }

    void Login(const HttpRequest& request, HttpResponse* response) {
        if(request.getMethod() == HttpMethod::GET) {
            response->setStatusCode(HttpStatusCode::OK);
            response->setContentType(HttpContentType::HTML);
            response->setHtmlBody("login.html");
        }
        else if(request.getMethod() == HttpMethod::POST) {
            ReturnOption<std::string> username = request.getForm().get("username");
            ReturnOption<std::string> password = request.getForm().get("password");
            bool login = false;
            if(username.exist() && password.exist()) {
                std::shared_ptr<MySqlConnection> conn = MySqlConnectionPool::getInstance()->getConnection();
                MySqlDataReader* rd = conn->ExecuteReader("select passwd from user where username = ?", username.value());
                while(rd->Read()) {
                    std::string passwd;
                    rd->GetValues(passwd);
                    if(passwd == password.value()) {
                        login = true;
                    }
                }
                delete rd;
            }
            if(login) {
                response->setStatusCode(HttpStatusCode::OK);
                response->setContentType(HttpContentType::HTML);
                response->setHtmlBody("welcome.html");
            }
            else {
                response->setStatusCode(HttpStatusCode::OK);
                response->setContentType(HttpContentType::HTML);
                response->setHtmlBody("error.html");
            }
        }
    }

    void Register(const HttpRequest& request, HttpResponse* response) {
        if(request.getMethod() == HttpMethod::GET) {
            response->setStatusCode(HttpStatusCode::OK);
            response->setContentType(HttpContentType::HTML);
            response->setHtmlBody("register.html");
        }
        else if(request.getMethod() == HttpMethod::POST) {
            ReturnOption<std::string> username = request.getForm().get("username");
            ReturnOption<std::string> password = request.getForm().get("password");
            bool register_flag = false;
            if(username.exist() && password.exist()) {
                std::shared_ptr<MySqlConnection> conn = MySqlConnectionPool::getInstance()->getConnection();
                size_t i = conn->ExecuteNonQuery("insert into user(username, passwd) values(?,?)", username.value(), password.value());
                if(i > 0) {
                    register_flag = true;
                }
            }
            if(register_flag) {
                response->setStatusCode(HttpStatusCode::OK);
                response->setContentType(HttpContentType::HTML);
                response->setHtmlBody("welcome.html");
            }
            else {
                response->setStatusCode(HttpStatusCode::OK);
                response->setContentType(HttpContentType::HTML);
                response->setHtmlBody("error.html");
            }
        }
    }

    void Picture(const HttpRequest& request, HttpResponse* response) {
        if(request.getMethod() == HttpMethod::GET) {
            response->setStatusCode(HttpStatusCode::OK);
            response->setContentType(HttpContentType::HTML);
            response->setHtmlBody("picture.html");
        }
        else {
            response->setStatusCode(HttpStatusCode::BAD_REQUEST);
            DefaultHandle(request, response);
        }
    }

    void Video(const HttpRequest& request, HttpResponse* response) {
        if(request.getMethod() == HttpMethod::GET) {
            response->setStatusCode(HttpStatusCode::OK);
            response->setContentType(HttpContentType::HTML);
            response->setHtmlBody("video.html");
        }
        else {
            response->setStatusCode(HttpStatusCode::BAD_REQUEST);
            DefaultHandle(request, response);
        }
    }

    void BlogIndex(const HttpRequest& request, HttpResponse* response) {
        if(request.getMethod() == HttpMethod::GET) {
            response->setStatusCode(HttpStatusCode::OK);
            response->setContentType(HttpContentType::HTML);
            response->setHtmlBody("blogindex.html");
        }
        else {
            response->setStatusCode(HttpStatusCode::BAD_REQUEST);
            DefaultHandle(request, response);
        }
    }

    void FileUpload(const HttpRequest& request, HttpResponse* response) {
        if(request.getMethod() == HttpMethod::GET) {
            response->setStatusCode(HttpStatusCode::OK);
            response->setContentType(HttpContentType::HTML);
            response->setHtmlBody("fileupload.html");
        }
        else if(request.getMethod() == HttpMethod::POST) {
            auto part = request.getMultiPart().getFile("filename");
            if(part) {
                FILE* file;
                const std::string filepath = STORE_ROOT + part->file_name;
                if(file) {
                    file = fopen(filepath.c_str(), "w");
                    fwrite(part->data.data(), part->data.size(), 1, file);
                    fflush(file);
                    fclose(file);

                    response->setStatusCode(HttpStatusCode::OK);
                    response->setContentType(HttpContentType::HTML);
                    response->setHtmlBody("fileupload.html");
                }
                else {
                    response->setStatusCode(HttpStatusCode::BAD_REQUEST);
                    DefaultHandle(request, response);
                }
            }
            else {
                response->setStatusCode(HttpStatusCode::BAD_REQUEST);
                DefaultHandle(request, response);
            }
        }
        else {
            response->setStatusCode(HttpStatusCode::BAD_REQUEST);
            DefaultHandle(request, response);
        }
    }

    void FileDownload(const HttpRequest& request, HttpResponse* response) {
        if(request.getPath() == "/filedownload") {
            response->setStatusCode(HttpStatusCode::OK);
            response->setContentType(HttpContentType::HTML);
            response->setHtmlBody("filedownload.html");
        }
        else  {
            std::filesystem::path p("/root/resources"+request.getPath());
            if(std::filesystem::exists(p)) {
                int fd = ::open(p.string().c_str(), O_RDONLY);
                if(fd < 0) {
                    response->setStatusCode(HttpStatusCode::NOT_FOUND);
                    DefaultHandle(request, response);
                    return;
                }
                auto len = std::filesystem::file_size(p);
                response->setFileFd(fd);
                response->addAtrribute("Accept-Ranges", "bytes");
                std::string e = p.extension();
                response->setContentType(Ext2HttpContentType.at(e));

                ReturnOption<std::string> range_option = request.getHeader().get("Range");
                if(range_option.exist()) {
                    std::string range = range_option.value();
                    response->setStatusCode(HttpStatusCode::PARTIAL_CONTENT);

                    off64_t beg_num = 0, end_num = 0;
                    std::string range_value = range.substr(6);
                    size_t pos = range_value.find("-");
                    std::string beg = range_value.substr(0, pos);
                    std::string end = range_value.substr(pos + 1);
                    if (beg != "" && end != "")
                    {
                        beg_num = stoi(beg);
                        end_num = stoi(end);
                    }
                    else if (beg != "" && end == "")
                    {
                        beg_num = stoi(beg);
                        end_num = len - 1;
                    }
                    else if (beg == "" && end != "")
                    {
                        beg_num = len - stoi(end);
                        end_num = len - 1;
                    }

                    // 需要读need_len个字节
                    // off64_t need_len = std::min(end_num - beg_num + 1, maxSendLen);
                    off64_t need_len = end_num - beg_num + 1;
                    end_num = beg_num + need_len - 1;
                    lseek(fd, beg_num, SEEK_SET);
                    response->setFileLen(static_cast<int>(need_len));

                    std::ostringstream os_range;
                    os_range << "bytes " << beg_num << "-" << end_num << "/" << len;
                    response->addAtrribute("Content-Range", os_range.str());
                    response->setContentLength(need_len);
                }
                else {
                    response->setStatusCode(HttpStatusCode::OK);
                    response->setFileLen(len);
                    response->setContentLength(len);
                }
            }
            else {
                response->setStatusCode(HttpStatusCode::NOT_FOUND);
                DefaultHandle(request, response);
            }
        }
    }

    void AboutMe(const HttpRequest& request, HttpResponse* response) {
        if(request.getMethod() == HttpMethod::GET) {
            response->setStatusCode(HttpStatusCode::OK);
            response->setContentType(HttpContentType::HTML);
            response->setHtmlBody("Aboutme.html");
        }
        else if(request.getMethod() == HttpMethod::POST) {

        }
        else {
            response->setStatusCode(HttpStatusCode::BAD_REQUEST);
            DefaultHandle(request, response);
        }
    }

    void WebDetails(const HttpRequest& request, HttpResponse* response) {
        if(request.getMethod() == HttpMethod::GET) {
            response->setStatusCode(HttpStatusCode::OK);
            response->setContentType(HttpContentType::HTML);
            response->setHtmlBody("WebDetails.html");
        }
        else {
            response->setStatusCode(HttpStatusCode::BAD_REQUEST);
            DefaultHandle(request, response);
        }
    }

    void MsgBoard(const HttpRequest& request, HttpResponse* response) {
        response->setStatusCode(HttpStatusCode::OK);
        response->setContentType(HttpContentType::HTML);
        response->setHtmlBody("msgboard.html");
    }
}