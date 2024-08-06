
add_library(ota-for-aws-iot-embedded-sdk-mock
        src/ota.c
)

target_include_directories(ota-for-aws-iot-embedded-sdk-mock
    PUBLIC
        inc

        portable/os
)

target_link_libraries(ota-for-aws-iot-embedded-sdk-mock
    PRIVATE
        fff
)
