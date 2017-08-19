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
#ifndef PONG_CONTEXT_HH
#define PONG_CONTEXT_HH
#include "config.hh"
#include <vulkan/vulkan.h>
#include <SDL2/SDL_syswm.h>
#include <cstdint>
#include <vector>
#include "vulkan_helpers.hh"
#include "thread_pool.hh"
#include "resource_manager.hh"

class context
{
public:
    /**
     * \brief Creates a context, ensuring that only one exists at a time.
     */
    context();
    context(context&& other) = delete;
    context(const context& other) = delete;
    ~context();

    thread_pool threads;
    resource_manager resources;

private:
    friend class window;

    VkInstance get_instance() const;
    SDL_SYSWM_TYPE get_wm_type() const;

    // By handling devices in the context it is easier to balance them
    // across windows.
    void allocate_device(
        VkSurfaceKHR surface,
        VkDevice& dev,
        VkPhysicalDevice& physical_device,
        queue_families& families
    );

    void free_device(
        VkSurfaceKHR surface,
        VkDevice dev
    );

    static bool& exists();

    void create_instance();
    void destroy_instance();

    bool inited_sdl;
    SDL_SYSWM_TYPE wm_type;
    VkInstance instance;
    std::vector<VkPhysicalDevice> devices;

#ifdef DEBUG
    void create_debug_callback();
    void destroy_debug_callback();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
        VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT object_type,
        uint64_t object,
        size_t location,
        int32_t message_code,
        const char* layer_prefix,
        const char* message,
        void* user_data
    );

    VkDebugReportCallbackEXT callback;
#endif
};

#endif
