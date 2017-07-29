project('VulkanPong', 'cpp')

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


conf_data.set('validate', 'false')
conf_data.set('name', 'Vulkan Pong')

subdir('src')
