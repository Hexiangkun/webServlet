//
// Created by 37496 on 2024/3/16.
//

#ifndef WEBSERVER_SEMPHORE_H
#define WEBSERVER_SEMPHORE_H

#include <semaphore.h>
#include <stdexcept>

namespace redis
{
    class Semaphore
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
}
#endif //WEBSERVER_SEMPHORE_H
