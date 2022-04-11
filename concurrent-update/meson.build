configure_file(output: 'config.h',
            configuration: cdata, 
)

retimer_inc = include_directories('.')

source_files = [
    'updateRetimerFwOverI2C.c',
    'compmngrretimer.h',
]

executable('updateRetimerFw',
            sources: source_files,
            include_directories: retimer_inc,
            dependencies:[
		sdbusplus,
               ],
            install: true,
            install_dir: get_option('bindir')
)
