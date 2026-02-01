import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

	Rectangle {
	    id: root
	    color: "#f2f2f2"
	    anchors.fill: parent
	    property int panelButtonSize: 32
	    property int panelIconSize: 22
	    function openDockContextMenu(buttonItem, modelData) {
	        if (!buttonItem || !modelData || !panel)
	            return

	        const panelTopLeft = panel.mapToGlobalPoint ? panel.mapToGlobalPoint(0, 0) : Qt.point(0, 0)
	        const localTopLeft = buttonItem.mapToItem(null, 0, 0)
	        const anchorTopLeft = panel.mapToGlobalPoint
	            ? panel.mapToGlobalPoint(localTopLeft.x, localTopLeft.y)
	            : Qt.point(localTopLeft.x, localTopLeft.y)
	        panel.onTriggerDockContextMenu(anchorTopLeft.x, panelTopLeft.y, modelData.appId ?? "")
	    }

    Item {
        id: content
        anchors.fill: parent
        anchors.leftMargin: 6
        anchors.rightMargin: 6
        anchors.topMargin: 4
        anchors.bottomMargin: 4

            RowLayout {
                id: leftCluster
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                spacing: 4

	            ToolButton {
	                id: btnPinnedApps
	                icon.source: "qrc:/resources/icons/pinned.png"
	                icon.width: root.panelIconSize
	                icon.height: root.panelIconSize
	                flat: true
	                Layout.preferredWidth: root.panelButtonSize
	                Layout.preferredHeight: root.panelButtonSize
	                onClicked: {
	                    const panelTopLeft = panel && panel.mapToGlobalPoint ? panel.mapToGlobalPoint(0, 0) : Qt.point(0, 0)
	                    const localTopLeft = btnPinnedApps.mapToItem(null, 0, 0)
	                    const anchorTopLeft = panel && panel.mapToGlobalPoint
	                        ? panel.mapToGlobalPoint(localTopLeft.x, localTopLeft.y)
	                        : Qt.point(localTopLeft.x, localTopLeft.y)
	                    panel.onTriggerPinnedApps(anchorTopLeft.x, panelTopLeft.y)
	                }
	            }

            RowLayout {
                id: openAppsRow
                spacing: 4
                Layout.leftMargin: 4

                Repeater {
                    model: dockApps
                        ? dockApps.apps
                        : (panelRunningApps ? panelRunningApps.apps : [])
                    delegate: ToolButton {
                        required property var modelData
                        text: modelData.text ?? ""
                        readonly property string rawIconSource: modelData.iconSource ?? ""
                        readonly property string rawIconName: modelData.iconName ?? ""
                        readonly property bool iconSourceLooksLikePath: rawIconSource.includes(":/")
                            || rawIconSource.startsWith("/")
                            || rawIconSource.startsWith("file:")
                            || rawIconSource.startsWith("qrc:")
                            || rawIconSource.startsWith("http:")
                            || rawIconSource.startsWith("https:")
                        icon.source: iconSourceLooksLikePath ? rawIconSource : ""
                        icon.name: iconSourceLooksLikePath ? rawIconName : (rawIconName !== "" ? rawIconName : rawIconSource)
                        icon.width: root.panelIconSize
                        icon.height: root.panelIconSize
                        flat: true
                        Layout.preferredWidth: root.panelButtonSize
                        Layout.preferredHeight: root.panelButtonSize
                        onClicked: {
                            if (dockApps && modelData.appId)
                                dockApps.activateApp(modelData.appId)
                            else if (panelRunningApps && modelData.localWindowPtr)
                                panelRunningApps.activateLocal(modelData.localWindowPtr)
                            else if (panelRunningApps && modelData.windowId)
                                panelRunningApps.activate(modelData.windowId)
                        }

	                        MouseArea {
	                            anchors.fill: parent
	                            acceptedButtons: Qt.RightButton
	                            cursorShape: Qt.PointingHandCursor
	                            onClicked: root.openDockContextMenu(parent, modelData)
	                        }
	                    }
	                }
	            }
	        }

        ToolButton {
            id: btnLauncher
            anchors.centerIn: parent
            icon.source: "qrc:/resources/icons/launcher.png"
            icon.width: 28
            icon.height: 28
            flat: true
            width: 40
            height: 40
            onClicked: panel.onTriggerLauncher()
        }

        RowLayout {
            id: rightCluster
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            spacing: 6

            ToolButton {
                id: btnSettings
                icon.source: "qrc:/resources/icons/settings.png"
                icon.width: root.panelIconSize
                icon.height: root.panelIconSize
                flat: true
                Layout.preferredWidth: root.panelButtonSize
                Layout.preferredHeight: root.panelButtonSize
                onClicked: panel.onTriggerSettings()
            }

            ToolButton {
                id: btnInternet
                icon.source: "qrc:/resources/icons/network.png"
                icon.width: root.panelIconSize
                icon.height: root.panelIconSize
                flat: true
                Layout.preferredWidth: root.panelButtonSize
                Layout.preferredHeight: root.panelButtonSize
                onClicked: {
                    const panelTopLeft = panel && panel.mapToGlobalPoint ? panel.mapToGlobalPoint(0, 0) : Qt.point(0, 0)
                    const localTopCenter = btnInternet.mapToItem(null, btnInternet.width / 2, 0)
                    const anchorTopCenter = panel && panel.mapToGlobalPoint
                        ? panel.mapToGlobalPoint(localTopCenter.x, localTopCenter.y)
                        : Qt.point(localTopCenter.x, localTopCenter.y)
                    panel.onTriggerNetwork(anchorTopCenter.x, panelTopLeft.y)
                }
            }

            ToolButton {
                id: btnSound
                icon.source: "qrc:/resources/icons/volume.png"
                icon.width: root.panelIconSize
                icon.height: root.panelIconSize
                flat: true
                Layout.preferredWidth: root.panelButtonSize
                Layout.preferredHeight: root.panelButtonSize
                onClicked: {
                    const panelTopLeft = panel && panel.mapToGlobalPoint ? panel.mapToGlobalPoint(0, 0) : Qt.point(0, 0)
                    const localTopCenter = btnSound.mapToItem(null, btnSound.width / 2, 0)
                    const anchorTopCenter = panel && panel.mapToGlobalPoint
                        ? panel.mapToGlobalPoint(localTopCenter.x, localTopCenter.y)
                        : Qt.point(localTopCenter.x, localTopCenter.y)
                    panel.onTriggerVolume(anchorTopCenter.x, panelTopLeft.y)
                }
            }

            Rectangle {
                id: batteryBox
                color: root.color
                radius: 4
                Layout.preferredHeight: parent.width
                Layout.preferredWidth: batteryRow.implicitWidth + 16

                RowLayout {
                    id: batteryRow
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    spacing: 6

                    Image {
                        source: "qrc:/resources/icons/battery.png"
                        fillMode: Image.PreserveAspectFit
                        sourceSize.width: 20
                        sourceSize.height: 20
                        Layout.preferredWidth: 20
                        Layout.preferredHeight: 20
                    }

                    Text {
                        text: (panelBattery && panelBattery.available) ? (panelBattery.percentage + "%") : "--"
                        color: "black"
                        font.pixelSize: 13
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillWidth: true
                    }
                }
            }

            Rectangle {
                id: timeBox
                color: "white"
                radius: 4
                border.color: "#d0d0d0"
                border.width: 1
                Layout.preferredHeight: parent.width
                Layout.preferredWidth: 100

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    spacing: 6

                    ColumnLayout {
                        spacing: 0
                        Layout.alignment: Qt.AlignVCenter

                        Text {
                            text: (panelClock ? panelClock.month : "--")
                            color: "black"
                            font.pixelSize: 15
                            font.bold: true
                        }

                        Text {
                            text: (panelClock ? panelClock.day : "--")
                            color: "black"
                            font.pixelSize: 15
                            font.bold: true
                        }
                    }

                    Text {
                        text: (panelClock ? panelClock.hourMin : "--:--")
                        color: "black"
                        font.pixelSize: 14
                        font.bold: true
                        verticalAlignment: Text.AlignVCenter
                        Layout.alignment: Qt.AlignVCenter
                        Layout.leftMargin: 4
                        Layout.fillWidth: true
                    }
                }              

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        const panelTopLeft = panel && panel.mapToGlobalPoint ? panel.mapToGlobalPoint(0, 0) : Qt.point(0, 0)
                        console.log("[PikselPanel] timeBox clicked",
                                    "timeBox.size=", timeBox.width + "x" + timeBox.height,
                                    "timeBox.localTopLeft=", timeBox.mapToItem(null, 0, 0).x + "," + timeBox.mapToItem(null, 0, 0).y,
                                    "timeBox.localTopRight=", timeBox.mapToItem(null, timeBox.width, 0).x + "," + timeBox.mapToItem(null, timeBox.width, 0).y,
                                    "panelTopLeft=", panelTopLeft.x + "," + panelTopLeft.y,
                                    "panel.size=", panel.width + "x" + panel.height)
                        const localTopRight = timeBox.mapToItem(null, timeBox.width, 0)
                        const anchorTopRight = panel && panel.mapToGlobalPoint
                            ? panel.mapToGlobalPoint(localTopRight.x, localTopRight.y)
                            : Qt.point(localTopRight.x, localTopRight.y)
                        panel.onTriggerCalendar(anchorTopRight.x, panelTopLeft.y)
                    }
                }
            }
        }
    }

}
