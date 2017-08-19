#!/bin/sh

cd "${MESON_SOURCE_ROOT}"
${VULKAN_SDK_PATH}/bin/glslangValidator "$@"
