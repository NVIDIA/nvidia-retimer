
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
               install : true,
               install_dir : get_option('bindir'))