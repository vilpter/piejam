// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2026 PieJam Fork Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15

Item {
    id: root

    property var recordingDetails: null

    signal tagsChanged(var tags)
    signal notesChanged(string notes)
    signal ratingChanged(int rating)
    signal statusChanged(int status)
    signal verifyIntegrityRequested()

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        clip: true

        ColumnLayout {
            width: parent.width
            spacing: 16

            // File info header
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Label {
                    text: root.recordingDetails ? root.recordingDetails.filename : ""
                    font.pixelSize: 20
                    font.bold: true
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                Label {
                    text: root.recordingDetails ? root.recordingDetails.filePath : ""
                    font.pixelSize: 11
                    opacity: 0.6
                    elide: Text.ElideMiddle
                    Layout.fillWidth: true
                }
            }

            // Audio info section
            GroupBox {
                title: qsTr("Audio Information")
                Layout.fillWidth: true

                GridLayout {
                    anchors.fill: parent
                    columns: 4
                    columnSpacing: 24
                    rowSpacing: 8

                    Label { text: qsTr("Duration:"); font.bold: true }
                    Label { text: root.recordingDetails ? root.recordingDetails.durationString : "" }

                    Label { text: qsTr("Format:"); font.bold: true }
                    Label { text: root.recordingDetails ? root.recordingDetails.formatString : "" }

                    Label { text: qsTr("Sample Rate:"); font.bold: true }
                    Label { text: root.recordingDetails ? (root.recordingDetails.sampleRate + " Hz") : "" }

                    Label { text: qsTr("Bit Depth:"); font.bold: true }
                    Label { text: root.recordingDetails ? (root.recordingDetails.bitDepth + " bit") : "" }

                    Label { text: qsTr("Channels:"); font.bold: true }
                    Label {
                        text: {
                            if (!root.recordingDetails) return ""
                            var ch = root.recordingDetails.channels
                            if (ch === 1) return qsTr("Mono")
                            if (ch === 2) return qsTr("Stereo")
                            return ch + " " + qsTr("channels")
                        }
                    }

                    Label { text: qsTr("File Size:"); font.bold: true }
                    Label { text: root.recordingDetails ? root.recordingDetails.fileSizeString : "" }
                }
            }

            // Levels info section
            GroupBox {
                title: qsTr("Level Analysis")
                Layout.fillWidth: true

                GridLayout {
                    anchors.fill: parent
                    columns: 4
                    columnSpacing: 24
                    rowSpacing: 8

                    Label { text: qsTr("Peak Level:"); font.bold: true }
                    Label {
                        text: root.recordingDetails ? (root.recordingDetails.peakLevelDb.toFixed(1) + " dB") : ""
                        color: root.recordingDetails && root.recordingDetails.peakLevelDb > -1
                               ? Material.color(Material.Red) : Material.foreground
                    }

                    Label { text: qsTr("RMS Level:"); font.bold: true }
                    Label { text: root.recordingDetails ? (root.recordingDetails.rmsLevelDb.toFixed(1) + " dB") : "" }

                    Label { text: qsTr("Clipping:"); font.bold: true }
                    Label {
                        text: root.recordingDetails
                              ? (root.recordingDetails.hasClipping ? qsTr("Detected") : qsTr("None"))
                              : ""
                        color: root.recordingDetails && root.recordingDetails.hasClipping
                               ? Material.color(Material.Red) : Material.color(Material.Green)
                    }

                    Item {}
                    Item {}
                }
            }

            // User metadata section
            GroupBox {
                title: qsTr("Metadata")
                Layout.fillWidth: true

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 12

                    // Rating
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("Rating:")
                            font.bold: true
                            Layout.preferredWidth: 80
                        }

                        Row {
                            spacing: 4
                            Repeater {
                                model: 5
                                Button {
                                    flat: true
                                    implicitWidth: 32
                                    implicitHeight: 32
                                    text: index < (root.recordingDetails ? root.recordingDetails.rating : 0) ? "\u2605" : "\u2606"
                                    font.pixelSize: 20
                                    Material.foreground: index < (root.recordingDetails ? root.recordingDetails.rating : 0)
                                                         ? Material.color(Material.Amber) : Material.foreground

                                    onClicked: {
                                        var newRating = index + 1
                                        // Toggle off if clicking same rating
                                        if (root.recordingDetails && root.recordingDetails.rating === newRating) {
                                            newRating = 0
                                        }
                                        root.ratingChanged(newRating)
                                    }
                                }
                            }
                        }
                    }

                    // Status
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("Status:")
                            font.bold: true
                            Layout.preferredWidth: 80
                        }

                        ComboBox {
                            Layout.preferredWidth: 180
                            model: [qsTr("New Recording"), qsTr("Archived"), qsTr("Exported"), qsTr("Needs Processing")]
                            currentIndex: root.recordingDetails ? root.recordingDetails.exportStatus : 0
                            onCurrentIndexChanged: {
                                if (root.recordingDetails && currentIndex !== root.recordingDetails.exportStatus) {
                                    root.statusChanged(currentIndex)
                                }
                            }
                        }
                    }

                    // Tags
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("Tags:")
                            font.bold: true
                            Layout.preferredWidth: 80
                            Layout.alignment: Qt.AlignTop
                        }

                        TagEditor {
                            Layout.fillWidth: true
                            tags: root.recordingDetails ? root.recordingDetails.tags : []
                            onTagsModified: function(newTags) {
                                root.tagsChanged(newTags)
                            }
                        }
                    }

                    // Notes
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("Notes:")
                            font.bold: true
                            Layout.preferredWidth: 80
                            Layout.alignment: Qt.AlignTop
                        }

                        TextArea {
                            id: notesField
                            Layout.fillWidth: true
                            Layout.preferredHeight: 80
                            text: root.recordingDetails ? root.recordingDetails.notes : ""
                            placeholderText: qsTr("Add notes about this recording...")
                            wrapMode: TextEdit.Wrap

                            onTextChanged: {
                                if (root.recordingDetails && text !== root.recordingDetails.notes) {
                                    notesSaveTimer.restart()
                                }
                            }

                            Timer {
                                id: notesSaveTimer
                                interval: 1000
                                onTriggered: root.notesChanged(notesField.text)
                            }
                        }
                    }

                    // Project info
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        visible: root.recordingDetails && root.recordingDetails.projectName

                        Label {
                            text: qsTr("Project:")
                            font.bold: true
                            Layout.preferredWidth: 80
                        }

                        Label {
                            text: root.recordingDetails ? root.recordingDetails.projectName : ""
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            // Actions section
            GroupBox {
                title: qsTr("Actions")
                Layout.fillWidth: true

                RowLayout {
                    anchors.fill: parent
                    spacing: 8

                    Button {
                        text: qsTr("Verify Integrity")
                        onClicked: root.verifyIntegrityRequested()
                    }

                    Item { Layout.fillWidth: true }
                }
            }

            // Spacer
            Item { Layout.fillHeight: true }
        }
    }
}
