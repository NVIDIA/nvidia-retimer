
configure_file(output: 'config.h',
              configuration : cdata,
              )
executable('nvidia-rt-monitor', 'main.cpp', 'rt_manager.cpp',
               dependencies:
                            [
                              sdbusplus,
                              sdeventplus,
                              phosphor_logging,
                              phosphor_dbus_interfaces,
                              fmt,
                            ],
               install : get_option('install_retimer_inventory'),
               install_dir : get_option('bindir'))