# Copyright 2023-2025 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

# Convert audio clip to C file
function(iot_reference_arm_corstone3xx_convert_audio_source_to_code audio_path generated_path)
    add_custom_target(convert-audio
        BYPRODUCTS
            ${generated_path}/sample_files.c
            ${generated_path}/test.c
        # use ml-embedded-evaluation-kit's Python Virtual Environment which
        # contains dependencies for gen_audio_cpp.py
        COMMAND bash -c " \
            source ${CMAKE_CURRENT_BINARY_DIR}/mlek_resources_downloaded/env/bin/activate && \
            pip install --upgrade soundfile && \
            python3 ${ml_embedded_evaluation_kit_SOURCE_DIR}/scripts/py/gen_audio_cpp.py \
            --audio_path ${audio_path} \
            --package_gen_dir ${generated_path} "
        VERBATIM
        USES_TERMINAL
    )
endfunction()
