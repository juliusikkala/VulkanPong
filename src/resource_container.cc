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
#include "resource_container.hh"
#include "resource_manager.hh"

basic_resource_container::basic_resource_container(resource_manager& manager)
: manager(manager), system_references(0)
{
}

basic_resource_container::~basic_resource_container()
{
}

void basic_resource_container::pin() const
{
    if(++system_references == 1)
    {
        start_load_system();
    }
}

void basic_resource_container::unpin() const
{
    if(--system_references == 0)
    {
        start_unload_system();
    }
}

void basic_resource_container::pin(device_id id) const
{
    pin();
    if(++device_results[id].references == 1)
    {
        start_load_device(id);
    }
}

void basic_resource_container::unpin(device_id id) const
{
    if(--device_results[id].references == 0)
    {
        start_unload_device(id);
    }
    unpin();
}

void basic_resource_container::wait_load_system() const
{
    if(!load_system_result.valid())
    {
        start_load_system();
    }
    load_system_result.wait();
}

void basic_resource_container::wait_load_device(device_id id) const
{
    device_load_results& d = device_results[id];
    if(!d.load.valid())
    {
        start_load_device(id);
    }
    d.load.wait();
}

void basic_resource_container::start_load_system() const
{
    std::lock_guard<std::mutex> lock(start_load_mutex);
    if(!load_system_result.valid())
    {
        load_system_result = manager.pool.postd(
            { unload_system_result.get_id() },
            PRIORITY_PRONTO,
            [&](){ load_system(); }
        );
        unload_system_result.clear();
    }
}

void basic_resource_container::start_unload_system() const
{
    std::lock_guard<std::mutex> lock(start_load_mutex);
    if(!unload_system_result.valid())
    {
        std::set<thread_pool::task_id> dependencies = {
            load_system_result.get_id() 
        };
        for(auto& pair: device_results)
        {
            dependencies.insert(pair.second.unload.get_id());
        }
        unload_system_result = manager.pool.postd(
            dependencies,
            PRIORITY_PRONTO,
            [&](){ unload_system(); }
        );
        load_system_result.clear();
    }
}

void basic_resource_container::start_load_device(device_id id) const
{
    std::lock_guard<std::mutex> lock(start_load_mutex);
    device_load_results& d = device_results[id];
    if(!d.load.valid())
    {
        d.load = manager.pool.postd(
            { load_system_result.get_id(), d.unload.get_id() },
            PRIORITY_PRONTO,
            [&](){ load_device(id); }
        );
        d.unload.clear();
    }
}

void basic_resource_container::start_unload_device(device_id id) const
{
    std::lock_guard<std::mutex> lock(start_load_mutex);
    device_load_results& d = device_results[id];
    if(!d.unload.valid())
    {
        d.unload = manager.pool.postd(
            { d.load.get_id() },
            PRIORITY_PRONTO,
            [&](){ unload_device(id); }
        );
        d.load.clear();
    }
}

basic_resource_container::device_load_results::device_load_results()
: references(0) {}

resource_data::~resource_data(){}
