aries_build_files = [
    'aries_api.c',
    'aries_i2c.c',
    'aries_misc.c',
    'aries_bifurcation_params.c',
    'astera_log.c'
]

i2c = meson.get_compiler('c').find_library('i2c')

test_sources = aries_build_files + ['aspeed.c', 'aries_test.c']
flash_sources = aries_build_files + ['aspeed.c', 'eeprom_update.c']
info_sources = aries_build_files + ['aspeed.c', 'aries_info.c']

executable('aries-test', test_sources,
               dependencies:
                            [
                              phosphor_logging,
                              i2c
                            ],
               install : get_option('install_aries_tools'),
               install_dir : get_option('bindir'))

executable('aries-update', flash_sources,
               dependencies:
                            [
                              phosphor_logging,
                              i2c
                            ],
               install : get_option('install_aries_tools'),
               install_dir : get_option('bindir'))

executable('aries-info', info_sources,
               dependencies:
                            [
                              phosphor_logging,
                              i2c
                            ],
               install : get_option('install_aries_tools'),
               install_dir : get_option('bindir'))
