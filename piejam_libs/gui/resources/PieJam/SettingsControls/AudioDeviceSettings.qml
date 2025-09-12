// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import ".."
import "../SettingsControls"

SubscribableItem {
    id: root

    implicitWidth: 752
    implicitHeight: 432

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8

        ComboBoxSetting {
            Layout.fillWidth: true

            model: root.model.soundCards.elements
            currentIndex: root.model.soundCards.focused

            nameLabelText: qsTr("device:")
            unselectedText: qsTr("Select...")

            onOptionSelected: root.model.selectSoundCard(index)
        }

        ComboBoxSetting {
            Layout.fillWidth: true

            model: root.model.sampleRates.elements
            currentIndex: root.model.sampleRates.focused

            nameLabelText: qsTr("sample rate:")
            unselectedText: qsTr("Select sample rate...")

            onOptionSelected: root.model.selectSampleRate(index)
        }

        Frame {
            Layout.fillWidth: true
            Layout.preferredHeight: 96

            ColumnLayout {
                anchors.fill: parent

                RowLayout {
                    Layout.fillWidth: true
                    Layout.maximumHeight: 32

                    Label {
                        Layout.preferredWidth: 128
                        Layout.fillHeight: true

                        verticalAlignment: Text.AlignVCenter
                        textFormat: Text.PlainText
                        font.pixelSize: 18

                        text: qsTr("buffer size:")
                    }

                    Slider {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        from: 0
                        to: root.model.periodSizes.elements.length
                        stepSize: 1
                        value: root.model.periodSizes.focused

                        onMoved: root.model.selectPeriodSize(value)
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.maximumHeight: 32

                    Item {
                        Layout.preferredWidth: 128
                        Layout.fillHeight: true
                    }

                    Label {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        verticalAlignment: Text.AlignVCenter
                        font.italic: true
                        textFormat: Text.PlainText
                        font.pixelSize: 18

                        text: (root.model.periodSizes.focused === -1 ? "0" : root.model.periodSizes.elements[root.model.periodSizes.focused]) +
                              qsTr(" Samples / ") +
                              root.model.bufferLatency.toFixed(2) +
                              qsTr(" ms")
                    }
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
