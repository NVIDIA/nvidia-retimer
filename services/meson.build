
unit_files = [
    'nvidia-retimer.service',
]

foreach unit : unit_files
configure_file(input: unit,
               output: unit,
               copy: true,
               install_dir: servicedir)
endforeach
