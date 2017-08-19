project('VulkanPong', 'cpp',
        default_options : ['cpp_std=c++14'])

# Config
conf_data = configuration_data()
conf_data.set('major', '0')
conf_data.set('minor', '0')
conf_data.set('patch', '1')

sys = host_machine.system()
conf_data.set('platform', sys)
if sys == 'linux'
  conf_data.set('USE_UNIX', true)
elif sys == 'bsd'
  conf_data.set('USE_UNIX', true)
elif sys == 'windows'
  conf_data.set('USE_WIN32', true)
endif

validation_layers = [
  'VK_LAYER_LUNARG_standard_validation'
]
validation_layers_escaped = []
foreach layer : validation_layers
  validation_layers_escaped += '"@0@"'.format(layer)
endforeach

conf_data.set(
  'validation_layers',
  ', '.join(validation_layers_escaped)
)

debug = get_option('buildtype').startswith('debug')

conf_data.set('USE_VALIDATION_LAYERS', debug)
conf_data.set('DEBUG', debug)

conf_data.set('name', 'Vulkan Pong')

# Additional targets
run_target(
  'run',
  command : ['scripts/run_debug.sh']
)

glsl_comp = find_program(
  'glslangValidator',
  '../scripts/glslangValidator.sh',
  required : true
)


# Source
subdir('src')
