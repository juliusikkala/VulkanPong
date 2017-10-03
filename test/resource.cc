#include <iostream>
#include <gtest/gtest.h>
#include "resource.hh"
//TODO: Fix tests

/*
struct test_data_a: public resource_data
{
    test_data_a(unsigned i): i(i), system_init(false), device_init(false) {}
    unsigned i;
    bool system_init, device_init;

    void load_system() override
    {
        EXPECT_FALSE(system_init);
        EXPECT_FALSE(device_init);
        system_init = true;
    }

    void unload_system() override
    {
        EXPECT_TRUE(system_init);
        EXPECT_FALSE(device_init);
        system_init = false;
    }

    void load_device() override
    {
        EXPECT_TRUE(system_init);
        EXPECT_FALSE(device_init);
        device_init = true;
    }

    void unload_device() override
    {
        EXPECT_TRUE(system_init);
        EXPECT_TRUE(device_init);
        device_init = false;
    }
};

class test_a: public resource<test_data_a>
{
public:
    test_a(resource_manager& res, const std::string& name)
    : resource(res, name) {}
};

struct test_data_b: public resource_data
{
    test_data_b(unsigned i): i(i) {}
    unsigned i;
};

class test_b: public resource<test_data_b>
{
public:
    test_b(resource_manager& res, const std::string& name)
    : resource(res, name) {}
};

class ResourceTest: public ::testing::Test {
protected:
    ResourceTest()
    : pool(8), manager(pool)
    {}

    thread_pool pool;
    resource_manager manager;
};

TEST_F(ResourceTest, CreateTest)
{
    unsigned times = 1000;
    for(unsigned i = 0; i < times; ++i)
        if(i&1)
            manager.create<test_a>("Test"+std::to_string(i), i);
        else
            manager.create<test_b>("Test"+std::to_string(i), i);

    for(unsigned i = 0; i < times; ++i)
        if(i&1)
            manager.get<test_data_a>("Test"+std::to_string(i));
        else
            manager.get<test_data_b>("Test"+std::to_string(i));

    for(unsigned i = 0; i < times; ++i)
    {
        bool threw = false;
        if(i&1)
        {
            try { manager.get<test_data_b>("Test"+std::to_string(i)); }
            catch(...) { threw = true; }
        }
        else
        {
            try { manager.get<test_data_a>("Test"+std::to_string(i)); }
            catch(...) { threw = true; }
        }
        ASSERT_TRUE(threw);
    }
}

TEST_F(ResourceTest, PinTest)
{
    manager.create<test_a>("TestA", 0);
    manager.create<test_b>("TestB", 1);
    
    for(unsigned i = 0; i < 1000; ++i)
    {
        test_a a(manager, "TestA");
        test_b b(manager, "TestB");
    }
}
*/
