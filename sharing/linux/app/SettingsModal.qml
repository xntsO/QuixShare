import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform as Platform
import QtCore
import "."

Item {
    id: root
    anchors.fill: parent
    visible: open
    z: 500

    property bool open: false
    signal closeRequested()

    focus: open
    Keys.onEscapePressed: root.closeRequested()
    onOpenChanged: {
        if (open) {
            deviceNameField.text = backend.deviceName
            forceActiveFocus()
        }
    }

    Platform.FolderDialog {
        id: folderDialog
        title: "Select where received files are saved"
        folder: backend.downloadPath.length > 0
                ? "file://" + backend.downloadPath
                : StandardPaths.writableLocation(StandardPaths.DownloadLocation)
        onAccepted: backend.setDownloadPath(folderDialog.folder)
    }

    Connections {
        target: appController
        function onDownloadFolderSelected(path) {
            backend.setDownloadPath(path)
        }
        function onNativeFolderPickerUnavailable() {
            if (root.open)
                folderDialog.open()
        }
    }

    Rectangle {
        anchors.fill: parent
        color: AppSettings.dark ? "#B0000000" : "#78000000"

        MouseArea {
            anchors.fill: parent
            onClicked: root.closeRequested()
        }
    }

    Rectangle {
        id: settingsPanel
        width: Math.min(500, parent.width - 32)
        height: Math.min(600, parent.height - 32)
        anchors.centerIn: parent
        radius: 26
        color: AppSettings.surface
        border.color: AppSettings.outline
        border.width: 1

        MouseArea {
            anchors.fill: parent
            onClicked: function(mouse) { mouse.accepted = true }
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: settingsPanel.width < 420 ? 16 : 24
            spacing: settingsPanel.height < 480 ? 10 : 14

            RowLayout {
                Layout.fillWidth: true

                Text {
                    text: "Settings"
                    color: AppSettings.text
                    font.pixelSize: 23
                    font.weight: 600
                    Layout.fillWidth: true
                }

                Button {
                    id: closeButton
                    text: "Close"
                    onClicked: root.closeRequested()
                    contentItem: Text {
                        text: closeButton.text
                        color: AppSettings.primary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.weight: 600
                    }
                    background: Rectangle {
                        implicitWidth: 72
                        implicitHeight: 38
                        radius: 12
                        color: closeButton.hovered ? AppSettings.surfaceHigh
                                                   : AppSettings.surfaceContainer
                    }
                }
            }

            ScrollView {
                id: settingsScroll
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                contentWidth: availableWidth
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                ScrollBar.vertical.policy: ScrollBar.AsNeeded

                ColumnLayout {
                    width: settingsScroll.availableWidth
                    spacing: 12

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 92
                        radius: 16
                        color: AppSettings.surfaceContainer

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 8

                            Text {
                                text: "Appearance"
                                color: AppSettings.text
                                font.weight: 600
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 4

                                Repeater {
                                    model: ["system", "light", "dark"]

                                    Button {
                                        id: themeChoice
                                        required property string modelData
                                        Layout.fillWidth: true
                                        text: modelData.charAt(0).toUpperCase()
                                              + modelData.slice(1)
                                        onClicked: AppSettings.themePreference = modelData
                                        contentItem: Text {
                                            text: themeChoice.text
                                            color: AppSettings.themePreference === themeChoice.modelData
                                                   ? AppSettings.onPrimaryContainer
                                                   : AppSettings.mutedText
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                            font.weight: 600
                                        }
                                        background: Rectangle {
                                            implicitHeight: 38
                                            radius: 11
                                            color: AppSettings.themePreference === themeChoice.modelData
                                                   ? AppSettings.primaryContainer
                                                   : (themeChoice.hovered ? AppSettings.surfaceHigh
                                                                          : "transparent")
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 108
                        radius: 16
                        color: AppSettings.surfaceContainer

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 7

                            Text {
                                text: "Device name"
                                color: AppSettings.text
                                font.weight: 600
                            }

                            RowLayout {
                                Layout.fillWidth: true

                                TextField {
                                    id: deviceNameField
                                    Layout.fillWidth: true
                                    text: backend.deviceName
                                    selectByMouse: true
                                    color: AppSettings.text
                                    placeholderText: "Name shown to nearby devices"
                                    onAccepted: backend.setDeviceName(text)
                                    background: Rectangle {
                                        radius: 10
                                        color: AppSettings.surface
                                        border.color: deviceNameField.activeFocus
                                                      ? AppSettings.primary
                                                      : AppSettings.outline
                                    }
                                }

                                Button {
                                    id: saveButton
                                    enabled: deviceNameField.text.trim().length > 0
                                    text: "Save"
                                    onClicked: backend.setDeviceName(deviceNameField.text)
                                    contentItem: Text {
                                        text: saveButton.text
                                        color: AppSettings.onPrimaryContainer
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                        font.weight: 600
                                    }
                                    background: Rectangle {
                                        implicitWidth: 66
                                        implicitHeight: 40
                                        radius: 11
                                        color: AppSettings.primaryContainer
                                    }
                                }
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 82
                        radius: 16
                        color: AppSettings.surfaceContainer

                        MouseArea {
                            anchors.fill: parent
                            enabled: !appController.folderPickerBusy
                            cursorShape: enabled ? Qt.PointingHandCursor
                                                 : Qt.ArrowCursor
                            onClicked: {
                                if (!appController.chooseDownloadFolder(
                                            backend.downloadPath)) {
                                    folderDialog.open()
                                }
                            }
                        }

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 4

                            Text {
                                text: "Received files folder"
                                color: AppSettings.text
                                font.weight: 600
                            }
                            Text {
                                Layout.fillWidth: true
                                text: appController.folderPickerBusy
                                      ? "Opening the system folder picker…"
                                      : backend.downloadPath
                                color: AppSettings.mutedText
                                font.pixelSize: 12
                                elide: Text.ElideMiddle
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: Math.max(58,
                                                         keepRunningLabel.implicitHeight + 24)
                        radius: 16
                        color: AppSettings.surfaceContainer

                        RowLayout {
                            id: keepRunningLayout
                            anchors.fill: parent
                            anchors.margins: 12
                            Text {
                                id: keepRunningLabel
                                text: appController.closeToTray
                                      ? "Keep running when the window closes"
                                      : "Keep running when closed (tray unavailable)"
                                color: appController.closeToTray
                                       ? AppSettings.text : AppSettings.mutedText
                                Layout.fillWidth: true
                                wrapMode: Text.WordWrap
                                maximumLineCount: 2
                                elide: Text.ElideRight
                            }
                            Switch {
                                enabled: appController.closeToTray
                                checked: AppSettings.keepRunningOnClose
                                onToggled: AppSettings.keepRunningOnClose = checked
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: Math.max(58,
                                                         startMinimizedLabel.implicitHeight + 24)
                        radius: 16
                        color: AppSettings.surfaceContainer

                        RowLayout {
                            id: startMinimizedLayout
                            anchors.fill: parent
                            anchors.margins: 12
                            Text {
                                id: startMinimizedLabel
                                text: appController.closeToTray
                                      ? "Start minimized"
                                      : "Start minimized (tray unavailable)"
                                color: appController.closeToTray
                                       ? AppSettings.text : AppSettings.mutedText
                                Layout.fillWidth: true
                                wrapMode: Text.WordWrap
                                maximumLineCount: 2
                                elide: Text.ElideRight
                            }
                            Switch {
                                enabled: appController.closeToTray
                                checked: AppSettings.startMinimized
                                onToggled: AppSettings.startMinimized = checked
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: aboutLayout.implicitHeight + 28
                        radius: 16
                        color: AppSettings.surfaceContainer

                        ColumnLayout {
                            id: aboutLayout
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.top: parent.top
                            anchors.margins: 14
                            spacing: 8

                            Text {
                                text: "About & project help"
                                color: AppSettings.text
                                font.pixelSize: 17
                                font.weight: 600
                            }

                            Text {
                                Layout.fillWidth: true
                                text: "An independent, unofficial Linux compatibility client. It is not affiliated with or endorsed by Google or device manufacturers."
                                color: AppSettings.mutedText
                                font.pixelSize: 12
                                wrapMode: Text.WordWrap
                            }

                            Text {
                                Layout.fillWidth: true
                                text: "Built from the open-source Google Nearby codebase and community Linux work. The interface was inspired by RQuickShare, but no RQuickShare code is included. Core source is Apache-2.0; included components keep their own licenses. The QuixShare name and artwork are owned by xntsO and covered by a separate branding policy."
                                color: AppSettings.mutedText
                                font.pixelSize: 12
                                wrapMode: Text.WordWrap
                            }

                            GridLayout {
                                Layout.fillWidth: true
                                columns: width >= 340 ? 2 : 1
                                columnSpacing: 8
                                rowSpacing: 8

                                Repeater {
                                    model: [
                                        { label: "Source & licenses", url: AppSettings.projectUrl },
                                        { label: "Upstream credit", url: AppSettings.upstreamUrl },
                                        { label: "UI inspiration", url: AppSettings.interfaceInspirationUrl },
                                        { label: "Report an issue", url: AppSettings.issuesUrl },
                                        { label: "Contribute / support", url: AppSettings.projectHelpUrl }
                                    ]

                                    Button {
                                        id: projectButton
                                        required property var modelData
                                        Layout.fillWidth: true
                                        text: modelData.label
                                        onClicked: Qt.openUrlExternally(modelData.url)
                                        contentItem: Text {
                                            text: projectButton.text
                                            color: AppSettings.primary
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                            font.weight: 600
                                        }
                                        background: Rectangle {
                                            implicitHeight: 40
                                            radius: 11
                                            color: projectButton.hovered
                                                   ? AppSettings.surfaceHigh
                                                   : AppSettings.surface
                                        }
                                    }
                                }
                            }

                            Text {
                                Layout.fillWidth: true
                                text: "Quick Share and Google names may be trademarks of their owners. Compatibility is not guaranteed."
                                color: AppSettings.mutedText
                                font.pixelSize: 11
                                wrapMode: Text.WordWrap
                            }
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                    }
                }
            }
        }
    }
}
