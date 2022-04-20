configure_file(output: 'config.h',
            configuration: cdata, 
)

add_project_arguments('-DFPGA_I2C_CNTRL_ADDR=0x62', language : 'c')
add_project_arguments('-DMAX_FW_IMAGE_SIZE=(256*1024)', language : 'c')

retimer_inc = include_directories('.')

source_files = [
    'updateRetimerMain.c',
]

lib_source_files = [
	'updateRetimerFwOverI2C.cpp'
]

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

