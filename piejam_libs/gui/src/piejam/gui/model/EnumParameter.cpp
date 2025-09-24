// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/gui/model/EnumParameter.h>

#include <piejam/gui/model/EnumListModel.h>

#include <piejam/runtime/selectors.h>

namespace piejam::gui::model
{

EnumParameter::EnumParameter(
        runtime::store_dispatch store_dispatch,
        runtime::subscriber& state_change_subscriber,
        runtime::parameter_id param_id)
    : IntParameter{store_dispatch, state_change_subscriber, param_id}
    , m_values{make_pimpl<EnumListModel>(observe_once(
              runtime::selectors::make_int_parameter_enum_values_selector(
                      std::get<runtime::int_parameter_id>(param_id))))}

{
}

auto
EnumParameter::values() const noexcept -> QAbstractListModel*
{
    return m_values.get();
}

} // namespace piejam::gui::model
