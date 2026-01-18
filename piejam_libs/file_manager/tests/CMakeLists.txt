# SPDX-FileCopyrightText: 2026 PieJam Fork Contributors
#
# SPDX-License-Identifier: CC0-1.0

if(PIEJAM_TESTS)
    add_executable(piejam_file_manager_test
        recording_test.cpp
        recording_scanner_test.cpp
        recording_database_test.cpp
    )

    target_link_libraries(piejam_file_manager_test
        PRIVATE
        piejam_file_manager
        gtest_driver
    )

    add_test(NAME piejam_file_manager_test COMMAND piejam_file_manager_test)
endif()
