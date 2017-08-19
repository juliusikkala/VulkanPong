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

template<typename T>
resource<T>::resource(
    context& ctx,
    const std::string& resource_name
): data_container(ctx.resources.get<T>(resource_name))
{
    data_container.pin();
}

template<typename T>
resource<T>::resource(const resource<T>& other)
: data_container(other.data_container)
{
    data_container.pin();
}

template<typename T>
resource<T>::~resource()
{
    data_container.unpin();
}

template<typename T>
void resource<T>::wait_load_system() const
{
    data_container.wait_load_system();
}

template<typename T>
void resource<T>::wait_load_device() const
{
    data_container.wait_load_device();
}

template<typename T>
const T& resource<T>::system() const
{
    return data_container.system();
}

template<typename T>
const T& resource<T>::device() const
{
    return data_container.device();
}
