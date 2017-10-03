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
#include "resource_container.hh"
#include <stdexcept>

template<typename T, typename... Args>
void resource_manager::create(const std::string& name, Args&&... args)
{
    std::unique_lock<std::shared_timed_mutex> lk(resources_mutex);
    resources[name].reset(
        new resource_container<
            typename T::system_data_type,
            typename T::device_data_type
        >(
            *this,
            std::forward<Args>(args)...
        )
    );
}

template<typename S, typename D>
resource_container<S, D>& resource_manager::get(
    const std::string& name
){
    std::shared_lock<std::shared_timed_mutex> lk(resources_mutex);

    basic_resource_container* container = resources.at(name).get();
    resource_container<S, D>* tcontainer =
        dynamic_cast<resource_container<S, D>*>(container);
    if(!tcontainer)
        throw std::runtime_error(
            "resource_manager::get(): Type mismatch for resource \""+name+"\""
        );

    return *tcontainer;
}
