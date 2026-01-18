// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2026 PieJam Fork Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15

ItemDelegate {
    id: root

    property string filename: ""
    property string durationString: ""
    property string formatString: ""
    property int sampleRate: 0
    property int channels: 0
    property int rating: 0
    property var tags: []
    property bool hasClipping: false
    property int exportStatus: 0
    property bool isSelected: false

    signal deleteRequested()

    implicitHeight: contentLayout.implicitHeight + 16
    highlighted: isSelected

    background: Rectangle {
        color: root.isSelected
               ? Material.accent
               : (root.hovered ? Qt.rgba(Material.foreground.r, Material.foreground.g, Material.foreground.b, 0.1) : "transparent")
        opacity: root.isSelected ? 0.3 : 1

        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: Material.dividerColor
        }
    }

    contentItem: ColumnLayout {
        id: contentLayout
        spacing: 4

        // Top row: filename and status indicators
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            // Status indicator dot
            Rectangle {
                width: 8
                height: 8
                radius: 4
                color: {
                    switch (root.exportStatus) {
                        case 0: return Material.color(Material.Blue)  // New
                        case 1: return Material.color(Material.Grey)  // Archived
                        case 2: return Material.color(Material.Green) // Exported
                        case 3: return Material.color(Material.Orange) // Needs Processing
                        default: return Material.color(Material.Grey)
                    }
                }
            }

            Label {
                text: root.filename
                font.pixelSize: 14
                font.bold: true
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            // Clipping warning
            Label {
                text: "\u26A0"
                font.pixelSize: 14
                color: Material.color(Material.Red)
                visible: root.hasClipping
                ToolTip.visible: clipWarningArea.containsMouse
                ToolTip.text: qsTr("Audio clipping detected")

                MouseArea {
                    id: clipWarningArea
                    anchors.fill: parent
                    hoverEnabled: true
                }
            }

            // Delete button
            ToolButton {
                implicitWidth: 28
                implicitHeight: 28
                text: "\u2715"
                font.pixelSize: 12
                opacity: root.hovered ? 1 : 0
                onClicked: root.deleteRequested()

                Behavior on opacity { NumberAnimation { duration: 150 } }
            }
        }

        // Second row: duration and format info
        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            Label {
                text: root.durationString
                font.pixelSize: 12
                opacity: 0.7
            }

            Label {
                text: root.formatString
                font.pixelSize: 12
                opacity: 0.7
            }

            Label {
                text: root.sampleRate > 0 ? (root.sampleRate / 1000) + " kHz" : ""
                font.pixelSize: 12
                opacity: 0.7
            }

            Label {
                text: root.channels === 1 ? qsTr("Mono") : (root.channels === 2 ? qsTr("Stereo") : (root.channels + qsTr("ch")))
                font.pixelSize: 12
                opacity: 0.7
                visible: root.channels > 0
            }

            Item { Layout.fillWidth: true }

            // Star rating display
            Row {
                spacing: 2
                Repeater {
                    model: 5
                    Label {
                        text: index < root.rating ? "\u2605" : "\u2606"
                        font.pixelSize: 12
                        color: index < root.rating ? Material.color(Material.Amber) : Material.foreground
                        opacity: index < root.rating ? 1 : 0.3
                    }
                }
            }
        }

        // Third row: tags (if any)
        Flow {
            Layout.fillWidth: true
            spacing: 4
            visible: root.tags && root.tags.length > 0

            Repeater {
                model: root.tags

                Rectangle {
                    width: tagLabel.implicitWidth + 12
                    height: tagLabel.implicitHeight + 4
                    radius: height / 2
                    color: Material.accent
                    opacity: 0.3

                    Label {
                        id: tagLabel
                        anchors.centerIn: parent
                        text: modelData
                        font.pixelSize: 10
                    }
                }
            }
        }
    }
}
