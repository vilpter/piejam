// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/gui/model/AudioInputOutputSettings.h>

#include <piejam/algorithm/edit_script.h>
#include <piejam/audio/types.h>
#include <piejam/gui/generic_list_model_edit_script_executor.h>
#include <piejam/gui/model/ExternalAudioDeviceConfig.h>
#include <piejam/gui/model/GenericListModel.h>
#include <piejam/runtime/actions/external_audio_device_actions.h>
#include <piejam/runtime/selectors.h>

namespace piejam::gui::model
{

struct AudioInputOutputSettings::Impl
{
    io_direction io_dir;
    box<runtime::external_audio::device_ids_t> device_ids{};

    ExternalAudioDeviceConfigList deviceConfigs{};
};

AudioInputOutputSettings::AudioInputOutputSettings(
        runtime::state_access const& state_access,
        io_direction const settings_type)
    : SubscribableModel(state_access)
    , m_impl{make_pimpl<Impl>(settings_type)}
{
}

auto
AudioInputOutputSettings::deviceConfigs() const noexcept -> QAbstractListModel*
{
    return &m_impl->deviceConfigs;
}

void
AudioInputOutputSettings::onSubscribe()
{
    namespace selectors = runtime::selectors;

    observe(selectors::make_num_device_channels_selector(m_impl->io_dir),
            [this](std::size_t const num_input_channels) {
                QStringList channels;
                channels.push_back("-");
                for (std::size_t n = 0; n < num_input_channels; ++n)
                {
                    channels.push_back(QString::number(n + 1));
                }

                setChannels(channels);
            });

    observe(selectors::make_external_audio_device_ids_selector(m_impl->io_dir),
            [this](box<runtime::external_audio::device_ids_t> const&
                           device_ids) {
                algorithm::apply_edit_script(
                        algorithm::edit_script(
                                *m_impl->device_ids,
                                *device_ids),
                        piejam::gui::generic_list_model_edit_script_executor{
                                m_impl->deviceConfigs,
                                [this](runtime::external_audio::device_id
                                               device_id) {
                                    return std::make_unique<
                                            ExternalAudioDeviceConfig>(
                                            state_access(),
                                            device_id);
                                }});

                m_impl->device_ids = device_ids;
            });
}

static void
addDevice(
        runtime::state_access state_access,
        io_direction direction,
        audio::bus_type bus_type)
{
    runtime::actions::add_external_audio_device action;
    action.direction = direction;
    action.type = bus_type;
    state_access.dispatch(action);
}

void
AudioInputOutputSettings::addMonoDevice()
{
    addDevice(state_access(), m_impl->io_dir, audio::bus_type::mono);
}

void
AudioInputOutputSettings::addStereoDevice()
{
    addDevice(state_access(), m_impl->io_dir, audio::bus_type::stereo);
}

} // namespace piejam::gui::model
