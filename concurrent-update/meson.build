retimer_inc = include_directories('.')

source_files = [
    'updateRetimerMain.c',
]

lib_source_files = [
	'updateRetimerFwOverI2C.cpp'
]

configure_file(output: 'config.h',
            configuration: cdata, 
)

retimer_headers = include_directories('.')

retimer_deps = [
  meson.get_compiler('cpp').find_library('dl'),
]

runtime_sources = ['updateRetimerFwOverI2C.c', 'updateRetimerFwOverI2C.h']

retimer_lib = static_library(
 'updateRetimerFwruntime',
  runtime_sources,
  implicit_include_directories: false,
  include_directories: retimer_headers,
  dependencies: retimer_deps)

retimer_dep = declare_dependency(
  dependencies: retimer_deps,
  include_directories: retimer_inc,
  link_with: retimer_lib)

executable('updateRetimerFw',
            sources: source_files,
            include_directories: retimer_inc,
            dependencies:[
			sdbusplus,
			retimer_dep,
               ],
            install: true,
            install_dir: get_option('bindir')
)

