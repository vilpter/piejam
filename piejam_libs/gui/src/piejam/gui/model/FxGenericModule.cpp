// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/gui/model/FxGenericModule.h>

#include <piejam/gui/model/ObjectListModel.h>
#include <piejam/gui/model/Parameter.h>
#include <piejam/gui/model/ParameterId.h>

#include <piejam/runtime/selectors.h>

#include <boost/container/flat_map.hpp>

namespace piejam::gui::model
{

struct FxGenericModule::Impl
{
    FxParametersList parametersList;
};

FxGenericModule::FxGenericModule(
        runtime::state_access const& state_access,
        runtime::fx::module_id const fx_mod_id)
    : FxModule{state_access, fx_mod_id}
    , m_impl{make_pimpl<Impl>()}
{
    for (auto const& [key, paramId] : parameters())
    {
        auto param = model::makeParameter(state_access, paramId);
        m_impl->parametersList.add(
                m_impl->parametersList.size(),
                std::move(param));
    }
}

auto
FxGenericModule::parametersList() const noexcept -> QAbstractListModel*
{
    return &m_impl->parametersList;
}

void
FxGenericModule::onSubscribe()
{
}

} // namespace piejam::gui::model
