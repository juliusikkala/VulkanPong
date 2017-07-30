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
#ifndef PONG_VULKAN_HELPERS_HH
#define PONG_VULKAN_HELPERS_HH
#include "config.hh"
#include <vulkan/vulkan.h>
#include <cstdint>
#include <string>
#include <vector>
#include <functional>

std::string get_vulkan_result_string(VkResult result);

void ensure_vulkan_instance_layers(
    const char* const* required_layers,
    uint32_t required_layers_count
);

void ensure_vulkan_instance_extensions(
    const char* const* required_extensions,
    uint32_t required_extensions_count
);


VkResult create_debug_report_callback(
    VkInstance instance,
    const VkDebugReportCallbackCreateInfoEXT* create_info,
    const VkAllocationCallbacks* allocator,
    VkDebugReportCallbackEXT* callback
);

void destroy_debug_report_callback(
    VkInstance instance,
    VkDebugReportCallbackEXT callback,
    const VkAllocationCallbacks* allocator
);

void destroy_debug_report_callback(
    VkInstance instance,
    VkDebugReportCallbackEXT callback,
    const VkAllocationCallbacks* allocator
);

// A rating callback must return a higher number for a better device and
// negative values for unsuitable devices.
using rate_vulkan_device_callback =
    std::function<int(VkPhysicalDevice, VkSurfaceKHR)>;

// Rates a device based on vendor and device type, preferring discrete GPUs and
// AMD and Nvidia over Intel.
int rate_vulkan_device(
    VkPhysicalDevice device,
    VkSurfaceKHR surface
);

std::vector<VkPhysicalDevice> find_vulkan_devices(
    VkInstance instance,
    VkSurfaceKHR surface,
    rate_vulkan_device_callback rate = rate_vulkan_device
);


struct queue_families
{
    int graphics_index;
    int compute_index;
    int present_index;
};
queue_families find_queue_families(
    VkPhysicalDevice device,
    VkSurfaceKHR surface
);

#endif
