//
// Created by 37496 on 2024/1/30.
//

#ifndef WEBSERVER_MUTEX_H
#define WEBSERVER_MUTEX_H

#include "web/base/Noncopyable.h"
#include <semaphore.h>
#include <pthread.h>
#include <atomic>
#include <stdexcept>

namespace Tiny_muduo
{
    class Semaphore : public Noncopyable
    {
    public:
        Semaphore(uint32_t count = 0)
        {
            if(sem_init(&_semaphore, 0, count)) {
                throw std::logic_error("sem_init error");
            }
        }
        ~Semaphore()
        {
            sem_destroy(&_semaphore);
        }

        void wait() {
            if(sem_wait(&_semaphore)) {
                throw std::logic_error("sem_wait error");
            }
        }

        void notify() {
            if(sem_post(&_semaphore)) {
                throw std::logic_error("sem_post error");
            }
        }

    private:
        sem_t _semaphore;
    };

    template<typename M>
    class LockInterface : public Noncopyable
    {
    public:
        LockInterface() = default;
        ~LockInterface() = default;

        virtual bool Lock() = 0;
        virtual bool Unlock() = 0;
        M& getMutex() { return _lock; }

    protected:
        M _lock;
    };

    //互斥锁
    class MutexLock : public LockInterface<pthread_mutex_t >
    {
    public:
        MutexLock()
        {
            pthread_mutex_init(&_lock, nullptr);
        }

        ~MutexLock()
        {
            pthread_mutex_destroy(&_lock);
        }

        bool Lock() override
        {
            return pthread_mutex_lock(&_lock) == 0;
        }

        bool Unlock() override
        {
            return pthread_mutex_unlock(&_lock) == 0;
        }

    };

    // 自旋锁
    class SpinLock : public LockInterface<pthread_spinlock_t >
    {
    public:
        SpinLock() {
            pthread_spin_init(&_lock, 0);
        }

        ~SpinLock() {
            pthread_spin_destroy(&_lock);
        }

        bool Lock() override {
            return pthread_spin_lock(&_lock) == 0;
        }

        bool Unlock() override {
            return pthread_spin_unlock(&_lock) == 0;
        }

    };

    //CAS原子锁
    class CASLock : public LockInterface<std::atomic_flag>
    {
    public:
        CASLock() {
            _lock.clear();
        }

        ~CASLock() = default;

        bool Lock() override {
            while(std::atomic_flag_test_and_set_explicit(&_lock, std::memory_order_acquire)){

            }
            return true;
        }

        bool Unlock() override {
            std::atomic_flag_clear_explicit(&_lock, std::memory_order_release);
            return true;
        }
    };

    template<class T>
    class LockGuard
    {
    public:
        LockGuard(T& mutex) : _mutex(mutex) {
            _mutex.Lock();
            _locked = true;
        }

        ~LockGuard() {
            if(_locked) {
                _mutex.Unlock();
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
}


#endif //WEBSERVER_MUTEX_H
