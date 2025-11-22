// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15

Dialog {
    id: root

    property alias message: messageLabel.text

    signal canceled()
    signal yesClicked()
    signal noClicked()

    parent: Overlay.overlay
    anchors.centerIn: Overlay.overlay

    modal: true

    Label {
        id: messageLabel

        textFormat: Text.PlainText
    }

    footer: Item {
        implicitHeight: 64

        Material.foreground: Material.accentColor

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 24
            anchors.rightMargin: 24
            anchors.bottomMargin: 24
            anchors.topMargin: 0

            Button {
                text: qsTr("Cancel")

                onClicked: {
                    root.close()
                    root.canceled();
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: qsTr("Yes")

                onClicked: {
                    root.close()
                    root.yesClicked()
                }
            }

            Button {
                text: qsTr("No")

                onClicked: {
                    root.close()
                    root.noClicked()
                }
            }
        }
    }
}
