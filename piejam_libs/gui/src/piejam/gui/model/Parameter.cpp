// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/gui/model/Parameter.h>

#include <piejam/gui/model/MidiAssignable.h>

#include <piejam/runtime/actions/set_parameter_value.h>
#include <piejam/runtime/selectors.h>
#include <piejam/runtime/ui/thunk_action.h>

#include <boost/mp11/list.hpp>
#include <boost/mp11/map.hpp>

namespace piejam::gui::model
{

struct Parameter::Impl
{
    Impl(runtime::state_access const& state_access, ParameterId const& paramId)
        : paramId{paramId}
    {
        if (state_access.observe_once(
                runtime::selectors::make_parameter_is_midi_assignable_selector(
                    paramId)))
        {
            midi = std::make_unique<MidiAssignable>(state_access, paramId);
        }
    }

    ParameterId paramId;
    std::unique_ptr<MidiAssignable> midi;
};

Parameter::Parameter(
    runtime::state_access const& state_access,
    ParameterId const& paramId)
    : SubscribableModel(state_access)
    , m_impl{make_pimpl<Impl>(state_access, paramId)}
{
    setName(
        QString::fromStdString(observe_once(
            runtime::selectors::make_parameter_name_selector(paramId))));
}

void
Parameter::onSubscribe()
{
    observe(
        runtime::selectors::make_parameter_value_string_selector(
            m_impl->paramId),
        [this](std::string const& text) {
            setValueString(QString::fromStdString(text));
        });
}

auto
Parameter::midi() const noexcept -> MidiAssignable*
{
    return m_impl->midi.get();
}

void
Parameter::resetToDefault()
{
    dispatch(
        runtime::actions::reset_parameter_to_default_value(m_impl->paramId));
}

auto
Parameter::paramId() const -> ParameterId
{
    return m_impl->paramId;
}

} // namespace piejam::gui::model
