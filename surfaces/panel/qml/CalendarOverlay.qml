import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property date selectedDate: new Date()
    property date viewDate: new Date(selectedDate.getFullYear(), selectedDate.getMonth(), 1)

    signal datePicked(var picked)

    property int popupWidth: 320
    property int popupHeight: 360

    width: popupWidth
    height: popupHeight
    implicitWidth: popupWidth
    implicitHeight: popupHeight

    readonly property int viewYear: viewDate.getFullYear()
    readonly property int viewMonth: viewDate.getMonth() // 0..11
    readonly property int daysThisMonth: new Date(viewYear, viewMonth + 1, 0).getDate()
    // JS Date.getDay(): 0=Sun..6=Sat -> convert to Monday=0..Sunday=6
    readonly property int firstOffset: (new Date(viewYear, viewMonth, 1).getDay() + 6) % 7
    readonly property int weekRows: 6
    readonly property int cellHeight: 38
    readonly property int gridRowSpacing: 4
    readonly property int gridPreferredHeight: (weekRows * cellHeight) + ((weekRows - 1) * gridRowSpacing)

    Rectangle {
        anchors.fill: parent
        radius: 10
        color: "white"
        border.color: "#d0d0d0"
        border.width: 1
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 8

        RowLayout {
            Layout.fillWidth: true

            ToolButton {
                text: "‹"
                onClicked: root.viewDate = new Date(root.viewYear, root.viewMonth - 1, 1)
            }

            Label {
                text: Qt.formatDate(root.viewDate, "MMMM yyyy")
                font.pixelSize: 14
                font.bold: true
                Layout.fillWidth: true
                elide: Text.ElideRight
            }

            ToolButton {
                text: "›"
                onClicked: root.viewDate = new Date(root.viewYear, root.viewMonth + 1, 1)
            }

            ToolButton {
                text: "Today"
                onClicked: root.selectedDate = new Date()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 0

            Repeater {
                model: 7
                delegate: Label {
                    required property int index
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    text: Qt.locale().dayName(index + 1, Locale.ShortFormat)
                    font.pixelSize: 12
                    color: "#666666"
                }
            }
        }

        GridLayout {
            id: grid
            Layout.fillWidth: true
            Layout.minimumHeight: root.gridPreferredHeight
            Layout.preferredHeight: root.gridPreferredHeight
            Layout.maximumHeight: root.gridPreferredHeight
            columns: 7
            rowSpacing: root.gridRowSpacing
            columnSpacing: 4

            Repeater {
                model: 42
                delegate: Item {
                    required property int index

                    Layout.fillWidth: true
                    Layout.minimumHeight: root.cellHeight
                    Layout.preferredHeight: root.cellHeight
                    Layout.maximumHeight: root.cellHeight

                    readonly property int dayNumber: index - root.firstOffset + 1
                    readonly property bool inMonth: dayNumber >= 1 && dayNumber <= root.daysThisMonth
                    readonly property var cellDate: inMonth ? new Date(root.viewYear, root.viewMonth, dayNumber) : null
                    readonly property bool isSelected: inMonth && root.isSameDay(cellDate, root.selectedDate)

                    Rectangle {
                        anchors.fill: parent
                        radius: 8
                        color: isSelected ? "#e6f0ff" : (mouseArea.containsMouse ? "#f3f3f3" : "transparent")
                        border.width: isSelected ? 1 : 0
                        border.color: "#7aa7ff"
                    }

                    Text {
                        anchors.centerIn: parent
                        text: inMonth ? dayNumber : ""
                        color: inMonth ? "black" : "#999999"
                        font.pixelSize: 13
                    }

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        enabled: inMonth
                        onClicked: {
                            root.selectedDate = cellDate
                            root.datePicked(cellDate)
                        }
                    }
                }
            }
        }
    }

    function isSameDay(a, b) {
        if (!a || !b) return false
        return a.getFullYear() === b.getFullYear()
            && a.getMonth() === b.getMonth()
            && a.getDate() === b.getDate()
    }
}
