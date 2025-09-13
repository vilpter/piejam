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
    property int currentIndex: -1
    property alias currentText: comboBox.currentText
    property string displayText: ""
    property int elideMode: Qt.ElideNone
    property var model: null
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

        model: root.model
        currentIndex: root.currentIndex

        displayText: root.displayText

        contentItem: Text {
            anchors.fill: comboBox

            text: root.displayText
            font: comboBox.font
            color: root.enabled ? Material.primaryTextColor : Material.secondaryTextColor
            verticalAlignment: Text.AlignVCenter
            elide: root.elideMode
            padding: 6
        }

        indicator: Canvas {
            anchors.right: comboBox.right
            anchors.bottom: comboBox.bottom
            anchors.bottomMargin: 6

            width: 10
            height: 10

            contextType: "2d"

            onPaint: {
                context.reset();
                context.moveTo(0, height);
                context.lineTo(width, 0);
                context.lineTo(width, height);
                context.closePath();
                context.fillStyle = Material.primaryTextColor;
                context.fill();
            }
        }

        onActivated: root.activated(index)

        // model change breaks the currentIndex binding, so we need to restore it
        onModelChanged: currentIndex = Qt.binding(function () { return root.currentIndex } )
    }
}
