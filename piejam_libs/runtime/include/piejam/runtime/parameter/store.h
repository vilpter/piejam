// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/runtime/parameter/bool_descriptor.h>
#include <piejam/runtime/parameter/float_descriptor.h>
#include <piejam/runtime/parameter/fwd.h>
#include <piejam/runtime/parameter/int_descriptor.h>

#include <piejam/lean_map_facade.h>

#include <boost/container/flat_map.hpp>

#include <memory>
#include <tuple>

namespace piejam::runtime::parameter
{

class store
{
public:
    template <class Parameter>
    struct slot
    {
        using parameter_type = Parameter;
        using value_type = parameter::value_type_t<Parameter>;
        using cached_type = std::shared_ptr<value_type const>;

        explicit slot(Parameter&& param)
            : m_param{std::move(param)}
        {
        }

        explicit slot(Parameter const& param)
            : m_param{param}
        {
        }

        [[nodiscard]]
        auto param() const noexcept -> Parameter const&
        {
            return m_param;
        }

        [[nodiscard]]
        auto get() const noexcept -> value_type
        {
            return *m_value;
        }

        [[nodiscard]]
        auto cached() const noexcept -> cached_type
        {
            return m_value;
        }

        void set(value_type value) noexcept
        {
            *m_value = value;
        }

        auto operator==(slot const& other) const noexcept -> bool
        {
            return m_param == other.m_param && get() == other.get();
        }

    private:
        Parameter m_param;
        std::shared_ptr<value_type> m_value{
                std::make_shared<value_type>(m_param.default_value)};
    };

    template <class P>
    auto emplace(id_t<P> const id, P&& param)
    {
        return BOOST_VERIFY(
                get_map<P>()
                        .emplace(id, slot<P>{std::forward<P>(param)})
                        .second);
    }

    template <class P>
    auto remove(id_t<P> const id) -> void
    {
        get_map<P>().erase(id);
    }

    template <class P>
    auto contains(id_t<P> const id) const noexcept -> bool
    {
        return get_map<P>().contains(id);
    }

    template <class P>
    auto find(id_t<P> const id) const noexcept -> slot<P> const*
    {
        return get_map<P>().find(id);
    }

    template <class P>
    auto find(id_t<P> const id) noexcept -> slot<P>*
    {
        return get_map<P>().find(id);
    }

    template <class P>
    auto at(id_t<P> const id) const noexcept -> slot<P> const&
    {
        return get_map<P>().at(id);
    }

    template <class P>
    auto at(id_t<P> const id) noexcept -> slot<P>&
    {
        return get_map<P>().at(id);
    }

    auto operator==(store const& other) const noexcept -> bool = default;

private:
    template <class P>
    using map_t = lean_map_facade<boost::container::flat_map<id_t<P>, slot<P>>>;

    using maps_t = std::tuple<
            map_t<bool_descriptor>,
            map_t<float_descriptor>,
            map_t<int_descriptor>>;

    template <class P>
    auto get_map() const -> map_t<P> const&
    {
        return std::get<map_t<P>>(*m_maps);
    }

    template <class P>
    auto get_map() -> map_t<P>&
    {
        return std::get<map_t<P>>(*m_maps);
    }

    std::shared_ptr<maps_t> m_maps{std::make_shared<maps_t>()};
};

} // namespace piejam::runtime::parameter
