configure_file(
  input : 'config.hh.in',
  output : 'config.hh',
  configuration : conf_data
)

src = [
  'main.cc',
  'context.cc',
  'window.cc',
  'thread_pool.cc',
  'vulkan_helpers.cc'
]

shaders = [
  ['shaders/shader.frag', 'fragment.spv'],
  ['shaders/shader.vert', 'vertex.spv']
]

cc = meson.get_compiler('cpp')
m_dep = cc.find_library('m', required : false)
sdl2_dep = dependency('sdl2', version: '>=2.0.5')
glm_dep = dependency('glm', version: '>=0.9.8')
vk_dep = cc.find_library('vulkan', required : true)
thread_dep = dependency('threads')

foreach shader : shaders
  custom_target(
    shader[1],
    input : shader[0],
    output : shader[1],
    command : [glsl_comp, '-V', '@INPUT@', '-o', '@OUTPUT@'],
    install : false
  )
endforeach

executable(
  'pong',
  src,
  dependencies: [m_dep, sdl2_dep, glm_dep, vk_dep, thread_dep],
  install: true,
  gui_app: true,
)

