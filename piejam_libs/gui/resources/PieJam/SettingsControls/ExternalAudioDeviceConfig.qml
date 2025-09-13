// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import PieJam.Controls 1.0 as PJControls

import ".."

SubscribableItem {
    id: root

    property variant channels: [ "-" ]

    RowLayout {
        anchors.fill: parent
        anchors.margins: 4

        PJControls.StringTextField {
            Layout.fillWidth: true

            model: root.model ? root.model.name : null
        }

        StackLayout {
            Layout.maximumWidth: 128

            currentIndex: root.model ? (root.model.mono ? 0 : 1) : -1

            PJControls.ComboBox {
                Layout.fillWidth: true
                Layout.fillHeight: true

                model: root.channels
                currentIndex: root.model ? root.model.monoChannel : -1

                onActivated: if (root.model) root.model.changeMonoChannel(index)
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ComboBox {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    model: root.channels
                    currentIndex: root.model ? root.model.stereoLeftChannel : -1

                    onActivated: if (root.model) root.model.changeStereoLeftChannel(index)
                }

                ComboBox {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    model: root.channels
                    currentIndex: root.model ? root.model.stereoRightChannel : -1

                    onActivated: if (root.model) root.model.changeStereoRightChannel(index)
                }
            }
        }

        Button {
            Layout.preferredWidth: 38

            text: "X"
            font.bold: true

            onClicked: if (root.model) root.model.remove()
        }
    }
}
