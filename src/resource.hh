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
#ifndef PONG_RESOURCE_HH
#define PONG_RESOURCE_HH
#include <string>
#include "resource_manager.hh"

class context;
template<typename T>
class resource
{
public:
    using resource_data_type = T;

    resource(
        context& ctx,
        const std::string& resource_name
    );
    resource(
        resource_manager& manager,
        const std::string& resource_name
    );
    resource(const resource<T>& other);
    ~resource();

    void wait_load_system() const;
    void wait_load_device() const;

protected:
    // Use this to access system data only
    const T& system() const;
    // Use this to access device and system data
    const T& device() const;

private:
    resource_manager::resource_container<T>& data_container;
};

#include "resource.tcc"
#endif
