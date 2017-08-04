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
#include "vulkan_helpers.hh"
#include <stdexcept>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include "config.hh"

std::string get_vulkan_result_string(VkResult result)
{
    switch(result)
    {
    case VK_SUCCESS:
        return "Command successfully completed";
    case VK_NOT_READY:
        return "A fence or query has not yet completed";
    case VK_TIMEOUT:
        return "A wait operation has not completed in the specified time";
    case VK_EVENT_SET:
        return "An event is signaled";
    case VK_EVENT_RESET:
        return "An event is unsignaled";
    case VK_INCOMPLETE:
        return "A return array was too small for the result";
    case VK_ERROR_OUT_OF_HOST_MEMORY:
        return "A host memory allocation has failed";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        return "A device memory allocation has failed";
    case VK_ERROR_INITIALIZATION_FAILED:
        return "Initialization of an object could not be completed for implementation-specific reasons";
    case VK_ERROR_DEVICE_LOST:
        return "The logical or physical device has been lost";
    case VK_ERROR_MEMORY_MAP_FAILED:
        return "Mapping of a memory object has failed";
    case VK_ERROR_LAYER_NOT_PRESENT:
        return "A requested layer is not present or could not be loaded";
    case VK_ERROR_EXTENSION_NOT_PRESENT:
        return "A requested extension is not supported";
    case VK_ERROR_FEATURE_NOT_PRESENT:
        return "A requested feature is not supported";
    case VK_ERROR_INCOMPATIBLE_DRIVER:
        return "The requested version of Vulkan is not supported by the driver or is otherwise incompatible for implementation-specific reasons";
    case VK_ERROR_TOO_MANY_OBJECTS:
        return "Too many objects of the type have already been created";
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
        return "A requested format is not supported on this device";
    case VK_ERROR_FRAGMENTED_POOL:
        return "A pool allocation has failed due to fragmentation of the pool’s memory";
    case VK_ERROR_SURFACE_LOST_KHR:
        return "A surface is no longer available";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        return "The requested window is already in use by Vulkan or another API in a manner which prevents it from being used again";
    case VK_SUBOPTIMAL_KHR:
        return "A swapchain no longer matches the surface properties exactly, but can still be used to present to the surface successfully";
    case VK_ERROR_OUT_OF_DATE_KHR:
        return "A surface has changed in such a way that it is no longer compatible with the swapchain, and further presentation requests using the swapchain will fail";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
        return "The display used by a swapchain does not use the same presentable image layout, or is incompatible in a way that prevents sharing an image";
    case VK_ERROR_OUT_OF_POOL_MEMORY_KHR:
        return "A pool memory allocation has failed";
    default:
        break;
    }
    if(result < 0)
    {
        return "Unknown Vulkan error code: " + std::to_string((int)result);
    }
    return "Unknown Vulkan success code: " + std::to_string((int)result);
}

void ensure_vulkan_instance_layers(
    const char* const* required_layers,
    uint32_t required_layers_count
) {
    if(required_layers_count == 0) return;

    uint32_t count = 0;

    VkResult err;

    if((err = vkEnumerateInstanceLayerProperties(&count, nullptr)) != VK_SUCCESS)
    {
        throw std::runtime_error(
            "Failed to enumerate Vulkan instance layers: "
            + get_vulkan_result_string(err)
        );
    }

    std::vector<VkLayerProperties> available_layers(count);
    if((err = vkEnumerateInstanceLayerProperties(
            &count,
            available_layers.data()
        )) != VK_SUCCESS)
    {
        throw std::runtime_error(
            "Failed to get Vulkan instance layer properties: "
            + get_vulkan_result_string(err)
        );
    }

    for(unsigned i = 0; i < required_layers_count; ++i)
    {
        const char* required = required_layers[i];
        bool found = false;
        for(VkLayerProperties& available: available_layers)
        {
            if(strcmp(required, available.layerName) == 0)
            {
                found = true;
                break;
            }
        }
        if(!found)
        {
            throw std::runtime_error(
                "Failed to find required Vulkan instance layer: "
                + std::string(required)
            );
        }
    }
}

