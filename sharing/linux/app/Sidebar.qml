import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "."

Rectangle {
    id: root
    Layout.fillHeight: true
    Layout.preferredWidth: compact ? 188 : 260
    color: AppSettings.navigation

    property bool compact: false
    property var pendingFiles: []
    property bool cancelEnabled: true

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.compact ? 14 : 18
        spacing: root.compact ? 10 : 14

        Text {
            text: "Visibility"
            color: AppSettings.mutedText
            font.pixelSize: 13
        }

        Button {
            id: visibilityButton
            Layout.fillWidth: true
            text: backend.visibleToEveryone ? "Always visible" : "Hidden"
            onClicked: backend.setVisibleToEveryone(!backend.visibleToEveryone)
            contentItem: RowLayout {
                Text {
                    text: visibilityButton.text
                    color: AppSettings.text
                    font.weight: 600
                    Layout.fillWidth: true
                }
                Text {
                    text: backend.visibleToEveryone ? "›" : "‹"
                    color: AppSettings.primary
                    font.pixelSize: 24
                }
            }
            background: Rectangle {
                implicitHeight: 48
                radius: 14
                color: visibilityButton.hovered ? AppSettings.surfaceHigh
                                                : AppSettings.surfaceContainer
            }
        }

        Text {
            Layout.fillWidth: true
            text: backend.visibleToEveryone
                  ? "Nearby devices can find " + backend.deviceName
                    + ". Every incoming transfer still needs your approval."
                  : "Your laptop is not advertising for incoming shares. Sending remains available."
            color: AppSettings.mutedText
            wrapMode: Text.WordWrap
            font.pixelSize: 12
            lineHeight: 1.25
        }

        Rectangle {
            visible: root.pendingFiles.length > 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 18
            color: AppSettings.surfaceContainer
            border.color: AppSettings.outline

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: root.compact ? 10 : 14
                spacing: root.compact ? 7 : 10

                Text {
                    text: root.pendingFiles.length === 1
                          ? "Sharing 1 file"
                          : "Sharing " + root.pendingFiles.length + " files"
                    color: AppSettings.text
                    font.pixelSize: 16
                    font.weight: 600
                }

                Rectangle {
                    visible: root.height >= 390
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: root.compact ? 64 : 82
                    Layout.preferredHeight: root.compact ? 64 : 82
                    radius: 20
                    color: AppSettings.surfaceHigh
                    Image {
                        anchors.centerIn: parent
                        width: root.compact ? 42 : 54
                        height: width
                        source: "qrc:/icons/file.svg"
                        fillMode: Image.PreserveAspectFit
                    }
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: root.pendingFiles
                    clip: true
                    spacing: 4
                    delegate: Text {
                        required property var modelData
                        width: ListView.view.width
                        text: AppSettings.baseName(modelData)
                        color: AppSettings.mutedText
                        font.pixelSize: 11
                        elide: Text.ElideMiddle
                    }
                }

                Button {
                    id: cancelButton
                    Layout.fillWidth: true
                    enabled: root.cancelEnabled
                    text: "Cancel"
                    onClicked: EventBus.cancelPendingShareRequested()
                    contentItem: Text {
                        text: cancelButton.text
                        color: AppSettings.text
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.weight: 600
                    }
                    background: Rectangle {
                        implicitHeight: 42
                        radius: 13
                        color: cancelButton.hovered ? AppSettings.surfaceHigh
                                                    : AppSettings.surface
                        border.color: AppSettings.outline
                    }
                }
            }
        }

        Item {
            visible: root.pendingFiles.length === 0
            Layout.fillHeight: true
        }

    }
}
