// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2026  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15

Dialog {
    id: root

    property string ssid: ""
    property string securityType: ""
    property alias password: passwordField.text
    property alias rememberNetwork: rememberCheckBox.checked

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: 400

    title: qsTr("Connect to ") + root.ssid

    modal: true
    focus: true

    standardButtons: Dialog.Cancel | Dialog.Ok

    onOpened: {
        passwordField.text = ""
        passwordField.forceActiveFocus()
    }

    onAccepted: {
        // Will be handled by caller
    }

    onRejected: {
        passwordField.text = ""
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 16

        // Security type display
        RowLayout {
            Layout.fillWidth: true
            visible: root.securityType !== "" && root.securityType !== "Open"

            Label {
                text: qsTr("Security:")
                font.bold: true
            }

            Label {
                text: root.securityType
            }
        }

        // Password field (hidden for open networks)
        ColumnLayout {
            Layout.fillWidth: true
            visible: root.securityType !== "Open"
            spacing: 4

            Label {
                text: qsTr("Password")
            }

            TextField {
                id: passwordField

                Layout.fillWidth: true
                Layout.preferredWidth: 300

                placeholderText: qsTr("Enter password")
                echoMode: showPasswordButton.checked ? TextInput.Normal : TextInput.Password
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText

                Keys.onReturnPressed: root.accept()
                Keys.onEnterPressed: root.accept()

                Button {
                    id: showPasswordButton

                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.rightMargin: 4

                    flat: true
                    checkable: true
                    text: checked ? qsTr("Hide") : qsTr("Show")
                    font.pixelSize: 12
                }
            }
        }

        // Remember network checkbox
        CheckBox {
            id: rememberCheckBox

            text: qsTr("Remember this network")
            checked: true
        }

        // Info text for open networks
        Label {
            Layout.fillWidth: true
            visible: root.securityType === "Open"
            text: qsTr("This is an open network. No password required.")
            font.italic: true
            opacity: 0.7
            wrapMode: Text.WordWrap
        }
    }
}
