// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/gui/model/FxGenericModule.h>

#include <piejam/gui/model/BoolParameter.h>
#include <piejam/gui/model/EnumParameter.h>
#include <piejam/gui/model/FloatParameter.h>
#include <piejam/gui/model/IntParameter.h>
#include <piejam/gui/model/ObjectListModel.h>
#include <piejam/gui/model/Parameter.h>
#include <piejam/gui/model/ParameterId.h>

#include <piejam/runtime/parameter/map.h>
#include <piejam/runtime/selectors.h>

#include <boost/container/flat_map.hpp>
#include <boost/mp11/map.hpp>

namespace piejam::gui::model
{

namespace
{

using IdToModel = boost::mp11::mp_list<
        boost::mp11::mp_list<runtime::bool_parameter_id, BoolParameter>,
        boost::mp11::mp_list<runtime::enum_parameter_id, EnumParameter>,
        boost::mp11::mp_list<runtime::float_parameter_id, FloatParameter>,
        boost::mp11::mp_list<runtime::int_parameter_id, IntParameter>>;

auto
makeParameter(
        runtime::state_access const& state_access,
        piejam::gui::model::ParameterId const& paramId)
        -> std::unique_ptr<Parameter>
{
    return std::visit(
            [&]<class ParamId>(ParamId const typed_param_id)
                    -> std::unique_ptr<Parameter> {
                using ParameterModel = boost::mp11::mp_second<
                        boost::mp11::mp_map_find<IdToModel, ParamId>>;
                return std::make_unique<ParameterModel>(
                        state_access,
                        typed_param_id);
            },
            paramId);
}

} // namespace

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
        m_impl->parametersList.add(
                m_impl->parametersList.size(),
                model::makeParameter(state_access, paramId));
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
