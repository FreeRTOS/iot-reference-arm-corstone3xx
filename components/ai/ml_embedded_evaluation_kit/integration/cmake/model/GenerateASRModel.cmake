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
# This file is based on https://review.mlplatform.org/plugins/gitiles/ml/ethos-u/ml-embedded-evaluation-kit/+/refs/tags/23.08/source/use_case/asr/usecase.cmake

set(ASR_LABELS_TXT_FILE
    ${ml_embedded_evaluation_kit_SOURCE_DIR}/resources/asr/labels/labels_wav2letter.txt)

set(ASR_LABELS_CPP_FILE Labels)
generate_labels_code(
    INPUT           "${ASR_LABELS_TXT_FILE}"
    DESTINATION_SRC ${SRC_GEN_DIR}
    DESTINATION_HDR ${INC_GEN_DIR}
    OUTPUT_FILENAME "${ASR_LABELS_CPP_FILE}"
)

set(ASR_ACTIVATION_BUF_SZ 0x00200000)
set(ASR_MODEL_SCORE_THRESHOLD 0.5)

if (ETHOS_U_NPU_ENABLED)
    set(DEFAULT_MODEL_PATH      ${DEFAULT_MODEL_DIR}/tiny_wav2letter_pruned_int8_vela_${ETHOS_U_NPU_CONFIG_ID}.tflite)
else()
    set(DEFAULT_MODEL_PATH      ${DEFAULT_MODEL_DIR}/tiny_wav2letter_pruned_int8.tflite)
endif()

set(EXTRA_MODEL_CODE
    "/* Model parameters for asr */"
    "extern const int   g_FrameLength    = 512"
    "extern const int   g_FrameStride    = 160"
    "extern const int   g_ctxLen         =  98"
    "extern const float g_ScoreThreshold = ${ASR_MODEL_SCORE_THRESHOLD}"
    )

set(ASR_MODEL_TFLITE_PATH ${DEFAULT_MODEL_PATH})

generate_tflite_code(
    MODEL_PATH  ${ASR_MODEL_TFLITE_PATH}
    DESTINATION ${SRC_GEN_DIR}
    EXPRESSIONS ${EXTRA_MODEL_CODE}
    NAMESPACE   "arm" "app" "asr"
)
