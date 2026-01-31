import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    property string appId: ""

    color: "#2a2a2a"
    radius: 8
    border.color: "#444"
    border.width: 1

    implicitWidth: content.implicitWidth + 12
    implicitHeight: content.implicitHeight + 12

    focus: true
    Keys.onEscapePressed: if (panel) panel.hideDockContextMenu()

    Column {
        id: content
        anchors.centerIn: parent
        spacing: 6

        Button {
            text: "Pin"
            width: 120
            enabled: dockApps && root.appId !== "" && !dockApps.isPinned(root.appId)
            onClicked: {
                if (dockApps && root.appId !== "")
                    dockApps.pinApp(root.appId)
                if (panel)
                    panel.hideDockContextMenu()
            }
        }

        Button {
            text: "Close"
            width: 120
            enabled: dockApps && root.appId !== ""
            onClicked: {
                if (dockApps && root.appId !== "")
                    dockApps.closeApp(root.appId)
                if (panel)
                    panel.hideDockContextMenu()
            }
        }
    }
}

