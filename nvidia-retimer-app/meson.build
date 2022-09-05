configure_file(output: 'config.h',
              configuration : cdata,
              )
executable('nvidia-retimer-app', 'main.cpp', 'retimer_app.cpp',
               include_directories: include_directories('.'),
               dependencies:
                            [
                              sdbusplus,
                              sdeventplus,
                              fmt,
                            ],
               install : true,
               install_dir : get_option('bindir'))

unit_files = [
    'nvidia-retimer-app.service',
]

foreach unit : unit_files
configure_file(input: unit,
               output: unit,
               copy: true,
               install_dir: servicedir,
               install: true)
endforeach