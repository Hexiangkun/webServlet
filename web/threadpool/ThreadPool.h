//
// Created by 37496 on 2024/2/24.
//

#ifndef WEBSERVER_THREADPOOL_H
#define WEBSERVER_THREADPOOL_H

#include<vector>
#include<thread>
#include<queue>
#include<functional>
#include<mutex>
#include<condition_variable>
#include<future>

namespace Tiny_muduo
{

    class ThreadPool
    {
    private:
        std::vector<std::thread> task_threads;
        std::queue<std::function<void()>> tasks_queue;
        std::mutex task_mtx;
        std::condition_variable cv;
        bool stop;
    public:
        ThreadPool(int size=std::thread::hardware_concurrency()/2):stop(false)
        {
            for (int i = 0; i < size; i++) {
                task_threads.emplace_back(std::thread([this]() {
                    while (true)
                    {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(task_mtx);
                            cv.wait(lock, [this]() {
                                return stop || !tasks_queue.empty();
                            });
                            if (stop && tasks_queue.empty()) {
                                return;
                            }
                            task = std::move(tasks_queue.front());
                            tasks_queue.pop();
                        }
                        task();
                    }
                }));
            }
        }
        ~ThreadPool()
        {
            {
                std::unique_lock<std::mutex> lock(task_mtx);
                stop = true;
            }
            cv.notify_all();
            for (std::thread &th: task_threads) {
                if (th.joinable()) {
                    th.join();
                }
            }
        }

        template<class F,class... Args>
        auto add(F&& f, Args&&... args)->std::future<typename std::result_of<F(Args...)>::type>;
    };


    template<class F, class... Args>
    auto ThreadPool::add(F&& f, Args&&...args) -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using return_type = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>> (std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(task_mtx);
            if (stop) {
                throw std::runtime_error("ThreadPool has already stop!");
            }
            tasks_queue.emplace([task]() {(*task)(); });
        }
        cv.notify_one();
        return res;
    }
}

#endif //WEBSERVER_THREADPOOL_H
