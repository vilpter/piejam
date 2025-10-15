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
    Impl(
        runtime::state_access const& state_access,
        runtime::parameter_id const& param_id)
        : param_id{param_id}
    {
        if (state_access.observe_once(
                runtime::selectors::make_parameter_is_midi_assignable_selector(
                    param_id)))
        {
            midi = std::make_unique<MidiAssignable>(state_access, param_id);
        }
    }

    runtime::parameter_id param_id;
    std::unique_ptr<MidiAssignable> midi;
};

Parameter::Parameter(
    runtime::state_access const& state_access,
    runtime::parameter_id const& param_id)
    : SubscribableModel(state_access)
    , m_impl{make_pimpl<Impl>(state_access, param_id)}
{
    setName(
        QString::fromStdString(observe_once(
            runtime::selectors::make_parameter_name_selector(param_id))));
}

void
Parameter::onSubscribe()
{
    observe(
        runtime::selectors::make_parameter_value_string_selector(
            m_impl->param_id),
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
        runtime::actions::reset_parameter_to_default_value(m_impl->param_id));
}

auto
Parameter::paramId() const -> runtime::parameter_id
{
    return m_impl->param_id;
}

} // namespace piejam::gui::model
