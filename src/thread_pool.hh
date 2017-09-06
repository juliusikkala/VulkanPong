/*
MIT License

Copyright (c) 2017 Julius Ikkala

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef PONG_THREAD_POOL_HH
#define PONG_THREAD_POOL_HH
#include <thread>
#include <mutex>
#include <functional>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <limits>
#include <memory>
#include <future>

constexpr unsigned PRIORITY_LOW = 0;
constexpr unsigned PRIORITY_MEDIUM = 100;
constexpr unsigned PRIORITY_HIGH = 1000;
constexpr unsigned PRIORITY_PRONTO = std::numeric_limits<unsigned>::max();

class thread_pool
{
public:
    // Launches as many threads as there are cores on the machine, -1
    thread_pool();
    // 0 threads will cause any given task be run immediately when queuing.
    thread_pool(unsigned thread_count);
    thread_pool(const thread_pool& other) = delete;
    ~thread_pool();

    // If resizing down, may be slow if all threads are being used (removed ones
    // have to be joined). Do not call this function from several threads
    // simultaneously.
    void resize(unsigned thread_count);

    unsigned size() const;
    unsigned busy() const;

    template<typename F, typename... Args>
    auto post(F&& f, Args&&... args)
    -> std::future<decltype(f(std::forward<Args>(args)...))>;

    template<typename F, typename... Args>
    auto postp(
        unsigned priority,
        F&& f,
        Args&&... args
    ) -> std::future<decltype(f(std::forward<Args>(args)...))>;

    // Block until queue is empty
    void finish();

private:
    struct task
    {
        template<typename F>
        task(F&& func, unsigned priority, bool finish_thread);

        std::packaged_task<void()> task_func;
        unsigned priority;
        bool finish_thread;

        bool operator<(const thread_pool::task& other) const;
    };

    void post(thread_pool::task&& t);

    void execute_loop();

    std::priority_queue<task> tasks;
    std::mutex tasks_mutex;
    std::condition_variable new_task, no_tasks_running;

    std::vector<std::thread> threads;
    std::atomic_uint busy_threads;
};

#include "thread_pool.tcc"
#endif
