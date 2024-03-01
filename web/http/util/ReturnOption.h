//
// Created by 37496 on 2024/2/22.
//

#ifndef WEBSERVER_RETURNOPTION_H
#define WEBSERVER_RETURNOPTION_H

namespace Tiny_muduo::Http
{
    template<typename T>
    class ReturnOption
    {
    public:
        ReturnOption(const T& val, bool exist) : _val(val), _exist(exist) {}
        T& value() { return _val; }
        bool exist() { return _exist; }
    private:
        T _val;
        bool _exist;
    };
}

#endif //WEBSERVER_RETURNOPTION_H
