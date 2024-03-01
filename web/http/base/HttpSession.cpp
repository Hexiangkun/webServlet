//
// Created by 37496 on 2024/2/22.
//

#include "http/base/HttpSession.h"
#include "http/util/Uuid.h"


namespace Tiny_muduo::Http
{
    static const int INTERVAL = 180; // 三分钟会话过期
    HttpSession::HttpSession()
            :_id(Uuid::generate_threadSafe()),
            _status(Status::New),
            _interval(INTERVAL)
    {

    }

    const std::string HttpSession::getId() const {
        ReadLockGuard<ReadWriteLock> lockGuard(_mutex);
        return _id;
    }

    bool HttpSession::isNew() const {
        ReadLockGuard<ReadWriteLock> lockGuard(_mutex);
        return _status == Status::New;
    }

    bool HttpSession::isAccessed() const {
        ReadLockGuard<ReadWriteLock> lockGuard(_mutex);
        return _status == Status::Accessed;
    }

    bool HttpSession::isDestroy() const {
        ReadLockGuard<ReadWriteLock> lockGuard(_mutex);
        return _status == Status::Destroy;
    }

    int HttpSession::getMaxInactiveInterval() const {
        ReadLockGuard<ReadWriteLock> lockGuard(_mutex);
        return _interval;
    }

    void HttpSession::setStatus(Tiny_muduo::Http::HttpSession::Status status) {
        WriteLockGuard<ReadWriteLock> lockGuard(_mutex);
        _status = status;
    }

    template<typename V>
    void HttpSession::setValue(const std::string &key, const V &val) {
        WriteLockGuard<ReadWriteLock> lockGuard(_mutex);
        add(key, std::forward<const V&>(val));
    }

    template<typename V>
    const V *HttpSession::getValue(const std::string &key) const {
        ReadLockGuard<ReadWriteLock> lockGuard(_mutex);
        auto ret = get(key);
        if(!ret.exist()) {
            return nullptr;
        }
        try {
            return &(std::any_cast<const V&>(ret.value()));
        }
        catch (std::bad_any_cast& ) {

        }
        return nullptr;
    }

    ReturnOption<HttpSession::_ptr> HttpSessionManager::getSession(const std::string &id) {
        ReadLockGuard<ReadWriteLock> lockGuard(_mutex);
        auto it = _sessions.find(id);
        if(it == _sessions.end()) {
            return {nullptr, false};
        }
        return {it->second, true};
    }

    void HttpSessionManager::addSession(const std::string &id, HttpSession::_ptr session) {
        WriteLockGuard<ReadWriteLock> lockGuard(_mutex);
        _sessions[id] = session;
    }

    HttpSession::_ptr HttpSessionManager::newSession() {
        HttpSession::_ptr session = std::make_shared<HttpSession>();
        WriteLockGuard<ReadWriteLock> lockGuard(_mutex);
        _sessions[session->getId()] = session;
        return session;
    }

    void HttpSessionManager::delSession(const std::string &id) {
        WriteLockGuard<ReadWriteLock> lockGuard(_mutex);
        if(_sessions.find(id) != _sessions.end()) {
            _sessions.erase(id);
        }
    }
}