void ensure_vulkan_instance_extensions(
    const char* const* required_extensions,
    uint32_t required_extensions_count
) {
    if(required_extensions_count == 0) return;

    uint32_t count = 0;

    VkResult err;

    if((err = vkEnumerateInstanceExtensionProperties(
            nullptr,
            &count,
            nullptr
        )) != VK_SUCCESS)
    {
        throw std::runtime_error(
            "Failed to enumerate Vulkan instance extensions: "
            + get_vulkan_result_string(err)
        );
    }

    std::vector<VkExtensionProperties> available_extensions(count);

    if((err = vkEnumerateInstanceExtensionProperties(
            nullptr,
            &count,
            available_extensions.data()
        )) != VK_SUCCESS)
    {
        throw std::runtime_error(
            "Failed to get Vulkan instance extension properties: "
            + get_vulkan_result_string(err)
        );
    }

    for(unsigned i = 0; i < required_extensions_count; ++i)
    {
        const char* required = required_extensions[i];
        bool found = false;
        for(VkExtensionProperties& available: available_extensions)
        {
            if(strcmp(required, available.extensionName) == 0)
            {
                found = true;
                break;
            }
        }
        if(!found)
        {
            throw std::runtime_error(
                "Failed to find required Vulkan instance extension: "
                + std::string(required)
            );
        }
    }
}

bool have_vulkan_device_extensions(
    VkPhysicalDevice device,
    const char* const* required_extensions,
    uint32_t required_extensions_count
) {
    if(required_extensions_count == 0) return true;

    uint32_t count = 0;

    VkResult err;

    if((err = vkEnumerateDeviceExtensionProperties(
            device,
            nullptr,
            &count,
            nullptr
        )) != VK_SUCCESS)
    {
        throw std::runtime_error(
            "Failed to enumerate Vulkan device extensions: "
            + get_vulkan_result_string(err)
        );
    }

    std::vector<VkExtensionProperties> available_extensions(count);

    if((err = vkEnumerateDeviceExtensionProperties(
            device,
            nullptr,
            &count,
            available_extensions.data()
        )) != VK_SUCCESS)
    {
        throw std::runtime_error(
            "Failed to get Vulkan device extension properties: "
            + get_vulkan_result_string(err)
        );
    }

    for(unsigned i = 0; i < required_extensions_count; ++i)
    {
        const char* required = required_extensions[i];
        bool found = false;
        for(VkExtensionProperties& available: available_extensions)
        {
            if(strcmp(required, available.extensionName) == 0)
            {
                found = true;
                break;
            }
        }
        if(!found) return false;
    }
    return true;
}

