#-------------------------------------------------------------------------------
# Copyright (c) 2022 Cypress Semiconductor Corporation (an Infineon company)
# or an affiliate of Cypress Semiconductor Corporation. All rights reserved.
# Copyright (c) 2023-2024 Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Based on: https://git.trustedfirmware.org/TF-M/trusted-firmware-m.git/tree/cmake/remote_library.cmake?ef726cb3f25e8762182755104c367567b2387d59
#
#-------------------------------------------------------------------------------

find_package(Git)

# This function applies patches if they are not applied yet.
# It assumes that patches have not been applied if it's not possible to revert them.
#
# WORKING_DIRECTORY - working directory where patches should be applied.
# PATCH_FILES - list of patches. Patches will be applied in alphabetical order.
function(iot_reference_arm_corstone3xx_apply_patches WORKING_DIRECTORY PATCH_FILES)
    # Validate if patches are already applied by reverting patches in reverse order
    # Step 1 - keep changes in stash with random message/name to detect
    # that stash has been created by git
    string(RANDOM LENGTH 16 STASH_NAME)
    set(STASH_NAME "fri-apply_patches-${STASH_NAME}")
    execute_process(COMMAND "${GIT_EXECUTABLE}" stash push -u -m "${STASH_NAME}"
        WORKING_DIRECTORY ${WORKING_DIRECTORY}
        RESULT_VARIABLE VALIDATION_STATUS
        ERROR_QUIET OUTPUT_QUIET
    )
    # Step 2 - get list of stashes to validate that stash has been created
    if (VALIDATION_STATUS EQUAL 0)
        execute_process(COMMAND "${GIT_EXECUTABLE}" stash list
            WORKING_DIRECTORY ${WORKING_DIRECTORY}
            OUTPUT_VARIABLE STASH_LIST
            RESULT_VARIABLE VALIDATION_STATUS
            ERROR_QUIET
        )
        # Look for stash message to detect stash creation
        string(FIND "${STASH_LIST}" "${STASH_NAME}" STASH_INDEX)
        if (STASH_INDEX LESS 0)
            # Stash is not created, most probably because there is no changes
            set(VALIDATION_STATUS 0)
        else()
            # Step 3 - restore changes with git stash apply
            if (VALIDATION_STATUS EQUAL 0)
                execute_process(COMMAND "${GIT_EXECUTABLE}" stash apply
                    WORKING_DIRECTORY ${WORKING_DIRECTORY}
                    RESULT_VARIABLE VALIDATION_STATUS
                    ERROR_QUIET OUTPUT_QUIET
                )
            endif()
        endif()
    endif()
    # Step 4 - revert patches in reverse order
    if (VALIDATION_STATUS EQUAL 0)
        # Sort list of patches in descending order for validation
        list(SORT PATCH_FILES ORDER DESCENDING)
        foreach(PATCH ${PATCH_FILES})
            execute_process(COMMAND "${GIT_EXECUTABLE}" apply --reverse --verbose "${PATCH}"
                WORKING_DIRECTORY ${WORKING_DIRECTORY}
                RESULT_VARIABLE VALIDATION_STATUS
                ERROR_QUIET OUTPUT_QUIET
            )
            if (NOT VALIDATION_STATUS EQUAL 0)
                # patch failed to be applied, assume that we need to restore and
                # apply all patch set
                break()
            endif()
        endforeach()
    endif()
    # Step 5 - pop stash to restore original state
    if (STASH_INDEX GREATER_EQUAL 0)
        # Clear index before restore
        execute_process(COMMAND "${GIT_EXECUTABLE}" clean -df
            WORKING_DIRECTORY ${WORKING_DIRECTORY}
            ERROR_QUIET OUTPUT_QUIET
        )
        execute_process(COMMAND "${GIT_EXECUTABLE}" reset --hard
            WORKING_DIRECTORY ${WORKING_DIRECTORY}
            ERROR_QUIET OUTPUT_QUIET
        )
        execute_process(COMMAND "${GIT_EXECUTABLE}" stash pop --index
            WORKING_DIRECTORY ${WORKING_DIRECTORY}
            ERROR_QUIET OUTPUT_QUIET
        )
    else()
        # There is no stash, restore commit by clearing index
        execute_process(COMMAND "${GIT_EXECUTABLE}" clean -df
            WORKING_DIRECTORY ${WORKING_DIRECTORY}
            ERROR_QUIET OUTPUT_QUIET
        )
        execute_process(COMMAND "${GIT_EXECUTABLE}" reset --hard
            WORKING_DIRECTORY ${WORKING_DIRECTORY}
            ERROR_QUIET OUTPUT_QUIET
        )
    endif()

    if (NOT VALIDATION_STATUS EQUAL 0)
        # Validation has been failed, so we assume that patches should be applied
        # Sort list of patches in ascending order
        list(SORT PATCH_FILES ORDER ASCENDING)

        set(EXECUTE_COMMAND "${GIT_EXECUTABLE}" apply --verbose ${PATCH_FILES})
        execute_process(COMMAND ${EXECUTE_COMMAND}
            WORKING_DIRECTORY ${WORKING_DIRECTORY}
            RESULT_VARIABLE PATCH_STATUS
            COMMAND_ECHO STDOUT
        )
        if (NOT PATCH_STATUS EQUAL 0)
            message( FATAL_ERROR "Failed to apply patches at ${WORKING_DIRECTORY}" )
        endif()
    endif()
endfunction()
