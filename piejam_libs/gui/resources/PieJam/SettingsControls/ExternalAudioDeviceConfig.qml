// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import PieJam.Controls 1.0

SubscribableItem {
    id: root

    property var channels: [ "-" ]

    RowLayout {
        anchors.fill: parent
        anchors.margins: 4

        StringTextField {
            Layout.fillWidth: true

            model: root.model ? root.model.name : null
        }

        StackLayout {
            Layout.maximumWidth: 128
            Layout.topMargin: 4
            Layout.bottomMargin: 4

            currentIndex: root.model ? (root.model.mono ? 0 : 1) : -1

            ChoiceBox {
                Layout.fillWidth: true
                Layout.fillHeight: true

                model: root.channels
                currentIndex: root.model ? root.model.monoChannel : -1
                textHorizontalAlignment: Text.AlignHCenter

                textRole: "text"

                onActivated: root.model.changeMonoChannel(index)
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ChoiceBox {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    model: root.channels
                    currentIndex: root.model ? root.model.stereoLeftChannel : -1
                    textHorizontalAlignment: Text.AlignHCenter

                    textRole: "text"

                    onActivated: root.model.changeStereoLeftChannel(index)
                }

                ChoiceBox {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    model: root.channels
                    currentIndex: root.model ? root.model.stereoRightChannel : -1
                    textHorizontalAlignment: Text.AlignHCenter

                    textRole: "text"

                    onActivated: root.model.changeStereoRightChannel(index)
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
