//
// Created by 37496 on 2024/2/16.
//

#ifndef WEBSERVER_UUID_H
#define WEBSERVER_UUID_H

#include <uuid/uuid.h>
#include <string>
#include "base/Mutex.h"

#define GUID_LEN 64

namespace Tiny_muduo
{
    namespace Uuid
    {
        static SpinLock uuid_mutex_;

        static std::string generate() {
            char buf[GUID_LEN] = {0};

            uuid_t uu;
            ::uuid_generate(uu);

            int32_t index = 0;
            for(size_t i = 0; i < 16; i++) {
                int32_t len = i < 15 ? sprintf(buf + index, "%02X-", uu[i]) : sprintf(buf + index, "%02X", uu[i]);
                if(len < 0) {
                    return std::move(std::string(""));
                }
                index += len;
            }

            return std::move(std::string(buf));
        }

        static std::string generate_threadSafe() {
            char buf[GUID_LEN] = {0};

            uuid_t uu;
            {
                LockGuard<SpinLock> lock(uuid_mutex_);
                ::uuid_generate(uu);
            }

            int32_t index = 0;
            for(size_t i = 0; i < 16; i++) {
                int32_t len = i < 15 ? sprintf(buf + index, "%02X-", uu[i]) : sprintf(buf + index, "%02X", uu[i]);
                if(len < 0) {
                    return std::move(std::string(""));
                }
                index += len;
            }

            return std::move(std::string(buf));
        }
    }
}

#endif //WEBSERVER_UUID_H
