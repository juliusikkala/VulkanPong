VulkanPong
==========

Trying to learn Vulkan by making a pong game.

Progress
========
- [x] Open a window
- [x] Initialise Vulkan
- [ ] Make the screen not black
- [ ] Show a triangle
- [ ] Show a ball
- [ ] Make game work
- [ ] Controller support?
- [ ] Fancy-schmancy shaders?
- [ ] Groundbreaking sound effects?

Dependencies
============

* SDL2
* GLM
* Vulkan (duh)

How to build and run
====================

* Install [meson](http://mesonbuild.com/Getting-meson.html).
* Run `meson build`
* `ninja -C build`
* `build/src/pong`

Attributions
============

Parts of this software have been copied from the [Vulkan Manual by Khronos Group](https://www.khronos.org/registry/vulkan/specs/1.0/man/html).
Copyright (c) 2014-2017 Khronos Group. This work is licensed under a [Creative Commons Attribution 4.0 International License](https://creativecommons.org/licenses/by/4.0/).
