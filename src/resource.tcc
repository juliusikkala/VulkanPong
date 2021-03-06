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
#include "resource.hh"
#include "context.hh"
#include "resource_manager.hh"

template<typename S, typename D>
resource<S, D>::resource(
    context& ctx,
    const std::string& resource_name
): resource(ctx.resources, resource_name) { }

template<typename S, typename D>
resource<S, D>::resource(
    resource_manager& manager,
    const std::string& resource_name
): data_container(manager.get<S, D>(resource_name))
{
    data_container.pin();
}

template<typename S, typename D>
resource<S, D>::resource(const resource<S, D>& other)
: data_container(other.data_container)
{
    data_container.pin();
}

template<typename S, typename D>
resource<S, D>::~resource()
{
    data_container.unpin();
}

template<typename S, typename D>
void resource<S, D>::wait_load_system() const
{
    data_container.wait_load_system();
}

template<typename S, typename D>
void resource<S, D>::wait_load_device(device_id id) const
{
    data_container.wait_load_device(id);
}

template<typename S, typename D>
const S& resource<S, D>::system() const
{
    return data_container.system();
}

template<typename S, typename D>
const D& resource<S, D>::device(device_id id) const
{
    return data_container.device(id);
}