VkResult create_debug_report_callback(
    VkInstance instance,
    const VkDebugReportCallbackCreateInfoEXT* create_info,
    const VkAllocationCallbacks* allocator,
    VkDebugReportCallbackEXT* callback
) {
    PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT =
        (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(
            instance,
            "vkCreateDebugReportCallbackEXT"
        );
    if(vkCreateDebugReportCallbackEXT)
        return vkCreateDebugReportCallbackEXT(instance, create_info, allocator, callback);
    else return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void destroy_debug_report_callback(
    VkInstance instance,
    VkDebugReportCallbackEXT callback,
    const VkAllocationCallbacks* allocator
) {
    PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT =
        (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(
            instance,
            "vkDestroyDebugReportCallbackEXT"
        );
    if(vkDestroyDebugReportCallbackEXT)
        vkDestroyDebugReportCallbackEXT(instance, callback, allocator);
}

int rate_vulkan_device(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceProperties(device, &properties);
    vkGetPhysicalDeviceFeatures(device, &features);

    int score = 0;

    switch(properties.vendorID)
    {
    case 0x1002: //AMD
    case 0x10DE: //NVIDIA
        score += 100;
        break;
    case 0x8086: //Intel (I see what you did there with that ID :D)
        score += 50;
        break;
    case 0x5143: //Qualcomm
        score += 40;
        break;
    default:
        score += 10;
        break;
    }

    switch(properties.deviceType)
    {
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        score += 100;
        break;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        score += 90;
        break;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        score += 50;
        break;
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        score += 10;
        break;
    default:
        break;
    }

    return score;
}

std::vector<VkPhysicalDevice> find_vulkan_devices(
    VkInstance instance,
    rate_vulkan_device_callback rate
) {
    uint32_t device_count = 0;
    VkResult err;
    if((err = vkEnumeratePhysicalDevices(
            instance,
            &device_count, 
            nullptr
        )) != VK_SUCCESS)
    {
        throw std::runtime_error(
            "Failed to enumerate devices: "
            + get_vulkan_result_string(err)
        );
    }

    if(device_count == 0)
    {
        return {};
    }

    std::vector<VkPhysicalDevice> all_devices(device_count);
    using scored_device = std::pair<VkPhysicalDevice, int>;
    std::vector<scored_device> scored_devices;
    scored_devices.reserve(device_count);

    if((err = vkEnumeratePhysicalDevices(
            instance,
            &device_count, 
            all_devices.data()
        )) != VK_SUCCESS)
    {
        throw std::runtime_error(
            "Failed to get devices: "
            + get_vulkan_result_string(err)
        );
    }

    for(VkPhysicalDevice device: all_devices)
        scored_devices.push_back({device, rate(device)});

    std::sort(
        scored_devices.begin(),
        scored_devices.end(),
        [](const scored_device& a, const scored_device& b){
            return a.second > b.second;
        }
    );
    
    std::vector<VkPhysicalDevice> suitable_devices;

    for(scored_device device: scored_devices)
    {
        if(device.second >= 0)
        {
            suitable_devices.push_back(device.first);
        }
    }

    return suitable_devices;
}

queue_families find_queue_families(
    VkPhysicalDevice device,
    VkSurfaceKHR surface
) {
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);

    std::vector<VkQueueFamilyProperties> families(count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

    queue_families found_families = {-1};

    for(unsigned i = 0; i < families.size(); ++i)
    {
        if(families[i].queueCount <= 0)
            continue;

        if(found_families.graphics_index < 0
            && families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            found_families.graphics_index = i;
        }
        if(found_families.compute_index < 0
            && families[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            found_families.compute_index = i;
        }

        if(found_families.present_index < 0)
        {
            VkBool32 present_support = false;
            VkResult err = vkGetPhysicalDeviceSurfaceSupportKHR(
                device,
                i,
                surface,
                &present_support
            );
            if(err != VK_SUCCESS)
            {
                throw std::runtime_error(
                    "Failed to check present support: "
                    + get_vulkan_result_string(err)
                );
            }
            else if(present_support)
            {
                found_families.present_index = i;
            }
        }
    }
    return found_families;
}

int rate_surface_format(const VkSurfaceFormatKHR& format)
{
    // TODO: This could be more robust...
    int score = 0;
    switch(format.format)
    {
    case VK_FORMAT_B8G8R8A8_UNORM:
        score += 100;
        break;
    case VK_FORMAT_R8G8B8A8_UNORM:
        score += 99;
        break;
    case VK_FORMAT_R8G8B8_UNORM:
        score += 98;
        break;
    case VK_FORMAT_B8G8R8_UNORM:
        score += 97;
        break;
    case VK_FORMAT_R16G16B16A16_UNORM:
        score += 96;
        break;
    case VK_FORMAT_R16G16B16_UNORM:
        score += 95;
        break;
    default:
        return -1;
    }

    switch(format.format)
    {
    // Most monitors are calibrated for sRGB
    case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR:
        score += 100;
        break;
    // Other color spaces are _probably_ usable
    default:
        score += 0;
        break;
    }
    return score;
}

std::vector<VkSurfaceFormatKHR> find_surface_formats(
    VkPhysicalDevice device,
    VkSurfaceKHR surface,
    const VkSurfaceFormatKHR& default_format,
    rate_surface_format_callback rate
) {
    unsigned count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);

    std::vector<VkSurfaceFormatKHR> formats(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        device,
        surface,
        &count,
        formats.data()
    );

    if(formats.empty())
    {
        throw std::runtime_error(
            "Empty 'formats' vector given to find_surface_format"
        );
    }

    if(formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
        return {default_format};

    using scored_format = std::pair<VkSurfaceFormatKHR, int>;
    std::vector<scored_format> scored_formats;
    scored_formats.reserve(formats.size());

    for(const VkSurfaceFormatKHR& format: formats)
        scored_formats.push_back({format, rate(format)});

    std::sort(
        scored_formats.begin(),
        scored_formats.end(),
        [](const scored_format& a, const scored_format& b){
            return a.second > b.second;
        }
    );
    
    if(scored_formats[0].second < 0)
    {
        throw std::runtime_error(
            "Unable to find compatible surface formats"
        );
    }

    std::vector<VkSurfaceFormatKHR> suitable_formats;

    for(scored_format format: scored_formats)
    {
        if(format.second >= 0)
        {
            suitable_formats.push_back(format.first);
        }
    }

    return suitable_formats;
}

std::vector<VkPresentModeKHR> get_compatible_present_modes(
    VkPhysicalDevice device,
    VkSurfaceKHR surface
) {
    unsigned count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);

    std::vector<VkPresentModeKHR> modes(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device,
        surface,
        &count,
        modes.data()
    );

    return modes;
}
