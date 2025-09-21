// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15

import PieJam.Controls 1.0
import PieJam.Models 1.0 as PJModels

import ".."

ChannelStripBase {
    id: root

    ColumnLayout {
        anchors.fill: parent
        spacing: 2

        HeaderLabel {
            Layout.fillWidth: true
            Layout.preferredHeight: 24

            text: root.model ? root.model.name : ""
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            spacing: 2

            clip: true
            boundsBehavior: Flickable.StopAtBounds
            boundsMovement: Flickable.StopAtBounds

            model: root.model ? root.model.auxSends : null

            delegate: AuxSend {
                anchors.left: parent ? parent.left : undefined
                anchors.right: parent ? parent.right : undefined

                model: item
            }
        }
    }
}
