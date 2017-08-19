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
#include <stdexcept>

template<typename T, typename... Args>
void resource_manager::create(const std::string& name, Args&&... args)
{
    std::unique_lock<std::shared_timed_mutex> lk(resources_mutex);
    resources[name] = new resource_container<typename T::data>(
        ctx,
        std::forward<Args>(args)...
    );
}

template<typename T>
resource_manager::resource_container<T>& resource_manager::get(
    const std::string& name
){
    std::shared_lock<std::shared_timed_mutex> lk(resources_mutex);

    basic_resource_container* container = resources.at(name);
    resource_container<T>* tcontainer = dynamic_cast<resource_container<T>*>(
        container
    );
    if(!tcontainer)
        throw std::runtime_error(
            "resource_manager::get(): Type mismatch for resource \""+name+"\""
        );

    return *tcontainer;
}

template<typename T>
template<typename... Args>
resource_manager::resource_container<T>::resource_container(
    context& ctx, 
    Args&&... args
): basic_resource_container(ctx), data(std::forward<Args>(args)...) {}

template<typename T>
resource_manager::resource_container<T>::~resource_container() {
    std::unique_lock<std::mutex> lk(load_mutex);
    if(status == UNLOADED)
    {
        status = SYSTEM_LOADED;
        unload_device();

        status = UNLOADED;
        unload_system();
    }
}

template<typename T>
void resource_manager::resource_container<T>::wait_load_system() const
{
    if(status == SYSTEM_LOADED || status == DEVICE_LOADED)
        return;

    std::unique_lock<std::mutex> lk(load_mutex);

    system_loaded.wait(
        lk,
        [&]{return status == SYSTEM_LOADED || status == DEVICE_LOADED;}
    );
}

template<typename T>
void resource_manager::resource_container<T>::wait_load_device() const
{
    if(status == DEVICE_LOADED)
        return;

    std::unique_lock<std::mutex> lk(load_mutex);

    system_loaded.wait(
        lk,
        [&]{return status == DEVICE_LOADED;}
    );
}

template<typename T>
const T& resource_manager::resource_container<T>::system() const
{
    wait_load_system();
    return *data;
}

template<typename T>
const T& resource_manager::resource_container<T>::device() const
{
    wait_load_device();
    return *data;
}

template<typename T>
void resource_manager::resource_container<T>::load_system() const
{
    data.load_system(ctx);
}

template<typename T>
void resource_manager::resource_container<T>::unload_system() const
{
    data.unload_system(ctx);
}

template<typename T>
void resource_manager::resource_container<T>::load_device() const
{
    data.unload_device(ctx);
}

template<typename T>
void resource_manager::resource_container<T>::unload_device() const
{
    data.unload_device(ctx);
}
