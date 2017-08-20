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
#include <iostream>
#include "context.hh"
#include "window.hh"
#include "thread_pool.hh"
#include "resource.hh"

struct test_data: public resource_data
{
    test_data(unsigned hs): heavy_stuff(hs) {}
    unsigned heavy_stuff;

    void load_system() override
    {
        std::cout<<"load system"<<std::endl;
    }

    void unload_system() override
    {
        std::cout<<"unload system"<<std::endl;
    }

    void load_device() override
    {
        std::cout<<"load device"<<std::endl;
    }

    void unload_device() override
    {
        std::cout<<"unload device"<<std::endl;
    }
};

class test: public resource<test_data>
{
public:
    test(context& ctx, const std::string& name)
    : resource(ctx, name)
    {}

    void show()
    {
        std::cout << system().heavy_stuff << std::endl;
    }
};

int main()
{
    context ctx;
    window win(ctx);
    ctx.resources.create<test>("test", 1);

    test t1(ctx, "test");
    test t2(ctx, "test");
    t1.show();
    t2.show();

    std::cout<<"This will become a pong game some day..."<<std::endl;
    return 0;
}
