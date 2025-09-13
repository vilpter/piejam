// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

// A ComboBox with elided display text.

Item {
    id: root

    property alias count: comboBox.count
    property alias currentIndex: comboBox.currentIndex
    property alias currentText: comboBox.currentText
    property string displayText: ""
    property int elideMode: Qt.ElideNone
    property alias model: comboBox.model
    property alias popup: comboBox.popup
    property alias textRole: comboBox.textRole
    property alias valueRole: comboBox.valueRole

    signal activated(int index)

    implicitWidth: comboBox.implicitWidth
    implicitHeight: comboBox.implicitHeight

    ComboBox {
        id: comboBox

        anchors.fill: parent

        FontMetrics {
            id: fontMetrics

            font: comboBox.font
        }

        displayText: root.elideMode === Qt.ElideNone
                ? root.displayText
                // TODO: how to derive proper contentItem.width
                : fontMetrics.elidedText(root.displayText, root.elideMode, comboBox.contentItem.width - 4)

        onActivated: root.activated(index)
    }
}


