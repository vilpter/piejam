// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/entity_id.h>
#include <piejam/runtime/parameter/store.h>
#include <piejam/runtime/parameters_store.h>

#include <gtest/gtest.h>

namespace piejam::runtime::parameter::test
{

struct float_parameter
{
    using value_type = float;

    float default_value{1.f};
};

TEST(parameters_store, add_default_get)
{
    parameters_store sut;

    auto id = id_t<float_parameter>::generate();
    sut.emplace(id);

    EXPECT_EQ(1.f, sut.at(id).value.get());
}

TEST(parameters_store, add_remove)
{
    parameters_store sut;

    auto id = id_t<float_parameter>::generate();
    sut.emplace(id);
    ASSERT_TRUE(sut.contains(id));
    sut.remove(id);

    EXPECT_FALSE(sut.contains(id));
}

TEST(parameters_store, get_existing_parameter)
{
    parameters_store sut;

    auto id = id_t<float_parameter>::generate();
    sut.emplace(id);
    EXPECT_NE(nullptr, sut.find(id));
}

TEST(parameters_store, find_for_non_existing_parameter)
{
    parameters_store sut;

    EXPECT_EQ(nullptr, sut.find(parameter::id_t<float_parameter>::generate()));
}

TEST(parameters_store, get_cached_for_existing_parameter)
{
    parameters_store sut;

    auto id = id_t<float_parameter>::generate();
    sut.emplace(id);
    auto desc = sut.find(id);
    ASSERT_NE(nullptr, desc);
    EXPECT_EQ(1.f, *desc->value.cached());
}

TEST(parameters_store, set_value)
{
    parameters_store sut;

    auto id = id_t<float_parameter>::generate();
    sut.emplace(id);

    sut.at(id).value.set(2.f);
    EXPECT_EQ(2.f, sut.at(id).value.get());
}

TEST(parameters_store, cached_is_updated_after_set)
{
    parameters_store sut;

    auto id = id_t<float_parameter>::generate();
    sut.emplace(id);

    auto cached = sut.at(id).value.cached();
    ASSERT_NE(nullptr, cached);
    EXPECT_EQ(1.f, *cached);

    sut.at(id).value.set(2.f);

    EXPECT_EQ(2.f, *cached);
}

TEST(parameters_store, equality)
{
    parameters_store m1;

    auto id = id_t<float_parameter>::generate();
    m1.emplace(id);

    EXPECT_EQ(m1, m1);

    parameters_store m2;

    m2.emplace(id);

    EXPECT_NE(m1, m2);

    auto m3 = m1;

    EXPECT_EQ(m1, m3);
}

} // namespace piejam::runtime::parameter::test
