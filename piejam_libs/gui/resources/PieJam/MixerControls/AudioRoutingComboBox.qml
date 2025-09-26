// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15

import PieJam.Controls 1.0
import PieJam.Models 1.0 as PJModels

SubscribableItem {
    id: root

    property bool hasSelectableDefault: true
    property string defaultText: "-"

    implicitHeight: comboBox.implicitHeight

    ChoiceBox {
        id: comboBox

        anchors.fill: parent

        displayText: root.model
                ? (root.model.selected.isDefault
                    ? (root.model.selected.state === PJModels.AudioRoutingSelection.State.Valid ? root.defaultText : "???")
                    : root.model.selected.label)
                : "-"
        elideMode: Qt.ElideMiddle

        function selectedStateToColor(s) {
            switch (s) {
                case PJModels.AudioRoutingSelection.State.Invalid:
                    return Material.Red

                case PJModels.AudioRoutingSelection.State.NotMixed:
                    return Material.Yellow

                default:
                    return Material.primaryTextColor
            }
        }

        Material.foreground: selectedStateToColor(root.model ? root.model.selected.state : null)

        popup: Menu {
            MenuItem {
                enabled: root.hasSelectableDefault && root.model && root.model.defaultIsValid

                visible: root.hasSelectableDefault
                height: visible ? implicitHeight : 0

                text: root.defaultText

                onClicked: root.model.changeToDefault()
            }

            Menu {
                title: qsTr("Devices")

                enabled: devicesRep.count > 0

                Repeater {
                    id: devicesRep

                    model: root.model ? root.model.devices : null

                    delegate: MenuItem {
                        text: model.item.value

                        onClicked: root.model.changeToDevice(index)

                        ModelSubscription {
                            target: model.item
                            subscribed: parent.visible
                        }
                    }
                }
            }

            Menu {
                title: qsTr("Channels")

                enabled: channelsRep.count > 0

                Repeater {
                    id: channelsRep

                    model: root.model ? root.model.channels : null

                    delegate: MenuItem {
                        text: model.item.value

                        onClicked: root.model.changeToChannel(index)

                        ModelSubscription {
                            target: model.item
                            subscribed: parent.visible
                        }
                    }
                }
            }

            MenuSeparator {
                visible: !root.hasSelectableDefault && root.model
                enabled: visible
                height: visible ? implicitHeight : 0
            }

            MenuItem {
                text: "[Clear Selection]"

                visible: !root.hasSelectableDefault && root.model
                enabled: visible
                height: visible ? implicitHeight : 0

                onClicked: root.model.changeToDefault()
            }
        }
    }
}
