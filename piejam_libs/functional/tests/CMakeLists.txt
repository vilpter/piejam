# SPDX-FileCopyrightText: 2020-2025 Dimitrij Kotrev
#
# SPDX-License-Identifier: CC0-1.0

if(NOT PIEJAM_TESTS)
    return()
endif()

add_executable(piejam_functional_test
    ${CMAKE_CURRENT_SOURCE_DIR}/get_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_if_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/memo_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/operators_test.cpp
)
target_link_libraries(piejam_functional_test gtest_driver piejam_compiler_warnings piejam_functional)

add_test(NAME piejam_functional_test COMMAND piejam_functional_test)

install(TARGETS piejam_functional_test RUNTIME DESTINATION bin)
