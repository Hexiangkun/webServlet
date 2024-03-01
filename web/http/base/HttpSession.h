//
// Created by 37496 on 2024/2/22.
//

#ifndef WEBSERVER_HTTPSESSION_H
#define WEBSERVER_HTTPSESSION_H


#include <any>
#include <memory>
#include "http/util/HttpMap.h"
#include "base/ReadWriteLock.h"
#include "base/Noncopyable.h"
#include "net/timer/TimerId.h"

namespace Tiny_muduo::Http
{
    class HttpSession : public HttpBaseMap<std::string , std::any>
    {
    public:
        using _ptr = std::shared_ptr<HttpSession>;
        enum Status {
            New,
            Accessed,
            Destroy,
        };
        HttpSession();

        const std::string getId() const;
        bool isNew() const;
        bool isAccessed() const;
        bool isDestroy() const;
        int getMaxInactiveInterval() const;

        void setStatus(Status status);

        template<typename V>
        void setValue(const std::string& key, const V& val);

        template<typename V>
        const V* getValue(const std::string& key) const;

        void setTimerId(const net::TimerId timerId) {
            _timerId = timerId;
        }

        net::TimerId getTimerId() const { return _timerId; }

    private:
        std::string _id;
        Status _status;
        int _interval;

        net::TimerId _timerId;
        mutable ReadWriteLock _mutex;
    };

    class HttpSessionManager : public Noncopyable
    {
    public:
        static HttpSessionManager& getInstance() {
            static HttpSessionManager httpSessionManager;
            return httpSessionManager;
        }

        HttpSessionManager() = default;

        ReturnOption<HttpSession::_ptr > getSession(const std::string& id);
        void addSession(const std::string& id, HttpSession::_ptr  session);
        HttpSession::_ptr newSession();
        void delSession(const std::string& id);

    private:
        mutable ReadWriteLock _mutex;
        std::unordered_map<std::string, HttpSession::_ptr > _sessions;
    };
}


#endif //WEBSERVER_HTTPSESSION_H
