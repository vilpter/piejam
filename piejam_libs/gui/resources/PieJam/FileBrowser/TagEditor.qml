// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2026 PieJam Fork Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15

Item {
    id: root

    property var tags: []

    signal tagsModified(var newTags)

    implicitHeight: contentLayout.implicitHeight

    ColumnLayout {
        id: contentLayout
        anchors.fill: parent
        spacing: 8

        // Existing tags display
        Flow {
            Layout.fillWidth: true
            spacing: 6
            visible: root.tags && root.tags.length > 0

            Repeater {
                model: root.tags

                Rectangle {
                    width: tagRow.implicitWidth + 8
                    height: tagRow.implicitHeight + 8
                    radius: 4
                    color: Material.accent
                    opacity: 0.8

                    Row {
                        id: tagRow
                        anchors.centerIn: parent
                        spacing: 4

                        Label {
                            text: modelData
                            font.pixelSize: 12
                            color: Material.foreground
                        }

                        ToolButton {
                            implicitWidth: 16
                            implicitHeight: 16
                            text: "\u2715"
                            font.pixelSize: 10
                            padding: 0

                            onClicked: {
                                var newTags = root.tags.filter(function(t) { return t !== modelData })
                                root.tagsModified(newTags)
                            }
                        }
                    }
                }
            }
        }

        // Add tag row
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            TextField {
                id: newTagField
                Layout.fillWidth: true
                Layout.preferredHeight: 36
                placeholderText: qsTr("Add tag...")
                font.pixelSize: 12

                onAccepted: addTag()

                Keys.onEscapePressed: {
                    text = ""
                    focus = false
                }
            }

            Button {
                text: qsTr("Add")
                implicitHeight: 36
                enabled: newTagField.text.trim().length > 0
                onClicked: addTag()
            }
        }

        // Common tags suggestions (shown when input is focused and empty)
        Flow {
            Layout.fillWidth: true
            spacing: 4
            visible: newTagField.activeFocus && newTagField.text.length === 0

            Label {
                text: qsTr("Suggestions:")
                font.pixelSize: 11
                opacity: 0.6
            }

            Repeater {
                model: getTagSuggestions()

                Button {
                    flat: true
                    implicitHeight: 24
                    text: modelData
                    font.pixelSize: 11
                    padding: 4

                    onClicked: {
                        newTagField.text = modelData
                        addTag()
                    }
                }
            }
        }
    }

    function addTag() {
        var tag = newTagField.text.trim()
        if (tag.length > 0 && !hasTag(tag)) {
            var newTags = root.tags ? root.tags.slice() : []
            newTags.push(tag)
            root.tagsModified(newTags)
            newTagField.text = ""
        }
    }

    function hasTag(tag) {
        if (!root.tags) return false
        return root.tags.some(function(t) {
            return t.toLowerCase() === tag.toLowerCase()
        })
    }

    function getTagSuggestions() {
        var commonTags = ["live", "rehearsal", "final", "mix", "demo", "scratch", "master", "reference", "vocals", "instrumental"]
        return commonTags.filter(function(t) {
            return !hasTag(t)
        }).slice(0, 6)
    }
}
