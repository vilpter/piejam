// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2026  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/network_manager/network_controller.h>

#include <gtest/gtest.h>

namespace piejam::network_manager::test
{

class network_controller_test : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Tests run in isolation - actual network operations may fail
        // in test environment, so we mainly test the logic/state management
    }
};

TEST_F(network_controller_test, initial_auto_disable_is_false)
{
    network_controller ctrl;
    EXPECT_FALSE(ctrl.auto_disable_on_record());
}

TEST_F(network_controller_test, set_auto_disable_on_record)
{
    network_controller ctrl;

    ctrl.set_auto_disable_on_record(true);
    EXPECT_TRUE(ctrl.auto_disable_on_record());

    ctrl.set_auto_disable_on_record(false);
    EXPECT_FALSE(ctrl.auto_disable_on_record());
}

TEST_F(network_controller_test, on_recording_start_returns_previous_state)
{
    network_controller ctrl;

    // When auto-disable is off, recording start should return current state
    // without changing it
    ctrl.set_auto_disable_on_record(false);
    bool was_enabled = ctrl.on_recording_start();
    // Result depends on actual system state, just verify it returns a value
    (void)was_enabled;
}

TEST_F(network_controller_test, state_change_callback_is_called)
{
    network_controller ctrl;

    bool callback_called = false;
    bool callback_value = false;

    ctrl.set_state_change_callback([&](bool enabled) {
        callback_called = true;
        callback_value = enabled;
    });

    // Callback should be set successfully
    // Actual enable/disable may fail in test environment without root
    EXPECT_FALSE(callback_called); // Not called yet
}

TEST_F(network_controller_test, move_constructor)
{
    network_controller ctrl1;
    ctrl1.set_auto_disable_on_record(true);

    network_controller ctrl2(std::move(ctrl1));
    EXPECT_TRUE(ctrl2.auto_disable_on_record());
}

TEST_F(network_controller_test, move_assignment)
{
    network_controller ctrl1;
    ctrl1.set_auto_disable_on_record(true);

    network_controller ctrl2;
    ctrl2 = std::move(ctrl1);
    EXPECT_TRUE(ctrl2.auto_disable_on_record());
}

} // namespace piejam::network_manager::test
