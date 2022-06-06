
unit_files = [
    'nvidia-retimer.service',
]

foreach unit : unit_files
configure_file(input: unit,
               output: unit,
               copy: true,
               install_dir: servicedir,
               install: get_option('install_retimer_inventory'))
endforeach
