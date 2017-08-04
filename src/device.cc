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
#include "device.hh"
#include <iostream>
#include <set>
#include "vulkan_helpers.hh"
#include "window.hh"
#include "context.hh"

constexpr const char* device_extensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
constexpr unsigned device_extensions_count =
    sizeof(device_extensions) / sizeof(const char*);

device::device(context& ctx, window& win)
{
    using namespace std::placeholders;

    std::vector<VkPhysicalDevice> suitable = find_vulkan_devices(
        ctx.get_instance(),
        std::bind(rate_device, _1, std::ref(win))
    );

    if(suitable.size() == 0)
    {
        throw std::runtime_error(
            "Failed to find a device with the required Vulkan extensions"
        );
    }

    VkPhysicalDevice physical_device = suitable[0];

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physical_device, &properties);

    std::cout << properties.deviceName
              << std::endl;

    queue_families families = find_queue_families(
        physical_device,
        win.get_surface()
    );

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

    vkGetDeviceQueue(dev, families.graphics_index, 0, &graphics_queue);
    vkGetDeviceQueue(dev, families.present_index, 0, &present_queue);
}

device::device(device&& other)
: dev(other.dev), graphics_queue(other.graphics_queue),
  present_queue(other.present_queue)
{
    other.dev = VK_NULL_HANDLE;
    other.graphics_queue = VK_NULL_HANDLE;
    other.present_queue = VK_NULL_HANDLE;
}

device::~device()
{
    if(dev)
    {
        vkDestroyDevice(dev, nullptr);
        dev = VK_NULL_HANDLE;
    }
}

int device::rate_device(VkPhysicalDevice device, window& win)
{
    // Make sure required queue families are available
    queue_families families = find_queue_families(device, win.get_surface());
    if(families.graphics_index < 0 || families.present_index < 0)
        return -1;

    std::vector<VkSurfaceFormatKHR> formats = get_compatible_surface_formats(
        device,
        win.get_surface()
    );

    std::vector<VkPresentModeKHR> modes = get_compatible_present_modes(
        device,
        win.get_surface()
    );

    // Must have at least one format and mode compatible with the surface
    if(formats.empty() || modes.empty())
        return -1;

    // Make sure required device extensions are available
    if(!have_vulkan_device_extensions(
            device, 
            device_extensions,
            device_extensions_count
        ))
        return -1;

    return rate_vulkan_device(device);
}
