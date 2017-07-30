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

window::window(context& ctx, const params& p)
: ctx(ctx)
{
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

    surface = create_window_surface(ctx.get_instance(), win);
}
    
window::window(window&& other)
: ctx(other.ctx), win(other.win), surface(other.surface)
{
    other.win = nullptr;
    other.surface = VK_NULL_HANDLE;
}

window::~window()
{
    if(surface) vkDestroySurfaceKHR(ctx.get_instance(), surface, nullptr);
    if(win) SDL_DestroyWindow(win);
}

