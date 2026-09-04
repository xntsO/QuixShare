import QtQuick
import QtQuick.Layouts
import "."

Rectangle {
    id: root
    width: 142
    height: 126
    radius: 20
    color: mouseArea.containsMouse && interactionEnabled
           ? AppSettings.surfaceHigh : AppSettings.surfaceContainer
    border.color: mouseArea.containsMouse && interactionEnabled
                  ? AppSettings.primary : AppSettings.outline
    border.width: 1

    property string deviceName: "Unknown device"
    property int deviceType: 0
    property var shareTargetId: 0
    property bool interactionEnabled: true

    function iconSource() {
        if (deviceType === 3)
            return "qrc:/icons/laptop.svg"
        if (deviceType === 2)
            return "qrc:/icons/tablet.svg"
        return "qrc:/icons/smartphone.svg"
    }

    Behavior on color { ColorAnimation { duration: 100 } }
    Behavior on opacity { NumberAnimation { duration: 160 } }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 62
            Layout.preferredHeight: 62
            radius: 31
            color: AppSettings.primaryContainer
            Image {
                anchors.centerIn: parent
                width: 38
                height: 38
                source: root.iconSource()
                fillMode: Image.PreserveAspectFit
            }
        }

        Text {
            Layout.fillWidth: true
            text: root.deviceName
            color: AppSettings.text
            font.pixelSize: 12
            font.weight: 600
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        enabled: root.interactionEnabled
        hoverEnabled: true
        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: EventBus.shareTargetSelected(root.shareTargetId)
    }
}
