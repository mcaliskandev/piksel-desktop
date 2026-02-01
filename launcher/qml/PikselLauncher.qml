import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    anchors.fill: parent
    focus: true

    Keys.onEscapePressed: launcher.requestHide()

    property string currentTime: Qt.formatTime(new Date(), "hh:mm")

    Timer {
        interval: 500
        running: true
        repeat: true
        onTriggered: root.currentTime = Qt.formatTime(new Date(), "hh:mm")
    }

    Rectangle {
        anchors.fill: parent
        color: "#333"
        radius: 8

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 24

            // Top header: back/exit button (left) + panel actions (right)
            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 22
                Layout.minimumHeight: 22
                spacing: 12

                RowLayout {
                    Layout.preferredWidth: 22
                    Layout.fillHeight: true
                    spacing: 10

                    ToolButton {
                        icon.source: "qrc:/resources/icons/exitPanel.png"
                        icon.width: 22
                        icon.height: 22
                        flat: true
                        Layout.preferredWidth: 40
                        Layout.preferredHeight: 40
                        ToolTip.visible: hovered
                        ToolTip.text: "Back"
                        onClicked: launcher.requestHide()
                    }

                    Rectangle {
                        id: dockItems
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: '#e7e7e7'
                        radius: 6
                        border.color: "#444"
                        border.width: 1

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10
                            spacing: 8

                            Text {
                                text: root.currentTime
                                color: "#111"
                                font.pixelSize: 22
                                font.bold: true
                                Layout.alignment: Qt.AlignVCenter
                            }

                            Item { Layout.fillWidth: true }

                            ToolButton {
                                icon.source: "qrc:/resources/icons/lock.png"
                                icon.width: 22
                                icon.height: 22
                                flat: true
                                Layout.preferredWidth: 40
                                Layout.preferredHeight: 40
                                ToolTip.visible: hovered
                                ToolTip.text: "Lock"
                                onClicked: {
                                    launcher.lockSystem()
                                    launcher.requestHide()
                                }
                            }

                            ToolButton {
                                icon.source: "qrc:/resources/icons/suspend.png"
                                icon.width: 22
                                icon.height: 22
                                flat: true
                                Layout.preferredWidth: 40
                                Layout.preferredHeight: 40
                                ToolTip.visible: hovered
                                ToolTip.text: "Suspend"
                                onClicked: {
                                    launcher.suspendSystem()
                                    launcher.requestHide()
                                }
                            }

                            ToolButton {
                                icon.source: "qrc:/resources/icons/power.png"
                                icon.width: 22
                                icon.height: 22
                                flat: true
                                Layout.preferredWidth: 40
                                Layout.preferredHeight: 40
                                ToolTip.visible: hovered
                                ToolTip.text: "Power Off"
                                onClicked: {
                                    launcher.powerOffSystem()
                                    launcher.requestHide()
                                }
                            }
                        }
                    }
                }
            }

            RowLayout {
                width: parent.width

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.preferredWidth: parent.width * 0.2

                    // Title
                    Text {
                        Layout.fillWidth: true
                        Layout.topMargin: 22
                        text: "Applications List"
                        color: "white"
                        font.pixelSize: 18
                        font.bold: true
                    }

                    // Scroll list
                    ScrollView {
                        id: appsScroll
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true

                        ScrollBar.vertical.policy: ScrollBar.AlwaysOn

                        ListView {
                            id: appsList
                            model: launcherApps.apps
                            clip: true
                            spacing: 8

                            delegate: ItemDelegate {
                                required property string appId
                                required property string appName
                                required property string appIconSource
                                required property string appIconName
                                required property string appAction
                                required property string appExec

                                width: ListView.view.width
                                height: 54

                                background: Rectangle {
                                    color: parent.hovered ? "#3d3d3d" : "#2a2a2a"
                                    radius: 8
                                    border.color: "#444"
                                    border.width: 1
                                }

                                contentItem: RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 12
                                    anchors.rightMargin: 12
                                    spacing: 12

                                    Item {
                                        Layout.preferredWidth: 26
                                        Layout.preferredHeight: 26
                                        Layout.alignment: Qt.AlignVCenter

                                        readonly property string computedSource: appIconSource !== "" ? appIconSource
                                            : (appIconName !== "" ? ("image://theme/" + appIconName) : "")
                                        readonly property string fallbackSource: "qrc:/resources/icons/launcher.png"
                                        property bool useFallback: false

                                        Image {
                                            id: iconImage
                                            anchors.fill: parent
                                            source: parent.useFallback || parent.computedSource === ""
                                                ? parent.fallbackSource
                                                : parent.computedSource
                                            sourceSize.width: 26
                                            sourceSize.height: 26
                                            fillMode: Image.PreserveAspectFit
                                            smooth: true
                                            asynchronous: true
                                            cache: true

                                            onStatusChanged: {
                                                if (status === Image.Error)
                                                    parent.useFallback = true
                                            }
                                            onSourceChanged: parent.useFallback = false
                                        }
                                    }

                                    Text {
                                        Layout.fillWidth: true
                                        text: appName
                                        color: "white"
                                        font.pixelSize: 16
                                        elide: Text.ElideRight
                                    }
                                }

                                onClicked: {
                                    launcher.launchEntry(appAction, appExec, appId, appName, appIconSource, appIconName)
                                    launcher.requestHide()
                                }
                            }
                        }
                    }
                }

                LauncherCustomApps {
                    id: launcherCustomApps
                }
            }
        }
    }
}
