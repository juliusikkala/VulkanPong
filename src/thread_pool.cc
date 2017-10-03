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
#include <algorithm>

thread_pool::thread_pool()
: thread_pool(std::max(std::thread::hardware_concurrency()-1, 0u)) { }

thread_pool::thread_pool(unsigned thread_count)
: busy_threads(0), id_counter(1) /*0 is reserved*/
{
    resize(thread_count);
}

thread_pool::~thread_pool()
{
    for(size_t i = 0; i < threads.size(); ++i)
    {
        post(task(
            std::packaged_task<void()>(), // No task for you,
            std::numeric_limits<unsigned>::max(), // just do it now,
            true // quit.
        )); // We don't need you anymore.
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

        unsigned tmp_threads_to_exit = threads_to_exit;
        for(unsigned i = 0; i < tmp_threads_to_exit; ++i)
        {
            std::thread::id& id = exited_thread_ids[i];
            post(task(
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
            ));
        }

        finished.wait(
            finished_lock,
            [&threads_to_exit]{return threads_to_exit == 0;}
        );

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

unsigned thread_pool::busy() const
{
    return busy_threads;
}

thread_pool::task_id thread_pool::post(
    thread_pool::task&& t,
    const std::set<task_id>& dependencies
){
    t.id = id_counter++;
    all_tasks.insert(t.id);
    if(dependencies.empty())
    {
        queue_task(std::move(t));
        return t.id;
    }

    std::lock_guard<std::mutex> lock(pending_tasks_mutex);
    std::set_intersection(
        all_tasks.begin(),
        all_tasks.end(),
        dependencies.begin(),
        dependencies.end(),
        std::inserter(t.dependencies, t.dependencies.end())
    );
    if(t.dependencies.empty())
    {
        queue_task(std::move(t));
    }
    else
    {
        pending_tasks.emplace_back(std::move(t));
    }
    return t.id;
}

void thread_pool::execute_loop()
{
    bool finished = false;
    while(!finished)
    {
        std::unique_lock<std::mutex> lk(tasks_mutex);
        if(busy_threads == 0 && tasks_queue.empty())
        {
            no_tasks_running.notify_all();
        }
        while(tasks_queue.empty())
        {
            new_task.wait(lk);
        }
        busy_threads++;
        task todo(std::move(const_cast<task&>(tasks_queue.top())));
        tasks_queue.pop();
        lk.unlock();

        finished = todo.finish_thread;

        try
        {
            if(todo.task_func.valid()) todo.task_func();

            std::lock_guard<std::mutex> lock(pending_tasks_mutex);
            on_finish_task(todo.id);
        }
        catch(std::exception& ex)
        {
            std::cerr << ex.what() << std::endl;

            std::lock_guard<std::mutex> lock(pending_tasks_mutex);
            on_fail_task(todo.id);
        }
        --busy_threads;
    }
}

void thread_pool::queue_task(thread_pool::task&& t)
{
    if(threads.size() == 0)
    {
        t.task_func();
    }
    else
    {
        tasks_mutex.lock();
        tasks_queue.push(std::move(t));
        tasks_mutex.unlock();
        new_task.notify_one();
    }
}

void thread_pool::on_finish_task(task_id id)
{
    all_tasks.erase(id);
    for(size_t i = 0; i < pending_tasks.size(); ++i)
    {
        task& t = pending_tasks[i];
        t.dependencies.erase(id);
        if(t.dependencies.empty())
        {
            queue_task(std::move(t));
            pending_tasks.erase(pending_tasks.begin()+i);
            --i;
        }
    }
}

bool thread_pool::on_fail_task(task_id id)
{
    all_tasks.erase(id);
    bool had_children = false;
    for(size_t i = 0; i < pending_tasks.size(); ++i)
    {
        task& t = pending_tasks[i];
        if(t.dependencies.count(id))
        {
            had_children = true;
            task_id child_id = t.id;
            pending_tasks.erase(pending_tasks.begin()+i);
            if(on_fail_task(child_id))
            {
                i = 0;
            }
        }
    }
    return had_children;
}

void thread_pool::finish()
{
    std::unique_lock<std::mutex> lk(tasks_mutex);
    if(busy_threads == 0 && tasks_queue.empty()) return;
    no_tasks_running.wait(
        lk,
        [this]{return busy_threads == 0 && tasks_queue.empty();}
    );
}

bool thread_pool::task::operator<(const thread_pool::task& other) const
{
    return priority < other.priority;
}
