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
#ifndef PONG_RESOURCE_MANAGER_HH
#define PONG_RESOURCE_MANAGER_HH
#include <unordered_map>
#include <string>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <functional>
#include <memory>
#include <future>

class shader;
class thread_pool;
class basic_resource_container;

//Make sure this is an integral type (pointers are fine)
using device_id = void*;

class resource_manager
{
template<typename T>
friend class resource;
public:
    resource_manager(thread_pool& pool);
    resource_manager(const resource_manager& other) = delete;
    resource_manager(resource_manager&& other) = delete;
    ~resource_manager();

    template<typename T, typename... Args>
    void create(const std::string& name, Args&&... args);

    template<typename T>
    resource_container<T>& get(const std::string& name);

    void pin(const std::string& name);
    void unpin(const std::string& name);

private:
    std::shared_timed_mutex resources_mutex;
    std::unordered_map<
        std::string /*name*/,
        std::unique_ptr<basic_resource_container> /*container*/
    > resources;

    thread_pool& pool;
};

class resource_data
{
public:
    virtual ~resource_data();

    virtual void load_system();
    virtual void unload_system();

    virtual void load_device();
    virtual void unload_device();
};

#include "resource_manager.tcc"
#endif
