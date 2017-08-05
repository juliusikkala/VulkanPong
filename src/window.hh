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
#ifndef PONG_WINDOW_HH
#define PONG_WINDOW_HH
#include "config.hh"
#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>
#include "vulkan_helpers.hh"

class context;
class window
{
public:
    struct parameters
    {
        parameters() {}
        const char* title = config::name;
        unsigned w = 640, h = 480;
        bool fullscreen = false;
        bool vsync = true;
    };

    window(context& ctx, const parameters& p = parameters());
    
    window(window&& other);
    ~window();

private:
    void create_device();
    void destroy_device();

    void create_swapchain();
    void destroy_swapchain();


    VkSurfaceKHR get_surface() const;

    parameters params;

    context& ctx;
    SDL_Window* win;
    VkSurfaceKHR surface;
    VkSurfaceCapabilitiesKHR surface_capabilities;

    VkDevice dev;
    VkPhysicalDevice physical_device;
    queue_families families;
    VkQueue graphics_queue, present_queue;
    
    VkSwapchainKHR swapchain;
};

#endif
