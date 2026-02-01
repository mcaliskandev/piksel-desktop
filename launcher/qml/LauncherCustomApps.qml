import QtQuick
import QtQuick.Layouts

GridLayout {
    id: root

    Layout.fillWidth: true
    Layout.preferredWidth: parent ? parent.width * 0.8 : 0

    columns: 2
}
