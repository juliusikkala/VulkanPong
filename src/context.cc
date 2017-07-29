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
#include "context.hh"
#include <stdexcept>
#include <SDL2/SDL.h>
#include "vulkan_helpers.hh"

SDL_SYSWM_TYPE get_wm_type()
{
    SDL_SysWMinfo info;
    // HACK!
    SDL_Window* win = SDL_CreateWindow("", 0, 0, 0, 0, SDL_WINDOW_HIDDEN);
    SDL_VERSION(&info.version);

    if(!SDL_GetWindowWMInfo(win, &info))
    {
        info.subsystem = SDL_SYSWM_UNKNOWN;
    }

    SDL_DestroyWindow(win);
    return info.subsystem;
}

context::context()
{
    if(exists()) throw std::runtime_error("A context already exists.");

    if(SDL_Init(SDL_INIT_EVERYTHING))
    {
        throw std::runtime_error(SDL_GetError());
    }
    inited_sdl = true;

    wm_type = get_wm_type();

    unsigned version = VK_MAKE_VERSION(
        config::major,
        config::minor,
        config::patch
    );

    std::vector<const char*> layers;
    if(config::validate)
        layers.push_back("VK_LAYER_LUNARG_standard_validation");

    std::vector<const char*> extensions({
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    });

    switch(wm_type)
    {
    #ifdef USE_WIN32
    case SDL_SYSWM_WINDOWS:
        extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
        break;
    #endif
    #ifdef USE_UNIX
    case SDL_SYSWM_X11:
        extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
        break;
    case SDL_SYSWM_WAYLAND:
        extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
        break;
    #endif
    default:
        throw std::runtime_error(
            "Unsupported WM type: " + std::to_string(wm_type)
        );
    }

    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = config::name;
    app_info.applicationVersion = version;
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = version;
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledLayerCount = layers.size();
    create_info.ppEnabledLayerNames = layers.data();
    create_info.enabledExtensionCount = extensions.size();
    create_info.ppEnabledExtensionNames = extensions.data();

    ensure_vulkan_instance_layers(
        create_info.ppEnabledLayerNames,
        create_info.enabledLayerCount
    );

    ensure_vulkan_instance_extensions(
        create_info.ppEnabledExtensionNames,
        create_info.enabledExtensionCount
    );

    VkResult err;
    if((err = vkCreateInstance(
            &create_info,
            nullptr,
            &instance
        )) != VK_SUCCESS)
    {
        throw std::runtime_error(
            "Failed to create Vulkan instance: "
            + get_vulkan_result_string(err)
        );
    }

    return;
}

context::context(context&& other)
: inited_sdl(other.inited_sdl), wm_type(other.wm_type), instance(other.instance)
{
    other.inited_sdl = false;
    other.instance = nullptr;
}

context::~context()
{
    if(instance)
        vkDestroyInstance(instance, nullptr);

    if(inited_sdl)
        SDL_Quit();
}

bool& context::exists()
{
    static bool e = false;
    return e;
}
