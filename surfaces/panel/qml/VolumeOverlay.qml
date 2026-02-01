import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property int volume: 50

    property int popupWidth: 260
    property int popupHeight: 64

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

    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Image {
            source: "qrc:/resources/icons/volume.png"
            fillMode: Image.PreserveAspectFit
            sourceSize.width: 20
            sourceSize.height: 20
            Layout.preferredWidth: 20
            Layout.preferredHeight: 20
        }

        Slider {
            id: volumeSlider
            from: 0
            to: 100
            stepSize: 1
            value: root.volume
            Layout.fillWidth: true
            onValueChanged: root.volume = Math.round(value)
        }

        Text {
            text: Math.round(volumeSlider.value)
            color: "black"
            font.pixelSize: 13
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            Layout.preferredWidth: 32
        }
    }
}
