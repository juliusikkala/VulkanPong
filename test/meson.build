gtest = dependency('gtest', main : true)

srcdir = include_directories('../src')

test(
  'Resources',
  executable(
    'resource',
    ['resource.cc', '../src/resource_manager.cc', '../src/thread_pool.cc'],
    dependencies : gtest,
    include_directories : srcdir
  )
)
