//
// Created by 37496 on 2024/2/13.
//

#ifndef WEBSERVER_TYPE_H
#define WEBSERVER_TYPE_H

#include <stdint.h>
#ifdef MUDUO_STD_STRING
#include <string>
#else  // !MUDUO_STD_STRING
#include <ext/vstring.h>
#include <ext/vstring_fwd.h>
#endif


namespace Tiny_muduo
{
#ifdef MUDUO_STD_STRING
    using std::string;
#else  // !MUDUO_STD_STRING
    typedef __gnu_cxx::__sso_string string;
#endif

    template<typename To, typename From>
    inline To implicit_cast(From const &f)
    {
        return f;
    }

    template<typename To, typename From>
    inline To down_cast(From* f)
    {
        if(false)
        {
            implicit_cast<From*, To>(0);
        }
#if !defined(NDEBUG) && !defined(GOOGLE_PROTOBUF_NO_RTTI)
        assert(f == NULL || dynamic_cast<To>(f) != NULL);  // RTTI: debug mode only!
#endif

        return static_cast<To>(f);
    }

}
#endif //WEBSERVER_TYPE_H
