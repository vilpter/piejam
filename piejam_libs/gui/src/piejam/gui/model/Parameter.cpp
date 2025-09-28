// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/gui/model/Parameter.h>

#include <piejam/gui/model/BoolParameter.h>
#include <piejam/gui/model/FloatParameter.h>
#include <piejam/gui/model/IntParameter.h>
#include <piejam/gui/model/MidiAssignable.h>
#include <piejam/gui/model/StereoLevel.h>

#include <piejam/runtime/actions/set_parameter_value.h>
#include <piejam/runtime/selectors.h>
#include <piejam/runtime/ui/thunk_action.h>

#include <boost/mp11/list.hpp>
#include <boost/mp11/map.hpp>

namespace piejam::gui::model
{

namespace
{

using parameter_id_to_FxParameter = boost::mp11::mp_list<
        boost::mp11::mp_list<runtime::bool_parameter_id, BoolParameter>,
        boost::mp11::mp_list<runtime::float_parameter_id, FloatParameter>,
        boost::mp11::mp_list<runtime::int_parameter_id, IntParameter>>;

} // namespace

Parameter::Parameter(
        runtime::state_access const& state_access,
        ParameterId const& paramId)
    : SubscribableModel(state_access)
    , m_paramId{paramId}
{
    setName(QString::fromStdString(observe_once(
            runtime::selectors::make_fx_parameter_name_selector(m_paramId))));

    if (observe_once(
                runtime::selectors::make_parameter_is_midi_assignable_selector(
                        paramId)))
    {
        m_midi = std::make_unique<MidiAssignable>(state_access, paramId);
    }
}

Parameter::~Parameter() = default;

auto
Parameter::type() const noexcept -> Type
{
    return std::visit(
            []<class T>(T const&) -> Type {
                return boost::mp11::mp_at_c<
                        boost::mp11::
                                mp_map_find<parameter_id_to_FxParameter, T>,
                        1>::StaticType;
            },
            m_paramId);
}

void
Parameter::onSubscribe()
{
    observe(runtime::selectors::make_fx_parameter_value_string_selector(
                    m_paramId),
            [this](std::string const& text) {
                setValueString(QString::fromStdString(text));
            });
}

auto
Parameter::midi() const noexcept -> MidiAssignable*
{
    return m_midi.get();
}

void
Parameter::resetToDefault()
{
    dispatch(runtime::actions::reset_parameter_to_default_value(m_paramId));
}

auto
Parameter::paramId() const -> ParameterId
{
    return m_paramId;
}

auto
makeParameter(
        runtime::state_access const& state_access,
        piejam::gui::model::ParameterId const& paramId)
        -> std::unique_ptr<Parameter>
{
    return std::visit(
            [&]<class ParamId>(ParamId const typed_param_id)
                    -> std::unique_ptr<Parameter> {
                using FxParameterType = boost::mp11::mp_at_c<
                        boost::mp11::mp_map_find<
                                parameter_id_to_FxParameter,
                                ParamId>,
                        1>;
                return std::make_unique<FxParameterType>(
                        state_access,
                        typed_param_id);
            },
            paramId);
}

} // namespace piejam::gui::model
