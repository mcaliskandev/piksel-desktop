import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property int popupWidth: 320
    property int popupHeight: 300

    width: popupWidth
    height: popupHeight
    implicitWidth: popupWidth
    implicitHeight: popupHeight

    function iconForStrength(strength) {
        if (strength === undefined || strength === null) return "qrc:/resources/icons/wifi_strength_low.png"
        const s = Math.max(0, Math.min(100, Number(strength)))
        if (s >= 75) return "qrc:/resources/icons/wifi_strength_full.png"
        if (s >= 50) return "qrc:/resources/icons/wifi_strength_high.png"
        if (s >= 25) return "qrc:/resources/icons/wifi_strength_medium.png"
        return "qrc:/resources/icons/wifi_strength_low.png"
    }

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
                source: "qrc:/resources/icons/network.png"
                fillMode: Image.PreserveAspectFit
                sourceSize.width: 18
                sourceSize.height: 18
                Layout.preferredWidth: 18
                Layout.preferredHeight: 18
            }

            Label {
                text: "Networks"
                font.pixelSize: 14
                font.bold: true
                Layout.fillWidth: true
                elide: Text.ElideRight
            }

            ToolButton {
                text: "⟳"
                onClicked: panelNetwork && panelNetwork.refresh ? panelNetwork.refresh() : undefined
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
                model: panelNetwork ? panelNetwork.networks : []

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
                            text: modelData.name ?? ""
                            font.pixelSize: 13
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }

                        Image {
                            source: root.iconForStrength(modelData.strength)
                            fillMode: Image.PreserveAspectFit
                            sourceSize.width: 20
                            sourceSize.height: 20
                            Layout.preferredWidth: 20
                            Layout.preferredHeight: 20
                        }
                    }
                }

                Label {
                    anchors.centerIn: parent
                    visible: list.count === 0
                    text: "No networks"
                    color: "#666666"
                    font.pixelSize: 13
                }
            }
        }
    }
}

