import QtQuick
import QtQuick.Layouts
import "."

Item {
    id: root
    property bool transferLocked: false
    property var activeTransferId: 0
    readonly property bool compact: width < 500 || height < 420

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.compact ? 16 : 32
        spacing: root.compact ? 8 : 14

        Text {
            Layout.fillWidth: true
            text: root.transferLocked ? "Transfer in progress" : "Nearby devices"
            color: AppSettings.text
            font.pixelSize: root.compact ? 20 : 23
            font.weight: 600
            elide: Text.ElideRight
        }
        Text {
            Layout.fillWidth: true
            text: root.transferLocked
                  ? "Keep both devices close until the transfer finishes."
                  : "Choose a device to send the selected files."
            color: AppSettings.mutedText
            font.pixelSize: 13
            wrapMode: Text.WordWrap
            maximumLineCount: 2
            elide: Text.ElideRight
        }

        Flickable {
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentWidth: width
            contentHeight: searchBody.implicitHeight
            clip: true
            boundsBehavior: Flickable.StopAtBounds

            ColumnLayout {
                id: searchBody
                y: Math.max(0, (parent.height - implicitHeight) / 2)
                width: parent.width
                spacing: root.compact ? 14 : 30

                Item {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: root.compact ? 54 : 70
                    Layout.preferredHeight: root.compact ? 54 : 70
                    Rectangle {
                        anchors.centerIn: parent
                        width: root.compact ? 46 : 60
                        height: width
                        radius: width / 2
                        color: AppSettings.primaryContainer
                    }
                    Rectangle {
                        anchors.centerIn: parent
                        width: root.compact ? 46 : 60
                        height: width
                        radius: width / 2
                        color: "transparent"
                        border.color: AppSettings.primary
                        SequentialAnimation on scale {
                            loops: Animation.Infinite
                            NumberAnimation { from: 1; to: 1.75; duration: 1400; easing.type: Easing.OutCubic }
                            NumberAnimation { from: 1.75; to: 1; duration: 0 }
                        }
                        SequentialAnimation on opacity {
                            loops: Animation.Infinite
                            NumberAnimation { from: 0.55; to: 0; duration: 1400 }
                            NumberAnimation { from: 0; to: 0.55; duration: 0 }
                        }
                    }
                    Text {
                        anchors.centerIn: parent
                        text: "⌁"
                        color: AppSettings.onPrimaryContainer
                        font.pixelSize: root.compact ? 24 : 30
                    }
                }

                Targets {
                    Layout.fillWidth: true
                    interactionLocked: root.transferLocked
                    activeTransferId: root.activeTransferId
                }

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.maximumWidth: parent.width - 24
                    text: "Searching with Bluetooth and the current Wi‑Fi network"
                    color: AppSettings.mutedText
                    font.pixelSize: 11
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                }
            }
        }
    }
}
