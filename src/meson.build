configure_file(
    input : 'config.hh.in',
    output : 'config.hh',
    configuration : conf_data
)

src = [
    'main.cc',
    'context.cc',
    'window.cc',
    'vulkan_helpers.cc'
]
cc = meson.get_compiler('cpp')
m_dep = cc.find_library('m', required : false)
sdl2_dep = dependency('sdl2', version: '>=2.0.5')
glm_dep = dependency('glm', version: '>=0.9.8')
vk_dep = cc.find_library('vulkan', required : true)

executable(
    'pong',
    src,
    dependencies: [m_dep, sdl2_dep, glm_dep, vk_dep],
    install: true,
    gui_app: true,
)
