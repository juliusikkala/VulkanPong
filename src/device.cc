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
#include "vulkan_helpers.hh"
#include "window.hh"
#include "context.hh"

device::device(context& ctx, window& win)
{
    std::vector<VkPhysicalDevice> suitable = find_vulkan_devices(
        ctx.get_instance(),
        win.get_surface()
    );

    if(suitable.size() == 0)
    {
        throw std::runtime_error("Failed to find a GPU with Vulkan support");
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

    std::vector<VkDeviceQueueCreateInfo> queue_infos;

    float priority = 1.0f;
    if(families.graphics_index >= 0)
    {
        VkDeviceQueueCreateInfo graphics = {};
        graphics.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        graphics.queueFamilyIndex = families.graphics_index;
        graphics.queueCount = 1;
        graphics.pQueuePriorities = &priority;

        queue_infos.push_back(graphics);
    }

    if(families.present_index >= 0
       && families.graphics_index != families.present_index)
    {
        //TODO: Check if having an idle compute queue affects performance
        VkDeviceQueueCreateInfo present = {};
        present.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        present.queueFamilyIndex = families.present_index;
        present.queueCount = 1;
        present.pQueuePriorities = &priority;
        
        queue_infos.push_back(present);
    }

    VkPhysicalDeviceFeatures features = {};

    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = queue_infos.size();
    create_info.pQueueCreateInfos = queue_infos.data();
    create_info.pEnabledFeatures = &features;
    create_info.ppEnabledLayerNames = config::validation_layers;
    create_info.enabledLayerCount = config::validation_layers_count;

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
