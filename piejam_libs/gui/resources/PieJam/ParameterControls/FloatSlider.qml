// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15

import PieJam.Controls 1.0 as PJControls
import PieJam.Models 1.0

import ".."

SubscribableItem {
    id: root

    property int orientation: Qt.Horizontal

    QtObject {
        id: private_

        readonly property var paramModel: root.model && root.model.type === Parameter.Type.Float ? root.model : null
        readonly property real value: private_.paramModel ? private_.paramModel.normalizedValue : 0

        function changeValue(newValue) {
            if (private_.paramModel) {
                private_.paramModel.changeNormalizedValue(newValue)
                Info.showParameterValue(private_.paramModel)
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent

        spacing: 0

        Slider {
            value: private_.value

            orientation: root.orientation

            Layout.fillWidth: true
            Layout.fillHeight: true

            onMoved: private_.changeValue(value)
        }
    }

    PJControls.MidiAssignArea {
        anchors.fill: parent

        model: private_.paramModel ? private_.paramModel.midi : null
    }
}
