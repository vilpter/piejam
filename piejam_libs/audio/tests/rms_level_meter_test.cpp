// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/audio/dsp/rms_level_meter.h>

#include <gtest/gtest.h>

namespace piejam::audio::dsp::test
{

using namespace std::chrono_literals;

class rms_level_meter_test : public ::testing::Test
{
protected:
    sample_rate sr{48000}; // 48 kHz test rate
};

TEST_F(rms_level_meter_test, initial_state)
{
    rms_level_meter<float> meter(sr);
    EXPECT_FLOAT_EQ(meter.level(), 0.0f);
}

TEST_F(rms_level_meter_test, reset)
{
    rms_level_meter<float> meter(sr);
    std::vector<float> samples(100, 0.5f);
    meter.process(samples);
    meter.reset();
    EXPECT_FLOAT_EQ(meter.level(), 0.0f);
}

TEST_F(rms_level_meter_test, constant_input)
{
    rms_level_meter<float> meter(sr, 100ms);

    // Fill the entire buffer with 0.5
    std::size_t history_size = meter.history_size();
    std::vector<float> samples(history_size, 0.5f);
    meter.process(samples);

    float expected = 0.5f; // RMS of constant 0.5
    EXPECT_NEAR(meter.level(), expected, 1e-6);
}

TEST_F(rms_level_meter_test, varying_input)
{
    rms_level_meter<float> meter(sr, 100ms);

    // Fill the buffer with alternating 0 and 1
    std::size_t history_size = meter.history_size();
    std::vector<float> samples(history_size);
    for (std::size_t i = 0; i < history_size; ++i)
    {
        samples[i] = (i % 2 == 0) ? 0.0f : 1.0f;
    }

    meter.process(samples);

    float expected = std::sqrt(0.5f); // RMS of alternating 0 and 1
    EXPECT_NEAR(meter.level(), expected, 1e-6);
}

TEST_F(rms_level_meter_test, unaligned_input)
{
    using namespace std::chrono_literals;

    rms_level_meter<float> meter(sr, 1ms);

    std::array<float, 8> samples;
    samples.fill(0.5f);

    meter.process(std::span(std::next(samples.begin()), samples.end()));

    float expected = 0.19094f;
    EXPECT_NEAR(meter.level(), expected, 1e-6);
}

// Test flush-to-zero for very low levels
TEST_F(rms_level_meter_test, flush_to_zero)
{
    rms_level_meter<float> meter(sr, 50ms, 0.01f);
    std::vector<float> samples{0.005f, 0.003f};
    meter.process(samples);

    EXPECT_FLOAT_EQ(meter.level(), 0.0f);
}

// Test history buffer wrap
TEST_F(rms_level_meter_test, history_wrap)
{
    std::size_t history_samples = 128; // small buffer for testing
    rms_level_meter<float> meter(sr,
                                 2ms); // ~100 samples at 48kHz
    std::vector<float> samples(
            history_samples * 2,
            0.1f); // push more than buffer

    meter.process(samples);

    EXPECT_NEAR(meter.level(), 0.1f, 1e-6);
}

TEST_F(rms_level_meter_test, multiple_pushes)
{
    rms_level_meter<float> meter(sr, 100ms);

    // Fill the entire buffer with 0.5
    std::size_t history_size = meter.history_size();
    std::vector<float> chunk1(history_size, 0.5f);
    meter.process(chunk1);

    float level_after_first = meter.level();
    EXPECT_NEAR(level_after_first, 0.5f, 1e-6);

    // Push zeros to simulate decay
    std::vector<float> chunk2(history_size, 0.0f);
    meter.process(chunk2);

    float level_after_second = meter.level();
    EXPECT_LT(level_after_second, level_after_first); // should decay
}

} // namespace piejam::audio::dsp::test
