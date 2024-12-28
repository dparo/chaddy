# SPDX-FileCopyrightText: 2024 Davide Paro <dparo@outlook.it>, et al.
# SPDX-FileContributor: Davide Paro <dparo@outlook.it>
#
# SPDX-License-Identifier: Apache-2.0

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(WARN_FLAGS "-Wall -Wextra -Wunused -Wconversion")
    string(APPEND CMAKE_C_FLAGS " ${WARN_FLAGS}")
    string(APPEND CMAKE_CXX_FLAGS " ${WARN_FLAGS}")
    string(APPEND CMAKE_LINKER_FLAGS " ${WARN_FLAGS}")

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(NOWARN_FLAGS "-Wno-unused-parameter")
        string(APPEND CMAKE_C_FLAGS " ${NOWARN_FLAGS}")
        string(APPEND CMAKE_CXX_FLAGS " ${NOWARN_FLAGS}")
        string(APPEND CMAKE_LINKER_FLAGS " ${NOWARN_FLAGS}")
    endif()
endif()

find_package(Git REQUIRED)
find_package(CURL REQUIRED)     # HINTS CURL_USE_STATIC_LIBS

execute_process(
    COMMAND "${GIT_EXECUTABLE}" describe --match=NeVeRmAtCh --always --abbrev=40
            --dirty
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_SHA1
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

# the date of the commit
execute_process(
    COMMAND "${GIT_EXECUTABLE}" log -1 --format=%ad --date=local
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_DATE
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

# the subject of the commit
execute_process(
    COMMAND "${GIT_EXECUTABLE}" log -1 --format=%s
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_COMMIT_SUBJECT
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

add_custom_target(
    "mypy-src" ALL
    COMMAND ${Python3_EXECUTABLE} -m mypy "${CMAKE_CURRENT_SOURCE_DIR}/python3"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

set_target_properties("mypy-src" PROPERTIES ENVIRONMENT "${PYTHON_ENV_PROP}")

# generate version.c
configure_file("version.c.in" "${CMAKE_CURRENT_BINARY_DIR}/version.c" @ONLY)

add_executable("cembedfile" cembedfile/cembedfile.c)
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/app_css.css.c"
    COMMAND cembedfile "${CMAKE_CURRENT_BINARY_DIR}/app_css.css.c" ../output.css app_css
    DEPENDS ../output.css
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_library(logc logc/log.c)
target_include_directories(logc PUBLIC logc)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(logc PUBLIC "RXI_LOGC_DEFAULT_LEVEL=LOG_DEBUG")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(logc PUBLIC "RXI_LOGC_DEFAULT_LEVEL=LOG_INFO")
endif()


add_library(libmain sum.c html5.c "${CMAKE_CURRENT_BINARY_DIR}/app_css.css.c" )
target_include_directories(libmain PUBLIC ./)
target_link_libraries(libmain PUBLIC m)
target_link_libraries(libmain PUBLIC logc CURL::libcurl)
target_link_libraries(libmain PUBLIC ZLIB::ZLIB)
target_link_libraries(libmain PRIVATE $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,libuv::uv>)
target_include_directories(libmain PUBLIC "${HEDLEY_INCLUDE_DIRS}" "${SCOTTT_DEBUGBREAK_INCLUDE_DIRS}" "${CURL_INCLUDE_DIRS}")



add_executable("${PROJECT_NAME}" main.c "${CMAKE_CURRENT_BINARY_DIR}/version.c")
target_link_libraries("${PROJECT_NAME}" PRIVATE libmain)
target_link_libraries("${PROJECT_NAME}" PRIVATE m)
target_link_libraries("${PROJECT_NAME}" PRIVATE argtable3::argtable3)
target_include_directories("${PROJECT_NAME}"
                           PRIVATE "${DEPS_DIR}/argtable3/src")
