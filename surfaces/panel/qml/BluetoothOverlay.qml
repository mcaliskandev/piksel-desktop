import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property int popupWidth: 320
    property int popupHeight: 280

    width: popupWidth
    height: popupHeight
    implicitWidth: popupWidth
    implicitHeight: popupHeight

    Rectangle {
        anchors.fill: parent
        radius: 10
        color: "white"
        border.color: "#d0d0d0"
        border.width: 1
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Image {
                source: "qrc:/resources/icons/connection.png"
                fillMode: Image.PreserveAspectFit
                sourceSize.width: 18
                sourceSize.height: 18
                Layout.preferredWidth: 18
                Layout.preferredHeight: 18
            }

            Label {
                text: "Bluetooth"
                font.pixelSize: 14
                font.bold: true
                Layout.fillWidth: true
                elide: Text.ElideRight
            }

            Label {
                text: panelBluetooth && panelBluetooth.powered ? "On" : "Off"
                font.pixelSize: 12
                color: panelBluetooth && panelBluetooth.powered ? "#1d7a3a" : "#666666"
            }

            ToolButton {
                text: "⟳"
                onClicked: panelBluetooth && panelBluetooth.refresh ? panelBluetooth.refresh() : undefined
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                id: list
                anchors.fill: parent
                clip: true
                spacing: 6
                model: panelBluetooth ? panelBluetooth.devices : []

                delegate: Rectangle {
                    required property var modelData
                    width: list.width
                    height: 40
                    radius: 8
                    color: "#f6f6f6"

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 10

                        Label {
                            text: modelData.name && modelData.name !== "" ? modelData.name : (modelData.address ?? "Unknown device")
                            font.pixelSize: 13
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }

                        Label {
                            text: modelData.connected ? "Connected" : "Known"
                            font.pixelSize: 12
                            color: modelData.connected ? "#1d7a3a" : "#666666"
                        }
                    }
                }

                Label {
                    anchors.centerIn: parent
                    visible: list.count === 0
                    text: panelBluetooth && !panelBluetooth.powered ? "Bluetooth is off" : "No devices"
                    color: "#666666"
                    font.pixelSize: 13
                }
            }
        }
    }
}

