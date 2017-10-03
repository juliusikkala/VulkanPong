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

template<typename S, typename D>
template<typename... Args>
resource_container<S, D>::resource_container(
    resource_manager& manager,
    Args&&... args
): basic_resource_container(manager), system_data(std::forward<Args>(args)...)
{}

template<typename S, typename D>
resource_container<S, D>::~resource_container()
{
    for(auto pair: device_data)
    {
        start_unload_device(pair.first);
    }
    unload_system();
}

template<typename S, typename D>
const S& resource_container<S, D>::system() const
{
    wait_load_system();
    return system_data;
}

template<typename S, typename D>
const D& resource_container<S, D>::device(device_id id) const
{
    wait_load_device(id);
    return device_data(id);
}

template<typename S, typename D>
void resource_container<S, D>::load_system() const
{
    system_data.load();
}

template<typename S, typename D>
void resource_container<S, D>::unload_system() const
{
    system_data.unload();
}

template<typename S, typename D>
void resource_container<S, D>::load_device(device_id id) const
{
    auto it = device_data.find(id);
    if(!it)
    {
        it = device_data.emplace(id, {id, system_data})->first;
    }
    it->second->load();
}

template<typename S, typename D>
void resource_container<S, D>::unload_device(device_id id) const
{
    auto it = device_data.find(id);
    if(it)
    {
        it->second->unload();
    }
}

