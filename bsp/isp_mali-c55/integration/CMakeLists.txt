# Copyright 2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

add_library(isp_platform_driver)

add_library(isp_platform_driver_calibrations)
target_link_libraries(isp_platform_driver_calibrations
    PUBLIC
        isp_driver
)
target_sources(isp_platform_driver_calibrations
    PUBLIC
        src/calibration/acamera_calibrations_dynamic_fs_lin_dummy.c
        src/calibration/acamera_calibrations_dynamic_linear_dummy.c
        src/calibration/acamera_calibrations_dynamic_native_dummy.c
        src/calibration/acamera_calibrations_static_fs_lin_dummy.c
        src/calibration/acamera_calibrations_static_linear_dummy.c
        src/calibration/acamera_calibrations_static_native_dummy.c
        src/calibration/acamera_get_calibrations_dummy.c
)

add_library(isp_platform_driver_sensor)
target_link_libraries(isp_platform_driver_sensor
    PUBLIC
        isp_driver
)
target_include_directories(isp_platform_driver_sensor
    PUBLIC
        src/sensor
)
target_sources(isp_platform_driver_sensor
    PUBLIC
        src/sensor/fvp_sensor_drv.c
)

add_library(isp_platform_driver_system)
target_link_libraries(isp_platform_driver_system
    PUBLIC
        isp_driver
        freertos_kernel
)
target_include_directories(isp_platform_driver_system
    PUBLIC
        inc/system
)
target_sources(isp_platform_driver_system
    PUBLIC
        src/system/system_cdma.c
        src/system/system_hw_io.c
        src/system/system_i2c.c
        src/system/system_interrupts.c
        src/system/system_isp_io.c
        src/system/system_log.c
        src/system/system_semaphore.c
        src/system/system_spi.c
        src/system/system_spinlock.c
        src/system/system_stdlib.c
        src/system/system_sw_io.c
        src/system/system_timer.c
)

target_link_libraries(isp_platform_driver
    PUBLIC
        isp_firmware_config
        isp_platform_driver_calibrations
        isp_platform_driver_sensor
        isp_platform_driver_system
)

add_library(isp_control STATIC)

target_include_directories(isp_control
    PUBLIC
        .
        app
        ${isp_mali-c55_SOURCE_DIR}/software/driver/bare-metal/app/control
)

target_sources(isp_control
    PUBLIC
        app/acamera_uart.c
        ${isp_mali-c55_SOURCE_DIR}/software/driver/bare-metal/app/control/acamera_buffer_manager.c
        ${isp_mali-c55_SOURCE_DIR}/software/driver/bare-metal/app/control/acamera_connection.c
        ${isp_mali-c55_SOURCE_DIR}/software/driver/bare-metal/app/control/application_command_api.c
)

target_link_libraries(isp_control
    isp_driver
    freertos_kernel
)
