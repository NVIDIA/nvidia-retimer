project('nvidia-retimer', ['c','cpp'],
        version: '0.1', meson_version: '>=0.57.0',
        default_options: [
          'warning_level=3',
          'cpp_std=c++20',
          'b_lto=true',
        ])

systemd = dependency('systemd')
servicedir = systemd.get_pkgconfig_variable('systemdsystemunitdir')

cdata = configuration_data()
cdata.set_quoted(
    'RT_JSON_PATH', '/usr/share/nvidia-retimer/rt_config.json')

cdata.set_quoted(
    'BUSNAME', 'com.Nvidia.Rt')

cdata.set_quoted(
    'BASE_INV_PATH', '/xyz/openbmc_project/inventory/system/board')

cdata.set('FPGA_I2C_CNTRL_ADDR',get_option('fpga_i2c_cntrl_addr'))
cdata.set('MAX_FW_IMAGE_SIZE',get_option('fw_image_size'))

cdata.set_quoted('DBUS_SERVICE_NAME', get_option('DBUS_SERVICE_NAME'))
cdata.set('FPGA_I2C_BUS', get_option('FPGA_I2C_BUS'))

cdata.set('PLATFORM_TYPE', get_option('PLATFORM_TYPE'))

sdbusplus = dependency('sdbusplus')
sdeventplus = dependency('sdeventplus')
fmt = dependency('fmt')
phosphor_dbus_interfaces = dependency('phosphor-dbus-interfaces')
phosphor_logging = dependency('phosphor-logging')

add_project_arguments('-Wno-incompatible-pointer-types', language : 'c') 



subdir('concurrent-update')
subdir('inventory')
subdir('services')
subdir('aries-fw-update')
if get_option('retimer_app_support')
  subdir('nvidia-retimer-app')
endif

if get_option('tests').enabled()
  subdir('test')
endif
