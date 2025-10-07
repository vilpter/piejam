// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/runtime/fx/fwd.h>
#include <piejam/runtime/fx/module.h>

#include <piejam/boxed_map.h>
#include <piejam/entity_id_hash.h>
#include <piejam/entity_map.h>

namespace piejam::runtime::fx
{

using active_modules_t =
    boxed_map<boost::container::flat_map<fx::module_id, bool_parameter_id>>;

using ladspa_instances_t =
    std::unordered_map<ladspa::instance_id, ladspa::plugin_descriptor>;

using unavailable_ladspa_plugins_t = boxed_map<entity_map<unavailable_ladspa>>;

struct state
{
    modules_t modules;
    active_modules_t active_modules;

    ladspa_instances_t ladspa_instances;
    unavailable_ladspa_plugins_t unavailable_ladspa_plugins;
};

} // namespace piejam::runtime::fx
