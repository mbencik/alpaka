#
# Copyright 2014-2020 Benjamin Worpitz, Axel Huebl, Jan Stephan
#
# This file is part of Alpaka.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

if(NOT ALPAKA_CI OR (ALPAKA_CI AND ALPAKA_CI_ANALYSIS))

    set(_TARGET_NAME "headerCheck")

    #---------------------------------------------------------------------------
    # Create source files.
    set(_ALPAKA_INCLUDE_DIRECTORY "${_ALPAKA_ROOT_DIR}/include")
    set(_ALPAKA_SUFFIXED_INCLUDE_DIR "${_ALPAKA_INCLUDE_DIRECTORY}/alpaka")
    append_recursive_files("${_ALPAKA_SUFFIXED_INCLUDE_DIR}" "hpp" "_ALPAKA_FILES_HEADER")

    set(_GENERATED_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/src")

    file(REMOVE_RECURSE ${_GENERATED_SOURCE_DIR})

    foreach(_HEADER_FILE ${_ALPAKA_FILES_HEADER})
        # Remove the parent directory from the path.
        # NOTE: This is not correct because it does not only replace at the beginning of the string.
        #  "STRING(REGEX REPLACE" would be correct if there was an easy way to escape arbitrary strings.
        string(
            REPLACE "${_ALPAKA_SUFFIXED_INCLUDE_DIR}/" ""
            _HEADER_FILE
            "${_HEADER_FILE}")
        set(_SOURCE_FILE "${_GENERATED_SOURCE_DIR}/${_HEADER_FILE}.cpp")
        file(WRITE "${_SOURCE_FILE}" "#include <alpaka/${_HEADER_FILE}>\n#include <alpaka/${_HEADER_FILE}>\n")
    endforeach()

    #---------------------------------------------------------------------------
    # Add executable.

    append_recursive_files_add_to_src_group("${_GENERATED_SOURCE_DIR}" "${_GENERATED_SOURCE_DIR}" "cpp" "_FILES_SOURCE")
    list(APPEND _FILES_SOURCE "src/main.cpp")

    # Always add all files to the target executable build call to add them to the build project.
    alpaka_add_executable(
        ${_TARGET_NAME}
        ${_FILES_SOURCE})
    target_link_libraries(
        ${_TARGET_NAME}
        PRIVATE common)

    set_target_properties(headerCheck PROPERTIES FOLDER "test/analysis")

    add_test(NAME ${_TARGET_NAME} COMMAND ${_TARGET_NAME} ${_ALPAKA_TEST_OPTIONS})

endif()
