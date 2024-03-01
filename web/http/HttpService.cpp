//
// Created by 37496 on 2024/2/28.
//

#include "HttpService.h"
#include "sqlconn/MySqlConnectionPool.h"

namespace Tiny_muduo::Http
{

    void Index(const HttpRequest& req, HttpResponse* resp)
    {
        resp->setStatusCode(HttpStatusCode::OK);
        resp->setContentType(HttpContentType::HTML);
        resp->setHtmlBody("index.html");
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
        response->setStatusCode(HttpStatusCode::OK);
        response->setContentType(HttpContentType::HTML);
        response->setHtmlBody("register.html");
    }
}