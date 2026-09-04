import QtQuick
import QtQuick.Layouts
import Qt.labs.platform as Platform
import QtCore
import "."

Rectangle {
    id: root
    readonly property bool compact: width < 440 || height < 140
    radius: 20
    color: dropArea.containsDrag ? AppSettings.primaryContainer
                                 : AppSettings.surfaceContainer
    border.color: dropArea.containsDrag ? AppSettings.primary
                                        : AppSettings.outline
    border.width: dropArea.containsDrag ? 2 : 1

    Platform.FileDialog {
        id: fileDialog
        title: "Choose files to share"
        folder: StandardPaths.writableLocation(StandardPaths.HomeLocation)
        fileMode: Platform.FileDialog.OpenFiles
        nameFilters: ["All files (*)"]
        onAccepted: {
            const selected = []
            for (let index = 0; index < fileDialog.files.length; ++index)
                selected.push(fileDialog.files[index].toString())
            EventBus.filesSelected(selected)
        }
    }

    Connections {
        target: appController
        function onFilesSelected(paths) {
            EventBus.filesSelected(paths)
        }
        function onNativeFilePickerUnavailable() {
            if (root.visible)
                fileDialog.open()
        }
    }

    DropArea {
        id: dropArea
        anchors.fill: parent
        onDropped: function(drop) {
            if (!drop.hasUrls)
                return
            const selected = []
            for (let index = 0; index < drop.urls.length; ++index)
                selected.push(drop.urls[index].toString())
            drop.acceptProposedAction()
            EventBus.filesSelected(selected)
        }
    }

    MouseArea {
        anchors.fill: parent
        enabled: !appController.filePickerBusy
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            if (!appController.chooseFiles(
                        StandardPaths.writableLocation(
                            StandardPaths.HomeLocation))) {
                fileDialog.open()
            }
        }
    }

    RowLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: root.compact ? 14 : 24
        spacing: root.compact ? 12 : 18

        Rectangle {
            Layout.preferredWidth: root.compact ? 44 : 54
            Layout.preferredHeight: root.compact ? 44 : 54
            radius: root.compact ? 13 : 16
            color: AppSettings.surfaceHigh
            Text {
                anchors.centerIn: parent
                text: "+"
                color: AppSettings.primary
                font.pixelSize: root.compact ? 26 : 31
                font.weight: 500
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            Text {
                Layout.fillWidth: true
                text: appController.filePickerBusy
                      ? "Opening the system file picker…"
                      : (dropArea.containsDrag ? "Release to share"
                                               : "Drop files to send")
                color: AppSettings.text
                font.pixelSize: root.compact ? 15 : 17
                font.weight: 600
                elide: Text.ElideRight
            }
            Text {
                Layout.fillWidth: true
                text: "or click to select one or more files"
                color: AppSettings.mutedText
                font.pixelSize: root.compact ? 11 : 12
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                elide: Text.ElideRight
            }
        }
    }
}
