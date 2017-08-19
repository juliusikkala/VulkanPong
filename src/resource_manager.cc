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
#include "resource_manager.hh"
#include "context.hh"

resource_manager::resource_manager(context& ctx)
: ctx(ctx) { }

resource_manager::~resource_manager() { }

void resource_manager::pin(const std::string& name)
{
    std::shared_lock<std::shared_timed_mutex> lk(resources_mutex);
    resources.at(name)->pin();
}

void resource_manager::unpin(const std::string& name)
{
    std::shared_lock<std::shared_timed_mutex> lk(resources_mutex);
    resources.at(name)->unpin();
}

resource_manager::basic_resource_container::basic_resource_container(
    context& ctx
): status(UNLOADED), references(0), ctx(ctx) { }

resource_manager::basic_resource_container::~basic_resource_container()
{
}

void resource_manager::basic_resource_container::pin() const
{
    if(++references == 1)
    {
        start_load();                
    }
}

void resource_manager::basic_resource_container::unpin() const
{
    if(--references == 0)
    {
        start_unload();                
    }
}

void resource_manager::basic_resource_container::start_load() const
{
    ctx.threads.post(
        PRIORITY_PRONTO,
        [&]{
            std::unique_lock<std::mutex> lk(load_mutex);
            if(status == UNLOADED)
            {
                load_system();
                status = SYSTEM_LOADED;
            }

            if(status == SYSTEM_LOADED)
            {
                load_device();
                status = DEVICE_LOADED;
            }
        }
    );
}

void resource_manager::basic_resource_container::start_unload() const
{
    ctx.threads.post(
        PRIORITY_PRONTO,
        [&]{
            std::unique_lock<std::mutex> lk(load_mutex);
            if(status == DEVICE_LOADED)
            {
                status = SYSTEM_LOADED;
                unload_device();
            }

            if(status == SYSTEM_LOADED)
            {
                status = UNLOADED;
                unload_system();
            }
        }
    );
}
