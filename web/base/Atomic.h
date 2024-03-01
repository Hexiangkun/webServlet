//
// Created by 37496 on 2024/2/27.
//

#ifndef WEBSERVER_ATOMIC_H
#define WEBSERVER_ATOMIC_H

#include <cstdint>
#include <ostream>

namespace Tiny_muduo
{
    template<typename T>
    class AtomicIntegerT
    {
    public:
        AtomicIntegerT():m_value(0)
        {
        }

        T get()
        {
            return __sync_val_compare_and_swap(&m_value, 0, 0);
        }

        T getAndAdd(T x)
        {
            return __sync_fetch_and_add(&m_value, x);
        }

        T addAndGet(T x)
        {
            return getAndAdd(x) + x;
        }

        T incrementAndGet()
        {
            return addAndGet(1);
        }

        T decrementAndGet()
        {
            return addAndGet(-1);
        }

        void add(T x)
        {
            getAndAdd(x);
        }

        void increment()
        {
            incrementAndGet();
        }

        void decrement()
        {
            decrementAndGet();
        }

        T getAndSet(T newValue)
        {
            return __sync_lock_test_and_set(&m_value, newValue);
        }

        friend std::ostream& operator<<(std::ostream& os, const AtomicIntegerT<T>& atomicInt) {
            os << atomicInt.m_value;
            return os;
        }

    private:
        volatile T m_value;
    };

    typedef AtomicIntegerT<int32_t> AtomicInt32;
    typedef AtomicIntegerT<int64_t> AtomicInt64;


}

#endif //WEBSERVER_ATOMIC_H
