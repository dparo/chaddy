# SPDX-FileCopyrightText: 2023 Davide Paro
#
# SPDX-License-Identifier: MIT

set(TEST_SOURCES_LIST "simple.c" "std.c")
set(TEST_PYTHON_LIST "test_py.py")

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(WARN_FLAGS "-Wall -Wextra -Wunused -Wconversion")
    string(APPEND CMAKE_C_FLAGS " ${WARN_FLAGS}")
    string(APPEND CMAKE_CXX_FLAGS " ${WARN_FLAGS}")
endif()

foreach(SRC IN LISTS TEST_SOURCES_LIST)
    get_filename_component(TARGET "${SRC}" NAME_WE)

    add_executable("${TARGET}" "${SRC}")
    target_link_libraries("${TARGET}" PRIVATE unity)
    target_link_libraries("${TARGET}" PRIVATE libmain)
    target_link_libraries("${TARGET}" PRIVATE logc)
    target_include_directories("${TARGET}" PRIVATE ./ ../src)

    if(UNIX AND NOT APPLE)
        target_compile_definitions("${TARGET}" PRIVATE _GNU_SOURCE)
    endif()

    # Conditionally enable Code Coverage under GNU/Linux only
    if(CODE_COVERAGE
       AND UNIX
       AND NOT APPLE)
        target_compile_options("${TARGET}" PRIVATE -fprofile-arcs
                                                   -ftest-coverage)
        target_link_options("${TARGET}" PRIVATE -fprofile-arcs)
    endif()

    add_test(
        NAME "${TARGET}"
        COMMAND "${TARGET}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endforeach()

execute_process(COMMAND "${Python3_EXECUTABLE}" -m pip install -r
                        "${CMAKE_SOURCE_DIR}/requirements.txt")

foreach(SRC IN LISTS TEST_PYTHON_LIST)
    get_filename_component(TARGET "${SRC}" NAME_WE)

    add_test(
        NAME "pytest::${TARGET}"
        COMMAND ${Python3_EXECUTABLE} -m pytest --mypy
                "${CMAKE_CURRENT_SOURCE_DIR}/${SRC}"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

    set_tests_properties("pytest::${TARGET}" PROPERTIES ENVIRONMENT
                                                        "${PYTHON_ENV_PROP}")
endforeach()
