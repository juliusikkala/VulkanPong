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
#include "thread_pool.hh"
#include <cstdlib>
#include <iostream>
#include "helpers.hh"

thread_pool::thread_pool()
: thread_pool(std::max(std::thread::hardware_concurrency()-1, 0u)) { }

thread_pool::thread_pool(unsigned thread_count)
{
    resize(thread_count);
}

thread_pool::~thread_pool()
{
    for(size_t i = 0; i < threads.size(); ++i)
    {
        post({nullptr, std::numeric_limits<unsigned>::max(), true});
    }
    for(std::thread& thread: threads)
    {
        thread.join();
    }
}

void thread_pool::resize(unsigned thread_count)
{
    // smaller
    if(thread_count < threads.size())
    {
        std::atomic_uint threads_to_exit(threads.size() - thread_count);
        std::vector<std::thread::id> exited_thread_ids(threads_to_exit);

        std::mutex finished_mutex;
        std::unique_lock<std::mutex> finished_lock(finished_mutex);
        std::condition_variable finished;
        std::vector<std::thread::id> finished_thread_ids;

        for(unsigned i = 0; i < threads_to_exit; ++i)
        {
            std::thread::id& id = finished_thread_ids[i];
            post({
                [&id, &threads_to_exit, &finished]()
                {
                    id = std::this_thread::get_id();
                    if(--threads_to_exit == 0)
                    {
                        finished.notify_all();
                    }
                },
                std::numeric_limits<unsigned>::max(),
                true
            });
        }

        finished.wait(finished_lock);

        for(unsigned i = 0; i < threads.size(); ++i)
        {
            for(std::thread::id id: exited_thread_ids)
            {
                std::thread& thread = threads[i];
                if(id == thread.get_id())
                {
                    thread.join();
                    threads.erase(threads.begin() + i);
                    --i;
                    break;
                }
            }
        }
    }
    // bigger
    else if(thread_count > threads.size())
    {
        unsigned threads_to_add = thread_count - threads.size();
        while(threads_to_add--)
            threads.emplace_back(std::bind(&thread_pool::execute_loop, this));
    }
}

unsigned thread_pool::size() const
{
    return threads.size();
}

void thread_pool::post(thread_pool::task&& t)
{
    if(threads.size() == 0)
    {
        t.task();
    }
    else
    {
        tasks_mutex.lock();
        tasks.push(std::forward<task>(t));
        tasks_mutex.unlock();
        new_task.notify_one();
    }
}

void thread_pool::execute_loop()
{
    bool finished = false;
    while(!finished)
    {
        std::unique_lock<std::mutex> lk(tasks_mutex);
        if(busy_threads == 0 && tasks.empty())
        {
            no_tasks_running.notify_all();
        }
        while(tasks.empty())
        {
            new_task.wait(lk);
        }
        busy_threads++;
        std::function<void()> task(tasks.top().task);
        finished = tasks.top().finish_thread;
        tasks.pop();
        lk.unlock();

        try
        {
            if(task) task();
        }
        catch(std::exception& ex)
        {
            std::cerr << ex.what() << std::endl;
        }
        --busy_threads;
    }
}

void thread_pool::finish()
{
    std::unique_lock<std::mutex> lk(tasks_mutex);
    no_tasks_running.wait(lk);
}

bool thread_pool::task::operator<(const thread_pool::task& other) const
{
    return priority < other.priority;
}
