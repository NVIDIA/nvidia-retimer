gtest = dependency('gtest', main: true, disabler: true, required: false)
gmock = dependency('gmock', disabler: true, required: false)

test_source_files = ['test_updateRetimerFw.cpp']

test_headers = include_directories('.')

test_deps = [
  meson.get_compiler('cpp').find_library('dl'),
  gtest,
  gmock,
  retimer_dep,
]

test_lib = static_library(
  'mock_syscall.cpp',
  implicit_include_directories: false,
  include_directories: test_headers,
  link_with : retimer_lib 
  )

test_dep = declare_dependency(
  dependencies: test_deps,
  include_directories: test_headers,
  link_with : test_lib
)

executable('test_updateRetimerFw',
            sources: test_source_files,
            dependencies:[
            test_dep,
			sdbusplus,
               ],
  			link_with : retimer_lib,
            install: true,
            install_dir: get_option('bindir')
)
