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
#include <iostream>
#include <set>
#include "vulkan_helpers.hh"

constexpr const char* device_extensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
constexpr unsigned device_extensions_count =
    sizeof(device_extensions) / sizeof(const char*);

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

static int rate_device(VkPhysicalDevice device)
{
    // Make sure required device extensions are available
    if(!have_vulkan_device_extensions(
            device, 
            device_extensions,
            device_extensions_count
        ))
        return -1;

    return rate_vulkan_device(device);
}

context::context()
: resources(threads)
{
    if(exists()) throw std::runtime_error("A context already exists.");

    if(SDL_Init(SDL_INIT_EVERYTHING))
    {
        throw std::runtime_error(SDL_GetError());
    }
    inited_sdl = true;

    wm_type = ::get_wm_type();

    create_instance();
#ifdef DEBUG
    create_debug_callback();
#endif

    devices = find_vulkan_devices(instance, rate_device);

    if(devices.size() == 0)
    {
        throw std::runtime_error(
            "Failed to find a device with the required Vulkan extensions"
        );
    }
}

context::~context()
{
#ifdef DEBUG
    destroy_debug_callback();
#endif

    destroy_instance();

    if(inited_sdl)
        SDL_Quit();
}

VkInstance context::get_instance() const
{
    return instance;
}

SDL_SYSWM_TYPE context::get_wm_type() const
{
    return wm_type;
}

void context::allocate_device(
    VkSurfaceKHR surface,
    VkDevice& dev,
    VkPhysicalDevice& physical_device,
    queue_families& families
) {
    physical_device = VK_NULL_HANDLE;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> modes;

    for(VkPhysicalDevice device: devices)
    {
        // Make sure required queue families are available
        families = find_queue_families(device, surface);
        if(families.graphics_index < 0 || families.present_index < 0)
            continue;

        formats = find_surface_formats(
            device,
            surface
        );

        modes = get_compatible_present_modes(
            device,
            surface
        );

        // Must have at least one format and mode compatible with the surface
        if(formats.empty() || modes.empty())
            continue;

        physical_device = device;
        break;
    }

    if(physical_device == VK_NULL_HANDLE)
    {
        throw std::runtime_error(
            "Failed to find a device compatible with the surface!"
        );
    }

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physical_device, &properties);

    std::cout << properties.deviceName
              << std::endl;

    std::set<int> unique_families = {
        families.graphics_index,
        families.present_index
    };
    std::vector<VkDeviceQueueCreateInfo> queue_infos;

    float priority = 1.0f;
    for(int index: unique_families)
    {
        VkDeviceQueueCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueFamilyIndex = index;
        info.queueCount = 1;
        info.pQueuePriorities = &priority;

        queue_infos.push_back(info);
    }

    VkPhysicalDeviceFeatures features = {};

    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = queue_infos.size();
    create_info.pQueueCreateInfos = queue_infos.data();
    create_info.pEnabledFeatures = &features;
    create_info.ppEnabledLayerNames = config::validation_layers;
    create_info.enabledLayerCount = config::validation_layers_count;
    create_info.ppEnabledExtensionNames = device_extensions;
    create_info.enabledExtensionCount = device_extensions_count;

    VkResult err;
    if((err = vkCreateDevice(
            physical_device,
            &create_info,
            nullptr,
            &dev
        )) != VK_SUCCESS)
    {
        throw std::runtime_error(
            "Failed to create a logical device: "
            + get_vulkan_result_string(err)
        );
    }
}

void context::free_device(
    VkSurfaceKHR surface,
    VkDevice dev
) {
    vkDestroyDevice(dev, nullptr);
}

bool& context::exists()
{
    static bool e = false;
    return e;
}

void context::create_instance()
{
    unsigned version = VK_MAKE_VERSION(
        config::major,
        config::minor,
        config::patch
    );

    std::vector<const char*> extensions({
        VK_KHR_SURFACE_EXTENSION_NAME,
    #ifdef DEBUG
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    #endif
    });

    switch(wm_type)
    {
#ifdef SDL_VIDEO_DRIVER_WINDOWS
    case SDL_SYSWM_WINDOWS:
        extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
        break;
#endif
#ifdef SDL_VIDEO_DRIVER_X11
    case SDL_SYSWM_X11:
        extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
        break;
#endif
#ifdef SDL_VIDEO_DRIVER_WAYLAND
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
    create_info.enabledLayerCount = config::validation_layers_count;
    create_info.ppEnabledLayerNames = config::validation_layers;
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
}

void context::destroy_instance()
{
    if(instance)
    {
        vkDestroyInstance(instance, nullptr);
        instance = VK_NULL_HANDLE;
    }
}

#ifdef DEBUG
void context::create_debug_callback()
{
    VkDebugReportCallbackCreateInfoEXT info = {};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    info.flags =
        VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    info.pfnCallback = debug_callback;

    VkResult err;
    if((err = create_debug_report_callback(
            instance,
            &info,
            nullptr,
            &callback
        )) != VK_SUCCESS)
    {
        throw std::runtime_error(
            "Failed to create debug callback: "
            + get_vulkan_result_string(err)
        );
    }
}

void context::destroy_debug_callback()
{
    if(callback)
    {
        destroy_debug_report_callback(instance, callback, nullptr);
        callback = VK_NULL_HANDLE;
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL context::debug_callback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT object_type,
    uint64_t object,
    size_t location,
    int32_t message_code,
    const char* layer_prefix,
    const char* message,
    void* user_data
) {
    std::cerr << layer_prefix << ": " << message << std::endl;

    return VK_FALSE;
}
#endif
