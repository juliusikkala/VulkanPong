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
#include "window.hh"
#include <stdexcept>
#include <SDL2/SDL_syswm.h>
#include "context.hh"
#include "vulkan_helpers.hh"

using create_surface_fn = VkResult(*)(
    VkInstance,
    const void*,
    const VkAllocationCallbacks*,
    VkSurfaceKHR*
);

VkSurfaceKHR create_window_surface(
    VkInstance instance,
    SDL_Window* win
) {
    VkSurfaceKHR surface;
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);

    const char* surface_creator_name;
    void* create_info;

#ifdef SDL_VIDEO_DRIVER_WINDOWS
    VkWin32SurfaceCreateInfoKHR win32_info = {};
#endif
#ifdef SDL_VIDEO_DRIVER_X11
    VkXlibSurfaceCreateInfoKHR xlib_info = {};
#endif
#ifdef SDL_VIDEO_DRIVER_WAYLAND
    VkWaylandSurfaceCreateInfoKHR wayland_info = {};
#endif

    if(SDL_GetWindowWMInfo(win, &info))
    {
        switch(info.subsystem)
        {
#ifdef SDL_VIDEO_DRIVER_WINDOWS
        case SDL_SYSWM_WINDOWS:
            win32_info.sType =
                VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
            win32_info.hwnd = info.info.win.window;
            win32_info.hinstance = info.info.win.hinstance;

            surface_creator_name = "vkCreateWin32SurfaceKHR";
            create_info = &win32_info;
            break;
#endif
#ifdef SDL_VIDEO_DRIVER_X11
        case SDL_SYSWM_X11:
            xlib_info.sType =
                VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
            xlib_info.dpy = info.info.x11.display;
            xlib_info.window = info.info.x11.window;

            surface_creator_name = "vkCreateXlibSurfaceKHR";
            create_info = &xlib_info;
            break;
#endif
#ifdef SDL_VIDEO_DRIVER_WAYLAND
        case SDL_SYSWM_WAYLAND:
            wayland_info.sType =
                VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
            wayland_info.display = info.info.wl.display;
            wayland_info.surface = info.info.wl.surface;

            surface_creator_name = "vkCreateWaylandSurfaceKHR";
            create_info = &wayland_info;
            break;
#endif
        default:
            throw std::runtime_error(
                "Unknown WM type: "
                + std::to_string(info.subsystem)
            );
        }
    }
    else
    {
        throw std::runtime_error(SDL_GetError());
    }

    create_surface_fn create_surface = (create_surface_fn)
        vkGetInstanceProcAddr(instance, surface_creator_name);
                
    if(!create_surface)
    {
        throw std::runtime_error(
            "Could not get address of " + std::string(surface_creator_name)
        );
    }

    VkResult err = create_surface(instance, create_info, nullptr, &surface);

    if(err != VK_SUCCESS)
    {
        throw std::runtime_error(
            "Failed to create window surface: "
            + get_vulkan_result_string(err));
    }

    return surface;
}

window::window(context& ctx, const parameters& p)
: params(p), ctx(ctx)
{
    using namespace std::placeholders;

    // Create window
    win = SDL_CreateWindow(
        p.title,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        p.w, p.h,
        (p.fullscreen && SDL_WINDOW_FULLSCREEN)
    );

    if(!win)
    {
        throw std::runtime_error(SDL_GetError());
    }

    // Create surface
    surface = create_window_surface(ctx.get_instance(), win);

    create_device();
    create_swapchain();
}
    
window::window(window&& other)
: params(other.params), ctx(other.ctx), win(other.win), surface(other.surface),
  surface_capabilities(other.surface_capabilities), format(other.format),
  present_mode(other.present_mode), extent(other.extent), dev(other.dev),
  physical_device(other.physical_device), families(other.families),
  graphics_queue(other.graphics_queue), present_queue(other.present_queue),
  swapchain(other.swapchain),
  swapchain_images(std::move(other.swapchain_images))
{
    other.win = nullptr;
    other.surface = VK_NULL_HANDLE;
    other.dev = VK_NULL_HANDLE;
    other.physical_device = VK_NULL_HANDLE;
    other.graphics_queue = VK_NULL_HANDLE;
    other.present_queue = VK_NULL_HANDLE;
    other.swapchain = VK_NULL_HANDLE;
}

window::~window()
{
    destroy_swapchain();
    destroy_device();
    if(surface) vkDestroySurfaceKHR(ctx.get_instance(), surface, nullptr);
    if(win) SDL_DestroyWindow(win);
}

VkSurfaceKHR window::get_surface() const
{
    return surface;
}

void window::create_device()
{
    ctx.allocate_device(
        surface,
        dev,
        physical_device,
        families
    );

    vkGetDeviceQueue(dev, families.graphics_index, 0, &graphics_queue);
    vkGetDeviceQueue(dev, families.present_index, 0, &present_queue);

    std::vector<VkSurfaceFormatKHR> formats = find_surface_formats(
        physical_device,
        surface
    );

    if(formats.empty())
        throw std::runtime_error("Failed to find compatible surface formats");

    format = formats[0];

    present_mode = VK_PRESENT_MODE_FIFO_KHR;

    if(params.vsync)
    {
        std::vector<VkPresentModeKHR> available_modes =
            get_compatible_present_modes(physical_device, surface);

        for(VkPresentModeKHR available_mode: available_modes)
        {
            if(available_mode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                present_mode = available_mode;
                break;
            }
            else if(available_mode == VK_PRESENT_MODE_FIFO_KHR)
            {
                present_mode = available_mode;
            }
        }
    }

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physical_device,
        surface,
        &surface_capabilities
    );

    extent = find_swap_extent(
        surface_capabilities,
        {params.w, params.h}
    );
}

void window::destroy_device()
{
    if(dev)
    {
        ctx.free_device(surface, dev);
        dev = VK_NULL_HANDLE;
    }
}

void window::create_swapchain()
{
    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface;
    create_info.minImageCount = (params.vsync &&
        surface_capabilities.minImageCount !=
        surface_capabilities.maxImageCount)
        ? surface_capabilities.minImageCount + 1
        : surface_capabilities.minImageCount;
    create_info.imageFormat = format.format;
    create_info.imageColorSpace = format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    unsigned family_indices[] = {
        (unsigned)families.graphics_index,
        (unsigned)families.present_index
    };

    if(families.graphics_index == families.present_index)
    {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = nullptr;
    }
    else
    {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = family_indices;
    }

    create_info.preTransform = surface_capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;// FIXME

    VkResult err;
    if((err = vkCreateSwapchainKHR(
            dev,
            &create_info,
            nullptr,
            &swapchain
        )) != VK_SUCCESS)
    {
        throw std::runtime_error(
            "Failed to create a swap chain: " + get_vulkan_result_string(err)
        );
    }

    unsigned image_count;
    vkGetSwapchainImagesKHR(dev, swapchain, &image_count, nullptr);
    swapchain_images.resize(image_count);
    vkGetSwapchainImagesKHR(
        dev,
        swapchain,
        &image_count,
        swapchain_images.data()
    );
}

void window::destroy_swapchain()
{
    if(swapchain)
    {
        vkDestroySwapchainKHR(dev, swapchain, nullptr);
        swapchain = VK_NULL_HANDLE;
    }
}
