retimer_inc = include_directories('.')

source_files = [
    'updateRetimerMain.c',
]

source_files_dbus = [
    'dbus-service-retimer.c',
]


lib_source_files = [
	'updateRetimerFwOverI2C.cpp',
	'dbus_log_event.c'
]

configure_file(output: 'config.h',
            configuration: cdata, 
)

retimer_headers = include_directories('.')

retimer_deps = [
  meson.get_compiler('cpp').find_library('dl'),
]

runtime_sources = ['updateRetimerFwOverI2C.c', 'updateRetimerFwOverI2C.h','updateRetimerFw_dbus_log_event.c','updateRetimerFw_dbus_log_event.h']

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

ssl = dependency('openssl')
executable('dbus-service-retimer',
            sources: source_files_dbus,
            include_directories: retimer_inc,
            dependencies:[
              sdbusplus,
              retimer_dep,
              ssl
            ],
            install: true,
            install_dir: get_option('bindir')
)

unit_files = [
    'nvidia-hashcompute-retimer.service',
]

foreach unit : unit_files
configure_file(input: unit,
               output: unit,
               copy: true,
               install_dir: servicedir,
               install: true)
endforeach

