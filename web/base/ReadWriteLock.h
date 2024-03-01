//
// Created by 37496 on 2024/2/3.
//

#ifndef WEBSERVER_READWRITELOCK_H
#define WEBSERVER_READWRITELOCK_H

#include <pthread.h>

namespace Tiny_muduo
{
    template<typename T>
    class ReadLockGuard
    {
    public:
        ReadLockGuard(T& mtx) :_mutex(mtx) {
            _mutex.rdlock();
            _locked = true;
        }

        ~ReadLockGuard() {
            if(_locked) {
                _mutex.unlock();
                _locked = false;
            }
        }

        T& getLock() {
            return _mutex;
        }


    private:
        T& _mutex;
        bool _locked;
    };


    template<typename T>
    class WriteLockGuard
    {
    public:
        WriteLockGuard(T& mtx) :_mutex(mtx) {
            _mutex.wrlock();
            _locked = true;
        }

        ~WriteLockGuard() {
            if(_locked) {
                _mutex.unlock();
                _locked = false;
            }
        }

        T& getLock() {
            return _mutex;
        }


    private:
        T& _mutex;
        bool _locked;
    };


    class ReadWriteLock
    {
    public:
        ReadWriteLock()
        {
            pthread_rwlock_init(&_mutex, nullptr);
        }

        ~ReadWriteLock() {
            pthread_rwlock_destroy(&_mutex);
        }

        void rdlock() {
            pthread_rwlock_rdlock(&_mutex);
        }

        void wrlock() {
            pthread_rwlock_wrlock(&_mutex);
        }

        void unlock() {
            pthread_rwlock_unlock(&_mutex);
        }
    private:
        pthread_rwlock_t _mutex;
    };
}

#endif //WEBSERVER_READWRITELOCK_H
