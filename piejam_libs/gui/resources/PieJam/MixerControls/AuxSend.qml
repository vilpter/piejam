// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15

import PieJam.Models 1.0 as PJModels
import PieJam.ParameterControls 1.0

import ".."

SubscribableItem {
    id: root

    implicitHeight: 96

    Frame {
        anchors.fill: parent

        spacing: 0
        padding: 4
        topPadding: 2
        bottomPadding: 0

        ColumnLayout {
            anchors.fill: parent

            Label {
                Layout.fillWidth: true
                Layout.preferredHeight: 24

                text: root.model ? root.model.name : ""
            }

            FloatSlider {
                Layout.fillWidth: true
                Layout.fillHeight: true

                model: root.model ? root.model.volume : null
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Button {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40

                    text: root.model && root.model.faderTap === PJModels.AuxSend.FaderTap.Pre ? "PRE" : "POST"

                    enabled: root.model

                    onClicked: root.model.toggleFaderTap()
                }

                Button {
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40


                    icon.source: (!root.model || root.model.canToggle)
                                 ? "qrc:///images/icons/power.svg"
                                 : "qrc:///images/icons/cycle_arrows.svg"
                    checkable: true
                    checked: root.model && root.model.enabled
                    enabled: root.model && root.model.canToggle

                    onClicked: root.model.toggleEnabled()
                }
            }
        }
    }
}
