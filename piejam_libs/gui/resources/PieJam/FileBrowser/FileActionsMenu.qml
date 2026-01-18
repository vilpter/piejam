// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2026 PieJam Fork Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

Menu {
    id: root

    property var recordingDetails: null

    signal deleteRequested()
    signal verifyIntegrityRequested()
    signal setStatusRequested(int status)
    signal addTagRequested()

    MenuItem {
        text: qsTr("Verify Integrity")
        icon.name: "document-properties"
        onTriggered: root.verifyIntegrityRequested()
    }

    MenuSeparator {}

    Menu {
        title: qsTr("Set Status")

        MenuItem {
            text: qsTr("New Recording")
            checkable: true
            checked: root.recordingDetails && root.recordingDetails.exportStatus === 0
            onTriggered: root.setStatusRequested(0)
        }

        MenuItem {
            text: qsTr("Archived")
            checkable: true
            checked: root.recordingDetails && root.recordingDetails.exportStatus === 1
            onTriggered: root.setStatusRequested(1)
        }

        MenuItem {
            text: qsTr("Exported")
            checkable: true
            checked: root.recordingDetails && root.recordingDetails.exportStatus === 2
            onTriggered: root.setStatusRequested(2)
        }

        MenuItem {
            text: qsTr("Needs Processing")
            checkable: true
            checked: root.recordingDetails && root.recordingDetails.exportStatus === 3
            onTriggered: root.setStatusRequested(3)
        }
    }

    MenuItem {
        text: qsTr("Add Tag...")
        onTriggered: root.addTagRequested()
    }

    MenuSeparator {}

    MenuItem {
        text: qsTr("Delete Recording")
        icon.name: "edit-delete"
        Material.foreground: Material.Red
        onTriggered: root.deleteRequested()
    }
}
