#include <iostream>
#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include "thread_pool.hh"
#define POOL_SIZE 8
using namespace std::chrono_literals;

TEST(ThreadPoolTest, ConstructorTest)
{
    {
        thread_pool pool;
        ASSERT_EQ(pool.size(), std::thread::hardware_concurrency()-1);
    }
    for(unsigned i = 1; i < 100; ++i)
    {
        thread_pool pool(i);
        ASSERT_EQ(pool.size(), i);
    }
}

TEST(ThreadPoolTest, PostTest)
{
    thread_pool pool(POOL_SIZE);

    auto res = pool.post(
      [](const std::string& arg){
        return arg == "How are you?" ? "I'm fine, thank you." : "How rude.";
      },
      "How are you?"
    );
    ASSERT_STREQ(res.get(), "I'm fine, thank you.");

    std::atomic_uint u(0);
    unsigned val = 1<<10;
    for(unsigned i = 0; i < val; ++i)
    {
        pool.postp(i, [&u](){ u++; });
    }
    pool.finish();
    ASSERT_EQ(u, val);
}

TEST(ThreadPoolTest, SynchronousTest)
{
    thread_pool pool(0);
    pool.post(
        [&](std::thread::id id){
            ASSERT_EQ(id, std::this_thread::get_id());
            ASSERT_EQ(pool.busy(), false);
        },
        std::this_thread::get_id()
    ).wait();
    ASSERT_EQ(pool.size(), 0);
    for(unsigned i = 0; i < 100; ++i)
    {
        pool.post([](){});
    }
    ASSERT_EQ(pool.size(), 0);
    ASSERT_EQ(pool.busy(), false);
    pool.finish();
}

TEST(ThreadPoolTest, FinishTest)
{
    {
        thread_pool pool(POOL_SIZE);
        pool.finish();

        for(unsigned i = 0; i < 1024; ++i)
        {
            pool.post([](){});
        }
        pool.finish();
    }

    {
        thread_pool pool;
        auto start = std::chrono::high_resolution_clock::now();
        for(unsigned i = 0; i < pool.size(); ++i)
        {
            pool.post([](){
                std::this_thread::sleep_for(250ms);
            });
        }
        pool.finish();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = end-start;
        ASSERT_GT(duration, 250ms);
        ASSERT_LT(duration, 300ms);
    }
}

TEST(ThreadPoolTest, ResizeTest)
{
    thread_pool pool(POOL_SIZE);

    auto start = std::chrono::high_resolution_clock::now();
    pool.post([](){ std::this_thread::sleep_for(250ms); });
    pool.resize(POOL_SIZE+2);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = end-start;
    ASSERT_LT(duration, 250ms);

    pool.finish();

    start = std::chrono::high_resolution_clock::now();
    pool.resize(POOL_SIZE);
    end = std::chrono::high_resolution_clock::now();
    duration = end-start;
    ASSERT_LT(duration, 1ms);

    start = std::chrono::high_resolution_clock::now();
    for(unsigned i = 0; i < POOL_SIZE; ++i)
    {
        pool.post([](){ std::this_thread::sleep_for(100ms); });
    }
    while(pool.busy() < POOL_SIZE);
    pool.resize(POOL_SIZE-2);
    end = std::chrono::high_resolution_clock::now();
    duration = end-start;
    ASSERT_GT(duration, 100ms);

    pool.finish();

    for(unsigned i = 0; i < 100; ++i)
    {
        pool.resize(i);
        ASSERT_EQ(pool.size(), i);
    }
    for(unsigned i = 100; i > 0; --i)
    {
        pool.resize(i);
        ASSERT_EQ(pool.size(), i);
    }
}

TEST(ThreadPoolTest, BusyTest)
{
    thread_pool pool(1);
    ASSERT_EQ(pool.busy(), 0);

    pool.post([](){ std::this_thread::sleep_for(10ms); });
    while(!pool.busy());

    pool.finish();
    ASSERT_EQ(pool.busy(), 0);
}
