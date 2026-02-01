import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property int popupWidth: 320
    property int popupHeight: 360

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
                source: "qrc:/resources/icons/pinned.png"
                fillMode: Image.PreserveAspectFit
                sourceSize.width: 18
                sourceSize.height: 18
                Layout.preferredWidth: 18
                Layout.preferredHeight: 18
            }

            Label {
                text: "Pinned Apps"
                font.pixelSize: 14
                font.bold: true
                Layout.fillWidth: true
                elide: Text.ElideRight
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
                model: dockApps ? dockApps.pinnedApps : []

                delegate: Rectangle {
                    required property var modelData
                    width: list.width
                    height: 44
                    radius: 8
                    color: mouseArea.containsMouse ? "#f0f0f0" : "#f6f6f6"

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 10

                        Item {
                            Layout.preferredWidth: 22
                            Layout.preferredHeight: 22
                            Layout.alignment: Qt.AlignVCenter

                            ToolButton {
                                anchors.fill: parent
                                readonly property string rawIconSource: modelData.iconSource ?? ""
                                readonly property string rawIconName: modelData.iconName ?? ""
                                readonly property bool iconSourceLooksLikePath: rawIconSource.includes(":/")
                                    || rawIconSource.startsWith("/")
                                    || rawIconSource.startsWith("file:")
                                    || rawIconSource.startsWith("qrc:")
                                    || rawIconSource.startsWith("http:")
                                    || rawIconSource.startsWith("https:")
                                flat: true
                                enabled: false
                                focusPolicy: Qt.NoFocus
                                padding: 0

                                icon.width: 22
                                icon.height: 22
                                icon.source: (rawIconSource !== "" && iconSourceLooksLikePath)
                                    ? rawIconSource
                                    : (rawIconName === "" && rawIconSource === "" ? "qrc:/resources/icons/launcher.png" : "")
                                icon.name: (rawIconSource !== "" && iconSourceLooksLikePath)
                                    ? ""
                                    : (rawIconName !== "" ? rawIconName : rawIconSource)

                                background: Item {}
                            }
                        }

                        Label {
                            text: modelData.text ?? ""
                            font.pixelSize: 13
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (dockApps && modelData.appId)
                                dockApps.activatePinned(modelData.appId)
                        }
                    }
                }

                Label {
                    anchors.centerIn: parent
                    visible: list.count === 0
                    text: "No pinned apps"
                    color: "#666666"
                    font.pixelSize: 13
                }
            }
        }
    }
}
