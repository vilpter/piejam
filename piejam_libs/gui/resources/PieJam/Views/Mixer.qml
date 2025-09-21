// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15

import QtQml 2.15

import PieJam.Controls 1.0
import PieJam.Models 1.0 as PJModels

import ".."
import "../MixerControls"

ViewPane {
    id: root

    Rectangle {
        width: 4
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        z: 100

        gradient: Gradient {

            orientation: Gradient.Horizontal

            GradientStop {
                position: 0
                color: "#ff000000"
            }

            GradientStop {
                position: 1
                color: "#00000000"
            }
        }
    }

    ListView {
        id: inputs

        anchors.left: parent.left
        anchors.right: mainChannelStrip.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 8
        anchors.leftMargin: 0

        spacing: 2
        clip: true
        orientation: ListView.Horizontal
        boundsBehavior: Flickable.StopAtBounds
        boundsMovement: Flickable.StopAtBounds

        model: root.model.userChannels

        delegate: ChannelStrip {
            anchors.top: parent ? parent.top : undefined
            anchors.bottom: parent ? parent.bottom : undefined

            model: item
        }

        header: Item {
            width: 8

            anchors.top: parent.top
            anchors.bottom: parent.bottom
        }

        footer: Item {
            id: channelAddStrip

            width: 142

            anchors.top: parent.top
            anchors.bottom: parent.bottom

            visible: MixerViewSettings.mode == MixerViewSettings.edit

            ChannelAddStrip {
                anchors.fill: parent
                anchors.leftMargin: 2
                anchors.rightMargin: 8

                onAddMonoClicked: {
                    root.model.addMonoChannel(name ? name : "In " + (inputs.count + 1))
                    name = ""
                    channelAddStrip.slideToEnd()
                }

                onAddStereoClicked: {
                    root.model.addStereoChannel(name ? name : "In " + (inputs.count + 1))
                    name = ""
                    channelAddStrip.slideToEnd()
                }

                onAddAuxClicked: {
                    root.model.addAuxChannel(name ? name : "Aux " + (inputs.count + 1))
                    name = ""
                    channelAddStrip.slideToEnd()
                }
            }

            function slideToEnd() {
                // Behavior will animate if value actually changes
                inputs.contentX = Math.max(0, inputs.contentWidth + channelAddStrip.width - inputs.width);
            }
        }

        Behavior on contentX {
            NumberAnimation { duration: 400; easing.type: Easing.InOutQuad }
        }
    }

    Rectangle {
        width: 4

        anchors.right: mainChannelStrip.left
        anchors.rightMargin: 8
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        z: 100

        gradient: Gradient {

            orientation: Gradient.Horizontal

            GradientStop {
                position: 0
                color: "#00000000"
            }

            GradientStop {
                position: 1
                color: "#ff000000"
            }
        }
    }

    ChannelStrip {
        id: mainChannelStrip

        anchors.right: mixerToolbar.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 8

        model: root.model.mainChannel

        deletable: false
    }

    ViewToolBar {
        id: mixerToolbar

        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        ViewToolBarButton {
            iconSource: "qrc:///images/icons/pencil.svg"
            checked: MixerViewSettings.mode === MixerViewSettings.edit

            onClicked: MixerViewSettings.toggleMode(MixerViewSettings.edit)
        }

        ViewToolBarButton {
            iconSource: "qrc:///images/icons/aux.svg"
            checked: MixerViewSettings.mode === MixerViewSettings.auxSend

            onClicked: MixerViewSettings.toggleMode(MixerViewSettings.auxSend)
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
