# SPDX-FileCopyrightText: 2023 Davide Paro
#
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.19)

project(
    CMakeTemplate
    VERSION 0.1.0
    DESCRIPTION "<PUT YOUR CMAKE PROJECT DESCRIPTION HERE>"
)
option(CODE_COVERAGE "Enable coverage reporting" OFF)
option(BUILD_DOC "Build documentation" OFF)

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/.venv")
    find_package(Python3 3.12 REQUIRED COMPONENTS Interpreter)
    execute_process(
        COMMAND
        "${Python3_EXECUTABLE}" -m venv "${CMAKE_SOURCE_DIR}/.venv"
    )
endif()

set(ENV{VIRTUAL_ENV} "${CMAKE_SOURCE_DIR}/.venv")
set(Python3_FIND_VIRTUALENV FIRST)
unset(Python3_EXECUTABLE)
find_package(Python3 3.12 REQUIRED COMPONENTS Interpreter)

find_program(CURL_EXE NAMES curl REQUIRED)

string(JOIN ";" PYTHON_ENV_PROP
       "PYTHONPATH=${CMAKE_SOURCE_DIR}/src/python3:$ENV{PYTHONPATH}"
       "MYPYPATH=${CMAKE_SOURCE_DIR}/src/python3:$ENV{MYPYPATH}")

if(BUILD_DOC)
    set(DOXYGEN_GENERATE_HTML ON)
    set(DOXYGEN_GENERATE_MAN OFF)
    set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/docs")
    set(DOXYGEN_OPTIMIZE_OUTPUT_FOR_C YES)
    set(DOXYGEN_GENERATE_TREEVIEW YES)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_INLINE_SOURCES NO)
    set(DOXYGEN_SOURCE_BROWSER YES)
    set(DOXYGEN_VERBATIM_HEADERS YES)
    set(
        DOXYGEN_HTML_EXTRA_STYLESHEET
        "${CMAKE_SOURCE_DIR}/deps/doxygen-awesome-css/doxygen-awesome.css"
    )
    find_package(Doxygen OPTIONAL_COMPONENTS mscgen dia dot)
endif()

if(NOT BUILD_DOC OR NOT DOXYGEN_FOUND)
    # stub
    function(doxygen_add_docs)
    endfunction()
endif()

option(
    ASAN_ENABLED
    "Enable address sanitizer for supported compilers and DEBUG builds"
    OFF
)

set(CMAKE_C_STANDARD "11")
set(CMAKE_CXX_STANDARD "20")

set(CXX_DISABLED_FEATURES_OPTS -fno-exceptions -fno-rtti)
# Directory level and below compile options for C++
add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:${CXX_DISABLED_FEATURES_OPTS}>")

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Force colored error output from the compiler when using Ninja
if(CMAKE_GENERATOR STREQUAL "Ninja" AND
  ((CMAKE_C_COMPILER_ID STREQUAL "GNU"
    AND NOT CMAKE_C_COMPILER_VERSION VERSION_LESS 4.9) OR
  (CMAKE_C_COMPILER_ID STREQUAL "Clang"
    AND NOT CMAKE_C_COMPILER_VERSION VERSION_LESS 3.5)))
    # Force colored warnings in Ninja's output, if
    # the compiler has -fdiagnostics-color support.
    # Rationale in https://github.com/ninja-build/ninja/issues/814
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-color=always")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
endif()

if(ASAN_ENABLED)
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU"
        OR CMAKE_C_COMPILER_ID STREQUAL "Clang")

        if(MSVC)
            target_compile_options(<target> PUBLIC /fsanitize=address)
        elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang")
            string(
                APPEND SANITIZE_FLAGS
                "-fno-omit-frame-pointer "
                "-fsanitize=memory,address,undefined,bounds"
            )
        elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
            string(
                APPEND SANITIZE_FLAGS
                "-fno-omit-frame-pointer "
                "-fsanitize=address,undefined,bounds"
            )
        endif()

        string(APPEND CMAKE_C_FLAGS_DEBUG " ${SANITIZE_FLAGS}")
        string(APPEND CMAKE_C_FLAGS_RELWITHDEBINFO " ${SANITIZE_FLAGS}")

        string(APPEND CMAKE_CXX_FLAGS_DEBUG " ${SANITIZE_FLAGS}")
        string(APPEND CMAKE_CXX_FLAGS_RELWITHDEBINFO " ${SANITIZE_FLAGS}")

        string(APPEND CMAKE_LINKER_FLAGS_DEBUG " ${SANITIZE_FLAGS}")
        string(APPEND CMAKE_LINKER_FLAGS_RELWITHDEBINFO " ${SANITIZE_FLAGS}")
    endif()
endif()

if(NOT DEFINED RUNNING_IN_CI)
    set(RUNNING_IN_CI false)
endif()

if(RUNNING_IN_CI)
    add_definitions("-DRUNNING_IN_CI=1")
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/cmake")
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/cmake")
endif()

add_subdirectory(deps)
add_subdirectory(src)

include(CTest)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
