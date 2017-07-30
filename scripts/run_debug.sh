#!/bin/sh

cd "${MESON_SOURCE_ROOT}"
LD_LIBRARY_PATH=${VULKAN_SDK_PATH}/lib VK_LAYER_PATH=${VULKAN_SDK_PATH}/etc/explicit_layer.d ${MESON_BUILD_ROOT}/src/pong 
