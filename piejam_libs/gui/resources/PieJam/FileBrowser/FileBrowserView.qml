// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2026 PieJam Fork Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15

import PieJam.Controls 1.0

Item {
    id: root

    property var model: null

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        // Header with storage info and controls
        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            Label {
                text: qsTr("Recordings")
                font.pixelSize: 24
                font.bold: true
            }

            Item { Layout.fillWidth: true }

            // Storage indicator
            ColumnLayout {
                spacing: 2

                Label {
                    text: root.model ? root.model.storageInfo : ""
                    font.pixelSize: 12
                    opacity: 0.8
                }

                ProgressBar {
                    Layout.preferredWidth: 150
                    from: 0
                    to: 100
                    value: root.model ? root.model.usedPercent : 0

                    background: Rectangle {
                        implicitWidth: 150
                        implicitHeight: 6
                        color: Material.background
                        radius: 3
                    }

                    contentItem: Item {
                        Rectangle {
                            width: parent.width * (root.model ? root.model.usedPercent / 100 : 0)
                            height: parent.height
                            radius: 3
                            color: {
                                var pct = root.model ? root.model.usedPercent : 0
                                if (pct > 90) return Material.color(Material.Red)
                                if (pct > 75) return Material.color(Material.Orange)
                                return Material.color(Material.Green)
                            }
                        }
                    }
                }
            }

            Button {
                text: qsTr("Refresh")
                flat: true
                enabled: root.model && !root.model.isScanning
                onClicked: {
                    if (root.model) {
                        root.model.rescan()
                    }
                }

                BusyIndicator {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.rightMargin: 8
                    width: 20
                    height: 20
                    running: root.model && root.model.isScanning
                    visible: running
                }
            }
        }

        // Filter bar
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                text: qsTr("Filter:")
                opacity: 0.7
            }

            ComboBox {
                id: tagFilter
                Layout.preferredWidth: 150
                model: root.model ? [qsTr("All Tags")].concat(root.model.allTags) : [qsTr("All Tags")]
                onCurrentIndexChanged: {
                    if (root.model) {
                        if (currentIndex === 0) {
                            root.model.clearFilters()
                        } else {
                            root.model.filterByTag(currentText)
                        }
                    }
                }
            }

            ComboBox {
                id: statusFilter
                Layout.preferredWidth: 150
                model: [qsTr("All Status"), qsTr("New"), qsTr("Archived"), qsTr("Exported"), qsTr("Needs Processing")]
                onCurrentIndexChanged: {
                    if (root.model) {
                        if (currentIndex === 0) {
                            root.model.clearFilters()
                        } else {
                            root.model.filterByStatus(currentIndex - 1)
                        }
                    }
                }
            }

            ComboBox {
                id: ratingFilter
                Layout.preferredWidth: 120
                model: [qsTr("Any Rating"), qsTr("1+ Stars"), qsTr("2+ Stars"), qsTr("3+ Stars"), qsTr("4+ Stars"), qsTr("5 Stars")]
                onCurrentIndexChanged: {
                    if (root.model) {
                        root.model.filterByMinRating(currentIndex)
                    }
                }
            }

            Item { Layout.fillWidth: true }

            Label {
                text: root.model ? qsTr("%1 recordings").arg(root.model.recordingCount) : ""
                opacity: 0.7
            }
        }

        // Main content - recordings list and detail panel
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8

            // Recordings list
            Frame {
                Layout.preferredWidth: parent.width * 0.4
                Layout.fillHeight: true

                ListView {
                    id: recordingsList

                    property int selectedIndex: -1

                    anchors.fill: parent
                    clip: true
                    boundsBehavior: Flickable.StopAtBounds
                    boundsMovement: Flickable.StopAtBounds

                    model: root.model ? root.model.recordings : null

                    delegate: RecordingCard {
                        width: recordingsList.width
                        filename: model.filename
                        durationString: model.durationString
                        formatString: model.formatString
                        sampleRate: model.sampleRate
                        channels: model.channels
                        rating: model.rating
                        tags: model.tags
                        hasClipping: model.hasClipping
                        exportStatus: model.exportStatus
                        isSelected: index === recordingsList.selectedIndex

                        onClicked: {
                            recordingsList.selectedIndex = index
                        }

                        onDeleteRequested: {
                            deleteConfirmDialog.recordingIndex = index
                            deleteConfirmDialog.recordingName = filename
                            deleteConfirmDialog.open()
                        }
                    }

                    ScrollBar.vertical: ScrollBar {}

                    Label {
                        anchors.centerIn: parent
                        text: root.model && root.model.isScanning
                              ? qsTr("Scanning...")
                              : qsTr("No recordings found.\nTap 'Refresh' to scan for files.")
                        horizontalAlignment: Text.AlignHCenter
                        visible: recordingsList.count === 0
                        opacity: 0.5
                    }
                }
            }

            // Detail panel
            Frame {
                Layout.fillWidth: true
                Layout.fillHeight: true

                MetadataPanel {
                    anchors.fill: parent
                    visible: recordingsList.selectedIndex >= 0
                    recordingDetails: {
                        if (root.model && recordingsList.selectedIndex >= 0) {
                            return root.model.getRecordingDetails(recordingsList.selectedIndex)
                        }
                        return null
                    }
                    onTagsChanged: function(tags) {
                        if (root.model && recordingsList.selectedIndex >= 0) {
                            root.model.updateTags(recordingsList.selectedIndex, tags)
                        }
                    }
                    onNotesChanged: function(notes) {
                        if (root.model && recordingsList.selectedIndex >= 0) {
                            root.model.updateNotes(recordingsList.selectedIndex, notes)
                        }
                    }
                    onRatingChanged: function(rating) {
                        if (root.model && recordingsList.selectedIndex >= 0) {
                            root.model.updateRating(recordingsList.selectedIndex, rating)
                        }
                    }
                    onStatusChanged: function(status) {
                        if (root.model && recordingsList.selectedIndex >= 0) {
                            root.model.updateExportStatus(recordingsList.selectedIndex, status)
                        }
                    }
                    onVerifyIntegrityRequested: {
                        if (root.model && recordingsList.selectedIndex >= 0) {
                            root.model.verifyIntegrity(recordingsList.selectedIndex)
                        }
                    }
                }

                Label {
                    anchors.centerIn: parent
                    text: qsTr("Select a recording to view details")
                    opacity: 0.5
                    visible: recordingsList.selectedIndex < 0
                }
            }
        }
    }

    // Delete confirmation dialog
    Dialog {
        id: deleteConfirmDialog

        property int recordingIndex: -1
        property string recordingName: ""

        parent: Overlay.overlay
        anchors.centerIn: parent
        title: qsTr("Delete Recording")
        standardButtons: Dialog.Yes | Dialog.No
        modal: true

        Label {
            text: qsTr("Are you sure you want to delete '%1'?\n\nThis action cannot be undone.").arg(deleteConfirmDialog.recordingName)
            wrapMode: Text.WordWrap
        }

        onAccepted: {
            if (root.model && recordingIndex >= 0) {
                root.model.deleteRecording(recordingIndex)
                recordingsList.selectedIndex = -1
            }
        }
    }

    // Integrity check result dialog
    Dialog {
        id: integrityDialog

        property bool passed: false
        property string message: ""

        parent: Overlay.overlay
        anchors.centerIn: parent
        title: qsTr("Integrity Check")
        standardButtons: Dialog.Ok

        RowLayout {
            spacing: 16

            Label {
                text: integrityDialog.passed ? "\u2714" : "\u2718"
                font.pixelSize: 32
                color: integrityDialog.passed
                       ? Material.color(Material.Green)
                       : Material.color(Material.Red)
            }

            Label {
                text: integrityDialog.message
                wrapMode: Text.WordWrap
                Layout.preferredWidth: 300
            }
        }
    }

    // Error dialog
    Dialog {
        id: errorDialog

        property alias text: errorLabel.text

        parent: Overlay.overlay
        anchors.centerIn: parent
        title: qsTr("Error")
        standardButtons: Dialog.Ok

        Label {
            id: errorLabel
            wrapMode: Text.WordWrap
        }
    }

    // Handle signals from model
    Connections {
        target: root.model

        function onIntegrityCheckComplete(index, passed, message) {
            integrityDialog.passed = passed
            integrityDialog.message = message
            integrityDialog.open()
        }

        function onOperationError(message) {
            errorDialog.text = message
            errorDialog.open()
        }

        function onNewRecordingDetected(filename) {
            // Could show a toast notification
            console.log("New recording detected:", filename)
        }

        function onScanComplete(fileCount) {
            console.log("Scan complete:", fileCount, "files found")
        }
    }
}
