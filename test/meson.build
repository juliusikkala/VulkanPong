gtest = dependency('gtest', main : true)

srcdir = include_directories('../src')

test(
  'Thread pool',
  executable(
    'thread_pool',
    ['thread_pool.cc', '../src/thread_pool.cc'],
    dependencies : gtest,
    include_directories : srcdir
  )
)

test(
  'Resources',
  executable(
    'resource',
    ['resource.cc', '../src/resource_manager.cc', '../src/thread_pool.cc'],
    dependencies : gtest,
    include_directories : srcdir
  )
)
