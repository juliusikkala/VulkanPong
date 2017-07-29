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

context::context()
{
    const char* error = "Unknown error";
    if(exists()) throw std::runtime_error("A context already exists.");

    if(SDL_Init(SDL_INIT_EVERYTHING))
    {
        error = SDL_GetError();
        goto fail_init;
    }

    return;

fail_:
    SDL_Quit();
fail_init:
    throw std::runtime_error(error);
}

context::context(context&& other)
{
}

context::~context()
{
    SDL_Quit();
}

bool& context::exists()
{
    static bool e = false;
    return e;
}
