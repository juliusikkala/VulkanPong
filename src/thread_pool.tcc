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
#include <utility>
#include <typeinfo>
#include <iostream>

template<typename F, typename... Args>
auto thread_pool::post(F&& f, Args&&... args)
-> std::future<decltype(f(std::forward<Args>(args)...))>
{
    return postp(0, std::forward<F>(f), std::forward<Args>(args)...);
}

template<typename F, typename R, typename... Args>
void set(std::promise<R>& p, F&& f, Args&&... args)
{
    p.set_value(f(std::forward<Args>(args)...));
}

template<typename F, typename... Args>
void set(std::promise<void>& p, F&& f, Args&&... args)
{
    f(std::forward<Args>(args)...);
    p.set_value();
}

template<typename F, typename... Args>
auto thread_pool::postp(
    unsigned priority,
    F&& f,
    Args&&... args
) -> std::future<decltype(f(std::forward<Args>(args)...))>
{
    auto p = std::make_shared<
        std::promise<decltype(f(std::forward<Args>(args)...))>
    >();

    std::future<decltype(f(std::forward<Args>(args)...))> future =
        p->get_future();

    post({
        [p, f, args...]() mutable {
            set(*p, f, std::forward<Args>(args)...);
        },
        priority,
        false
    });

    return future;
}

