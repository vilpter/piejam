// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2026 PieJam Fork Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15

import PieJam.Models 1.0 as PJModels

Item {
    id: root

    property var model: null

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 16

        // Network Enable/Disable Section
        Frame {
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 8

                Label {
                    text: qsTr("Network Control")
                    font.pixelSize: 20
                    font.bold: true
                }

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        text: qsTr("Network Enabled")
                        Layout.fillWidth: true
                    }

                    Switch {
                        id: networkSwitch
                        checked: root.model ? root.model.networkEnabled : false
                        onToggled: {
                            if (root.model) {
                                root.model.toggleNetwork()
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        text: qsTr("Auto-disable during recording")
                        Layout.fillWidth: true
                    }

                    Switch {
                        id: autoDisableSwitch
                        checked: root.model ? root.model.autoDisableOnRecord : false
                        onToggled: {
                            // TODO: Need to add setter for autoDisableOnRecord
                        }
                    }
                }
            }
        }

        // WiFi Status Section
        Frame {
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 8

                Label {
                    text: qsTr("WiFi Status")
                    font.pixelSize: 20
                    font.bold: true
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: 2
                    columnSpacing: 16
                    rowSpacing: 4

                    Label {
                        text: qsTr("Status:")
                        font.bold: true
                    }
                    Label {
                        text: root.model && root.model.wifiConnected
                              ? qsTr("Connected")
                              : (root.model && root.model.isConnecting
                                 ? qsTr("Connecting...")
                                 : qsTr("Disconnected"))
                        color: root.model && root.model.wifiConnected
                               ? Material.color(Material.Green)
                               : Material.foreground
                    }

                    Label {
                        text: qsTr("Network:")
                        font.bold: true
                        visible: root.model && root.model.wifiConnected
                    }
                    Label {
                        text: root.model ? root.model.currentSsid : ""
                        visible: root.model && root.model.wifiConnected
                    }

                    Label {
                        text: qsTr("IP Address:")
                        font.bold: true
                        visible: root.model && root.model.wifiConnected
                    }
                    Label {
                        text: root.model ? root.model.ipAddress : ""
                        visible: root.model && root.model.wifiConnected
                    }

                    Label {
                        text: qsTr("Signal:")
                        font.bold: true
                        visible: root.model && root.model.wifiConnected
                    }
                    RowLayout {
                        visible: root.model && root.model.wifiConnected

                        ProgressBar {
                            Layout.preferredWidth: 100
                            from: 0
                            to: 100
                            value: root.model ? root.model.signalStrength : 0
                        }

                        Label {
                            text: (root.model ? root.model.signalStrength : 0) + "%"
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Button {
                        text: qsTr("Scan Networks")
                        enabled: root.model && !root.model.isScanning
                        onClicked: {
                            if (root.model) {
                                root.model.scanNetworks()
                            }
                        }

                        BusyIndicator {
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.rightMargin: 8
                            width: 24
                            height: 24
                            running: root.model && root.model.isScanning
                            visible: running
                        }
                    }

                    Button {
                        text: qsTr("Disconnect")
                        visible: root.model && root.model.wifiConnected
                        onClicked: {
                            if (root.model) {
                                root.model.disconnectWifi()
                            }
                        }
                    }

                    Button {
                        text: qsTr("Refresh")
                        onClicked: {
                            if (root.model) {
                                root.model.refreshStatus()
                            }
                        }
                    }
                }
            }
        }

        // Tab bar for WiFi / NFS Server / NFS Client
        TabBar {
            id: networkTabBar
            Layout.fillWidth: true

            TabButton {
                text: qsTr("WiFi")
            }
            TabButton {
                text: qsTr("NFS Server")
            }
            TabButton {
                text: qsTr("NFS Client")
            }
        }

        // Stacked layout for tab content
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: networkTabBar.currentIndex

            // WiFi Tab - Available Networks Section
            Frame {
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8

                    RowLayout {
                        Layout.fillWidth: true

                        Label {
                            text: qsTr("Available Networks")
                            font.pixelSize: 20
                            font.bold: true
                            Layout.fillWidth: true
                        }

                        Button {
                            text: qsTr("Saved Networks")
                            flat: true
                            onClicked: savedNetworksPopup.open()
                        }
                    }

                    ListView {
                        id: networkList

                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        clip: true
                        model: root.model ? root.model.availableNetworks() : null

                        delegate: WiFiNetworkDelegate {
                            width: networkList.width
                            ssid: model.ssid
                            signalPercent: model.signalPercent
                            security: model.security
                            band: model.band
                            isConnected: model.isConnected

                            onClicked: {
                                if (!isConnected) {
                                    connectionDialog.ssid = ssid
                                    connectionDialog.securityType = security
                                    connectionDialog.open()
                                }
                            }
                        }

                        ScrollBar.vertical: ScrollBar {}

                        Label {
                            anchors.centerIn: parent
                            text: root.model && root.model.isScanning
                                  ? qsTr("Scanning...")
                                  : qsTr("No networks found.\nTap 'Scan Networks' to search.")
                            horizontalAlignment: Text.AlignHCenter
                            visible: networkList.count === 0
                            opacity: 0.5
                        }
                    }
                }
            }

            // NFS Server Tab
            NFSServerSettings {
                model: root.model
            }

            // NFS Client Tab
            NFSClientSettings {
                model: root.model
            }
        }
    }

    // Connection Dialog
    WiFiConnectionDialog {
        id: connectionDialog

        onAccepted: {
            if (root.model) {
                root.model.connectToNetwork(ssid, password, rememberNetwork)
            }
        }
    }

    // Saved Networks Popup
    Popup {
        id: savedNetworksPopup

        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 400
        height: 300

        modal: true
        focus: true

        ColumnLayout {
            anchors.fill: parent
            spacing: 8

            Label {
                text: qsTr("Saved Networks")
                font.pixelSize: 18
                font.bold: true
            }

            ListView {
                id: savedNetworkList

                Layout.fillWidth: true
                Layout.fillHeight: true

                clip: true
                model: root.model ? root.model.savedNetworks() : null

                delegate: ItemDelegate {
                    width: savedNetworkList.width
                    text: model.ssid

                    RowLayout {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.rightMargin: 8

                        Label {
                            text: model.security
                            opacity: 0.6
                        }

                        ToolButton {
                            text: "X"
                            onClicked: {
                                if (root.model) {
                                    root.model.forgetNetwork(model.ssid)
                                }
                            }
                        }
                    }
                }

                Label {
                    anchors.centerIn: parent
                    text: qsTr("No saved networks")
                    visible: savedNetworkList.count === 0
                    opacity: 0.5
                }
            }

            Button {
                Layout.alignment: Qt.AlignRight
                text: qsTr("Close")
                onClicked: savedNetworksPopup.close()
            }
        }
    }

    // Handle signals from model
    Connections {
        target: root.model

        function onConnectionSucceeded(ssid) {
            // Could show a toast/notification
        }

        function onConnectionFailed(ssid, error) {
            errorDialog.text = qsTr("Failed to connect to %1:\n%2").arg(ssid).arg(error)
            errorDialog.open()
        }

        function onNetworkError(message) {
            errorDialog.text = message
            errorDialog.open()
        }
    }

    // Error Dialog
    Dialog {
        id: errorDialog

        property alias text: errorLabel.text

        parent: Overlay.overlay
        anchors.centerIn: parent
        title: qsTr("Network Error")
        standardButtons: Dialog.Ok

        Label {
            id: errorLabel
        }
    }
}
