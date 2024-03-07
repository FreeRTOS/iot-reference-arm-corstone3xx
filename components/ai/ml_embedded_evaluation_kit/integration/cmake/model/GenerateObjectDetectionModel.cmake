#----------------------------------------------------------------------------
#  SPDX-FileCopyrightText: Copyright 2021-2024 Arm Limited and/or its affiliates <open-source-office@arm.com>
#  SPDX-License-Identifier: Apache-2.0
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#----------------------------------------------------------------------------
# This file is based on https://review.mlplatform.org/plugins/gitiles/ml/ethos-u/ml-embedded-evaluation-kit/+/refs/tags/23.11/source/use_case/object_detection/usecase.cmake

set(OBJECT_DETECTION_IMAGE_SIZE 192)
set(OBJECT_DETECTION_ANCHOR_1 "{38, 77, 47, 97, 61, 126}")
set(OBJECT_DETECTION_ANCHOR_2 "{14, 26, 19, 37, 28, 55 }")
set(OBJECT_DETECTION_ORIGINAL_IMAGE_SIZE ${OBJECT_DETECTION_IMAGE_SIZE})
set(OBJECT_DETECTION_ACTIVATION_BUF_SZ 0x00082000)

if (ETHOS_U_NPU_ENABLED)
    set(DEFAULT_MODEL_PATH      ${DEFAULT_MODEL_DIR}/yolo-fastest_192_face_v4_vela_${ETHOS_U_NPU_CONFIG_ID}.tflite)
else()
    set(DEFAULT_MODEL_PATH      ${DEFAULT_MODEL_DIR}/yolo-fastest_192_face_v4.tflite)
endif()

set(EXTRA_MODEL_CODE
    "extern const int originalImageSize = ${OBJECT_DETECTION_ORIGINAL_IMAGE_SIZE};"
    "/* NOTE: anchors are different for any given input model size, estimated during training phase */"
    "extern const float anchor1[] = ${OBJECT_DETECTION_ANCHOR_1};"
    "extern const float anchor2[] = ${OBJECT_DETECTION_ANCHOR_2};"
    )

set(OBJECT_DETECTION_MODEL_TFLITE_PATH ${DEFAULT_MODEL_PATH})

# Generate model file
generate_tflite_code(
    MODEL_PATH ${OBJECT_DETECTION_MODEL_TFLITE_PATH}
    DESTINATION ${SRC_GEN_DIR}
    EXPRESSIONS ${EXTRA_MODEL_CODE}
    NAMESPACE   "arm" "app" "object_detection")
