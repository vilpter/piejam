// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15

import PieJam.Controls 1.0
import PieJam.Models 1.0 as PJModels

import ".."

SubscribableItem {
    id: root

    implicitWidth: 132

    Material.primary: root.model ? root.model.color : Material.Pink
    Material.accent: root.model ? root.model.color : Material.Pink

    Frame {
        anchors.fill: parent

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            HeaderLabel {
                Layout.fillWidth: true
                Layout.preferredHeight: 24

                text: root.model ? root.model.name : ""
            }

            AudioRoutingComboBox {
                Layout.fillWidth: true

                hasSelectableDefault: false
                defaultText: "Send..."
                model: root.model ? root.model.aux : null
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0

                    VolumeFader {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        visible: root.model && root.model.volume

                        model: root.model ? root.model.volume : null

                        scaleData: PJModels.MixerDbScales.sendFaderScale

                        onMoved: {
                            if (root.model && root.model.canToggle && !root.model.enabled)
                            {
                                root.model.toggleEnabled()
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 40

                        visible: root.model && root.model.volume

                        Button {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 40

                            text: root.model && root.model.faderTap === PJModels.MixerChannelAuxSend.FaderTap.Pre ? "PRE" : "POST"

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
    }
}
