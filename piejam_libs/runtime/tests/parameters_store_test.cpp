// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/entity_id.h>
#include <piejam/runtime/parameter/store.h>

#include <gtest/gtest.h>

namespace piejam::runtime::parameter::test
{

struct float_parameter
{
    using value_type = float;

    float default_value{1.f};

    constexpr auto operator==(float_parameter const&) const noexcept
            -> bool = default;
};

TEST(store, add_default_get)
{
    store sut;

    auto id = id_t<float_parameter>::generate();
    sut.emplace(id, float_parameter{});

    EXPECT_EQ(1.f, sut.at(id).get());
}

TEST(store, add_remove)
{
    store sut;

    auto id = id_t<float_parameter>::generate();
    sut.emplace(id, {});
    ASSERT_TRUE(sut.contains(id));
    sut.remove(id);

    EXPECT_FALSE(sut.contains(id));
}

TEST(store, get_existing_parameter)
{
    store sut;

    auto id = id_t<float_parameter>::generate();
    sut.emplace(id, {});
    EXPECT_NE(nullptr, sut.find(id));
}

TEST(store, find_for_non_existing_parameter)
{
    store sut;

    EXPECT_EQ(nullptr, sut.find(parameter::id_t<float_parameter>::generate()));
}

TEST(store, get_cached_for_existing_parameter)
{
    store sut;

    auto id = id_t<float_parameter>::generate();
    sut.emplace(id, {});
    auto desc = sut.find(id);
    ASSERT_NE(nullptr, desc);
    EXPECT_EQ(1.f, *desc->cached());
}

TEST(store, set_value)
{
    store sut;

    auto id = id_t<float_parameter>::generate();
    sut.emplace(id, {});

    sut.at(id).set(2.f);
    EXPECT_EQ(2.f, sut.at(id).get());
}

TEST(store, cached_is_updated_after_set)
{
    store sut;

    auto id = id_t<float_parameter>::generate();
    sut.emplace(id, {});

    auto cached = sut.at(id).cached();
    ASSERT_NE(nullptr, cached);
    EXPECT_EQ(1.f, *cached);

    sut.at(id).set(2.f);

    EXPECT_EQ(2.f, *cached);
}

TEST(store, equality)
{
    store m1;

    auto id = id_t<float_parameter>::generate();
    m1.emplace(id, {});

    EXPECT_EQ(m1, m1);

    store m2;

    m2.emplace(id, {});

    EXPECT_NE(m1, m2);

    auto m3 = m1;

    EXPECT_EQ(m1, m3);
}

} // namespace piejam::runtime::parameter::test
