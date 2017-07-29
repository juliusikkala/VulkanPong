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

static void* get_native_window_handle(SDL_Window* win)
{
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);

    if(SDL_GetWindowWMInfo(win, &info))
    {
        switch(info.subsystem)
        {
        #ifdef SDL_VIDEO_DRIVER_WINDOWS
        case SDL_SYSWM_WINDOWS:
            return info.info.win.window;
        #endif
        #ifdef SDL_VIDEO_DRIVER_X11
        case SDL_SYSWM_X11:
            return (void*)info.info.x11.window;
        #endif
        #ifdef SDL_VIDEO_DRIVER_WAYLAND
        case SDL_SYSWM_WAYLAND:
            return info.info.wl.surface;
        #endif
        default:
            return nullptr;
        }
    }
    return nullptr;
}

window::window(context& ctx, const params& p)
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
        throw new std::runtime_error(SDL_GetError());
    }
}
    
window::window(window&& other)
: win(other.win)
{
    other.win = nullptr;
}

window::~window()
{
    if(win) SDL_DestroyWindow(win);
}

