//
// Created by 37496 on 2024/3/3.
//

#ifndef WEBSERVER_SPINLOCK_H
#define WEBSERVER_SPINLOCK_H

#include <atomic>

namespace Tiny_muduo
{

    class Spinlock {
        // 这里flag的初始状态只能是清除，即为false
        // 这样的话第一个到达的线程上锁后其余的便开始原地等待，直到第一个线程解锁
        std::atomic_flag flag;

    public:
        Spinlock() : flag(ATOMIC_FLAG_INIT) {}

        void lock() {
            while (flag.test_and_set(std::memory_order_acquire)) {
            }
        }

        void unlock() { flag.clear(std::memory_order_release); }
    };
}


#endif //WEBSERVER_SPINLOCK_H
