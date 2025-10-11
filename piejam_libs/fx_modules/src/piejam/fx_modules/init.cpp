// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/fx_modules/dual_pan/dual_pan_internal_id.h>
#include <piejam/fx_modules/dual_pan/gui/FxDualPan.h>
#include <piejam/fx_modules/filter/filter_internal_id.h>
#include <piejam/fx_modules/filter/gui/FxFilter.h>
#include <piejam/fx_modules/scope/gui/FxScope.h>
#include <piejam/fx_modules/scope/scope_internal_id.h>
#include <piejam/fx_modules/spectrum/gui/FxSpectrum.h>
#include <piejam/fx_modules/spectrum/spectrum_internal_id.h>
#include <piejam/fx_modules/tuner/gui/FxTuner.h>
#include <piejam/fx_modules/tuner/tuner_internal_id.h>
#include <piejam/fx_modules/utility/gui/FxUtility.h>
#include <piejam/fx_modules/utility/utility_internal_id.h>

#include <QDir>
#include <QtQml>

#include <mutex>

static void
initResources()
{
    Q_INIT_RESOURCE(piejam_fx_modules_resources);
}

#define PIEJAM_FX_MODEL(type, name)                                            \
    qmlRegisterUncreatableType<type>(                                          \
        "PieJam.FxModules.Models",                                             \
        1,                                                                     \
        0,                                                                     \
        name,                                                                  \
        "C++ type");

namespace piejam::fx_modules
{

static void
registerTypes()
{
    PIEJAM_FX_MODEL(dual_pan::gui::FxDualPan, "FxDualPan");
    PIEJAM_FX_MODEL(filter::gui::FxFilter, "FxFilter");
    PIEJAM_FX_MODEL(scope::gui::FxScope, "FxScope");
    PIEJAM_FX_MODEL(spectrum::gui::FxSpectrum, "FxSpectrum");
    PIEJAM_FX_MODEL(tuner::gui::FxTuner, "FxTuner");
    PIEJAM_FX_MODEL(utility::gui::FxUtility, "FxUtility");
}

void
init()
{
    static std::once_flag s_init;
    std::call_once(s_init, []() {
        initResources();
        registerTypes();

        dual_pan::internal_id();
        filter::internal_id();
        scope::internal_id();
        spectrum::internal_id();
        tuner::internal_id();
        utility::internal_id();
    });
}

} // namespace piejam::fx_modules
