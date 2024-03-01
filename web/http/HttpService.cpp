//
// Created by 37496 on 2024/2/28.
//

#include "HttpService.h"
#include "sqlconn/MySqlConnectionPool.h"

namespace Tiny_muduo::Http
{
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
            std::filesystem::path p("/root/resources/"+request.getPath().substr(1));
            if(std::filesystem::exists(p)) {
                response->setStatusCode(HttpStatusCode::OK);
                response->setContentType(Ext2HttpContentType.at(p.extension()));
                response->setFileBody(p.string());
            }
            else {
                response->setStatusCode(HttpStatusCode::BAD_REQUEST);
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