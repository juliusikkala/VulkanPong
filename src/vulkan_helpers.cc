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

    if((err = vkEnumerateInstanceLayerProperties(&count, NULL)) != VK_SUCCESS)
    {
        throw std::runtime_error(
            "Failed to enumerate Vulkan instance layers: "
            + get_vulkan_result_string(err)
        );
    }

    VkLayerProperties* available_layers = new VkLayerProperties[count];
    if((err = vkEnumerateInstanceLayerProperties(
            &count,
            available_layers
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
        for(unsigned j = 0; j < count; ++j)
        {
            VkLayerProperties& available = available_layers[j];
            if(strcmp(required, available.layerName) == 0)
            {
                found = true;
                break;
            }
        }
        if(!found)
        {
            delete [] available_layers;
            throw std::runtime_error(
                "Failed to find required Vulkan instance layer: "
                + std::string(required)
            );
        }
    }

    delete [] available_layers;
}

void ensure_vulkan_instance_extensions(
    const char* const* required_extensions,
    uint32_t required_extensions_count
) {
    if(required_extensions_count == 0) return;

    uint32_t count = 0;

    VkResult err;

    if((err = vkEnumerateInstanceExtensionProperties(
            NULL,
            &count,
            NULL
        )) != VK_SUCCESS)
    {
        throw std::runtime_error(
            "Failed to enumerate Vulkan extension layers: "
            + get_vulkan_result_string(err)
        );
    }

    VkExtensionProperties* available_extensions =
        new VkExtensionProperties[count];

    if((err = vkEnumerateInstanceExtensionProperties(
            NULL,
            &count,
            available_extensions
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
        for(unsigned j = 0; j < count; ++j)
        {
            VkExtensionProperties& available = available_extensions[j];
            if(strcmp(required, available.extensionName) == 0)
            {
                found = true;
                break;
            }
        }
        if(!found)
        {
            delete [] available_extensions;
            throw std::runtime_error(
                "Failed to find required Vulkan instance extension: "
                + std::string(required)
            );
        }
    }

    delete [] available_extensions;
}
