# SPDX-FileCopyrightText: 2026 PieJam Fork Contributors
#
# SPDX-License-Identifier: CC0-1.0

find_package(Qt5 REQUIRED COMPONENTS Core Concurrent)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SNDFILE REQUIRED sndfile)

set(CMAKE_AUTOMOC ON)

add_library(piejam_file_manager STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/piejam/file_manager/fwd.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/piejam/file_manager/recording.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/piejam/file_manager/recording_scanner.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/piejam/file_manager/metadata_extractor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/piejam/file_manager/recording_database.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/piejam/file_manager/recording_list_model.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/piejam/file_manager/file_browser_controller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/piejam/file_manager/recording_scanner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/piejam/file_manager/metadata_extractor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/piejam/file_manager/recording_database.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/piejam/file_manager/recording_list_model.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/piejam/file_manager/file_browser_controller.cpp
)

target_include_directories(piejam_file_manager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(piejam_file_manager PRIVATE ${SNDFILE_INCLUDE_DIRS})

target_link_libraries(piejam_file_manager
    PUBLIC
    Qt5::Core
    Qt5::Concurrent
    PRIVATE
    piejam_compiler_warnings
    piejam_log
    ${SNDFILE_LIBRARIES}
)

add_subdirectory(tests)
