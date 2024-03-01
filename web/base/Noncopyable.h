//
// Created by 37496 on 2024/1/30.
//

#ifndef WEBSERVER_NONCOPYABLE_H
#define WEBSERVER_NONCOPYABLE_H

namespace Tiny_muduo
{
    class Noncopyable
    {
    protected:
        Noncopyable()= default;
        ~Noncopyable() = default;

    public:
        Noncopyable(const Noncopyable& ) = delete;
        Noncopyable& operator=(const Noncopyable& ) = delete;
    };
}

#endif //WEBSERVER_NONCOPYABLE_H
