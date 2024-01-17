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
# This file is based on https://review.mlplatform.org/plugins/gitiles/ml/ethos-u/ml-embedded-evaluation-kit/+/refs/tags/23.08/source/use_case/kws/usecase.cmake

set(KWS_LABELS_TXT_FILE
    ${ml_embedded_evaluation_kit_SOURCE_DIR}/resources/kws/labels/micronet_kws_labels.txt)

set(KWS_LABELS_CPP_FILE Labels)
generate_labels_code(
    INPUT           "${KWS_LABELS_TXT_FILE}"
    DESTINATION_SRC ${SRC_GEN_DIR}
    DESTINATION_HDR ${INC_GEN_DIR}
    OUTPUT_FILENAME "${KWS_LABELS_CPP_FILE}"
)

set(KWS_ACTIVATION_BUF_SZ 0x00100000)
set(KWS_MODEL_SCORE_THRESHOLD 0.7)

if (ETHOS_U_NPU_ENABLED)
    set(DEFAULT_MODEL_PATH      ${DEFAULT_MODEL_DIR}/kws_micronet_m_vela_${ETHOS_U_NPU_CONFIG_ID}.tflite)
else()
    set(DEFAULT_MODEL_PATH      ${DEFAULT_MODEL_DIR}/kws_micronet_m.tflite)
endif()

set(EXTRA_MODEL_CODE
    "/* Model parameters for KWS */"
    "extern const int   g_FrameLength    = 640"
    "extern const int   g_FrameStride    = 320"
    "extern const float g_ScoreThreshold = ${KWS_MODEL_SCORE_THRESHOLD}"
    )

set(KWS_MODEL_TFLITE_PATH ${DEFAULT_MODEL_PATH})

generate_tflite_code(
    MODEL_PATH  ${KWS_MODEL_TFLITE_PATH}
    DESTINATION ${SRC_GEN_DIR}
    EXPRESSIONS ${EXTRA_MODEL_CODE}
    NAMESPACE   "arm" "app" "kws"
)
