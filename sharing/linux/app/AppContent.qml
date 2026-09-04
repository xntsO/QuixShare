pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "."

Rectangle {
    id: root
    anchors.fill: parent
    color: AppSettings.appBackground

    property var pendingFiles: []
    property int currentIndex: 0
    property var selectedTransferId: 0
    property var activeOutgoingTransferId: 0
    property bool outgoingStartFailed: false
    property bool settingsOpen: false
    property string errorMessage: ""
    readonly property bool outgoingTransferActive: activeOutgoingTransferId != 0
    readonly property bool narrowLayout: width < 720
    readonly property bool shortLayout: height < 520
    readonly property int pageMargin: narrowLayout || shortLayout ? 16 : 32

    function selectFiles(paths) {
        const files = []
        for (let index = 0; index < paths.length; ++index) {
            files.push(String(paths[index]))
        }
        if (files.length === 0)
            return
        pendingFiles = files
        backend.startDiscovery()
        currentIndex = 1
    }

    function returnHome() {
        pendingFiles = []
        selectedTransferId = 0
        activeOutgoingTransferId = 0
        outgoingStartFailed = false
        currentIndex = 0
        backend.startReceive()
    }

    Connections {
        target: backend
        function onIncomingTransfer(shareTargetId) {
            root.selectedTransferId = shareTargetId
            root.currentIndex = 2
        }
        function onOutgoingTransferStartFailed(shareTargetId) {
            if (shareTargetId == root.activeOutgoingTransferId)
                root.outgoingStartFailed = true
        }
        function onSettingChangeFailed(setting, message) {
            root.errorMessage = message
            errorTimer.restart()
        }
    }

    Connections {
        target: EventBus
        function onFileSelected(path) { root.selectFiles([path]) }
        function onFilesSelected(paths) { root.selectFiles(paths) }
        function onShareTargetSelected(shareTargetId) {
            if (root.pendingFiles.length === 0 || root.outgoingTransferActive)
                return
            if (backend.sendFiles(shareTargetId, root.pendingFiles)) {
                root.outgoingStartFailed = false
                root.activeOutgoingTransferId = shareTargetId
            } else {
                root.errorMessage = "The transfer could not be started. Check that every selected file still exists."
                errorTimer.restart()
            }
        }
        function onCancelPendingShareRequested() {
            if (!root.outgoingTransferActive)
                root.returnHome()
        }
    }

    Timer {
        id: errorTimer
        interval: 4500
        onTriggered: root.errorMessage = ""
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: root.shortLayout ? 72 : 88
            color: AppSettings.topBar
            border.color: AppSettings.outline
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: root.narrowLayout ? 16 : 26
                anchors.rightMargin: root.narrowLayout ? 14 : 20
                spacing: root.narrowLayout ? 8 : 12

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    Text {
                        text: "Device name"
                        color: AppSettings.mutedText
                        font.pixelSize: 12
                    }
                    Text {
                        text: backend.deviceName
                        color: AppSettings.text
                        font.pixelSize: root.narrowLayout ? 20 : 24
                        font.weight: 600
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                        Layout.maximumWidth: root.narrowLayout ? 360 : 460
                    }
                }

                Rectangle {
                    visible: !root.narrowLayout
                    Layout.preferredWidth: 156
                    Layout.preferredHeight: 34
                    radius: 17
                    color: backend.visibleToEveryone ? AppSettings.successContainer
                                                       : AppSettings.surfaceContainer
                    RowLayout {
                        anchors.centerIn: parent
                        spacing: 7
                        Rectangle {
                            Layout.preferredWidth: 8
                            Layout.preferredHeight: 8
                            radius: 4
                            color: backend.visibleToEveryone ? "#34A853"
                                                               : AppSettings.mutedText
                        }
                        Text {
                            text: backend.visibleToEveryone ? "Ready to receive"
                                                            : "Receiving paused"
                            color: AppSettings.text
                            font.pixelSize: 12
                            font.weight: 600
                        }
                    }
                }

                Button {
                    id: themeButton
                    text: AppSettings.dark ? "☀" : "☾"
                    onClicked: AppSettings.toggleTheme()
                    ToolTip.visible: hovered
                    ToolTip.text: AppSettings.dark ? "Use light theme" : "Use dark theme"
                    contentItem: Text {
                        text: themeButton.text
                        color: AppSettings.text
                        font.pixelSize: 22
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        implicitWidth: 42
                        implicitHeight: 42
                        radius: 13
                        color: themeButton.hovered ? AppSettings.surfaceHigh
                                                   : AppSettings.surfaceContainer
                    }
                }

                Button {
                    id: settingsButton
                    text: "⚙"
                    onClicked: root.settingsOpen = true
                    ToolTip.visible: hovered
                    ToolTip.text: "Settings"
                    contentItem: Text {
                        text: settingsButton.text
                        color: AppSettings.text
                        font.pixelSize: 20
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        implicitWidth: 42
                        implicitHeight: 42
                        radius: 13
                        color: settingsButton.hovered ? AppSettings.surfaceHigh
                                                      : AppSettings.surfaceContainer
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Sidebar {
                compact: root.narrowLayout
                pendingFiles: root.pendingFiles
                cancelEnabled: !root.outgoingTransferActive
            }

            Rectangle {
                id: contentPane
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: AppSettings.surface
                radius: root.narrowLayout ? 20 : 28

                StackLayout {
                    anchors.fill: parent
                    currentIndex: root.currentIndex

                    Item {
                        ColumnLayout {
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.topMargin: root.pageMargin
                            anchors.bottomMargin: root.pageMargin
                            width: Math.min(parent.width - 2 * root.pageMargin, 900)
                            spacing: root.shortLayout ? 10 : 16

                            Text {
                                Layout.fillWidth: true
                                text: "Ready to receive or send"
                                color: AppSettings.text
                                font.pixelSize: root.narrowLayout ? 20 : 23
                                font.weight: 600
                                elide: Text.ElideRight
                            }
                            Text {
                                Layout.fillWidth: true
                                text: backend.visibleToEveryone
                                      ? backend.deviceName + " is visible to nearby Quick Share devices."
                                      : "Receiving is paused. You can still select files to send."
                                color: AppSettings.mutedText
                                font.pixelSize: 13
                                wrapMode: Text.WordWrap
                                maximumLineCount: 2
                                elide: Text.ElideRight
                            }

                            Item {
                                id: receivePulseArea
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                Rectangle {
                                    anchors.centerIn: parent
                                    width: Math.max(0, Math.min(150, receivePulseArea.height - 8,
                                                                receivePulseArea.width - 16))
                                    height: width
                                    radius: width / 2
                                    color: "transparent"
                                    border.color: AppSettings.primary
                                    opacity: 0.10
                                    SequentialAnimation on scale {
                                        loops: Animation.Infinite
                                        NumberAnimation { from: 0.65; to: 1.15; duration: 1900; easing.type: Easing.OutCubic }
                                        NumberAnimation { from: 1.15; to: 0.65; duration: 0 }
                                    }
                                }
                                Rectangle {
                                    anchors.centerIn: parent
                                    width: Math.min(82, Math.max(0, receivePulseArea.height - 18),
                                                    Math.max(0, receivePulseArea.width - 26))
                                    height: width
                                    radius: width / 2
                                    color: AppSettings.primaryContainer
                                    Text {
                                        anchors.centerIn: parent
                                        text: "⌁"
                                        color: AppSettings.onPrimaryContainer
                                        font.pixelSize: 38
                                        font.weight: 600
                                    }
                                }
                            }

                            Drop {
                                Layout.fillWidth: true
                                Layout.preferredHeight: root.shortLayout ? 116 : 170
                            }
                        }
                    }

                    SearchingDevices {
                        transferLocked: root.outgoingTransferActive
                        activeTransferId: root.activeOutgoingTransferId
                    }

                    Item {
                        Repeater {
                            model: backend.transfers
                            IncomingShare {
                                required property var model
                                anchors.fill: parent
                                visible: model.direction === "receive"
                                         && model.transferId == root.selectedTransferId
                                shareTargetId: model.transferId
                                direction: model.direction
                                filename: model.localPath
                                targetname: model.deviceName
                                progressValue: model.progress
                                status: model.status
                                totalBytes: model.totalBytes
                                transferredBytes: model.transferredBytes
                                totalAttachmentsCount: model.totalAttachmentsCount
                                transferredAttachmentsCount: model.transferredAttachmentsCount
                                isFinalStatus: model.isFinalStatus
                                awaitingLocalConfirmation: model.awaitingLocalConfirmation
                                onReturnHomeRequested: root.returnHome()
                            }
                        }
                    }
                }

                Repeater {
                    model: backend.transfers
                    TransferCard {
                        id: outgoingCard
                        required property var model
                        property bool cancelPending: false
                        width: Math.min(contentPane.width - 40, 690)
                        height: Math.min(150, contentPane.height - 40)
                        x: (contentPane.width - width) / 2
                        y: 20
                        z: 100
                        compact: true
                        visible: root.currentIndex === 1
                                 && model.direction === "send"
                                 && model.transferId == root.activeOutgoingTransferId
                        shareTargetId: model.transferId
                        direction: model.direction
                        filename: model.localPath
                        targetname: model.deviceName
                        progressValue: model.progress
                        status: root.outgoingStartFailed ? "kStartFailed" : model.status
                        isFinalStatus: model.isFinalStatus || root.outgoingStartFailed
                        totalBytes: model.totalBytes
                        totalAttachmentsCount: model.totalAttachmentsCount
                        transferredAttachmentsCount: model.transferredAttachmentsCount
                        cancelEnabled: !cancelPending
                        finalActionText: isFinalStatus ? "Done" : ""
                        onVisibleChanged: {
                            if (visible && !isFinalStatus) {
                                cancelPending = false
                                cancelTimeout.stop()
                            }
                        }
                        onCancelRequested: {
                            cancelPending = true
                            cancelTimeout.restart()
                            backend.cancel(model.transferId)
                        }
                        onIsFinalStatusChanged: {
                            if (isFinalStatus)
                                cancelTimeout.stop()
                        }
                        onFinalActionRequested: {
                            backend.clearTransfer(model.transferId)
                            root.returnHome()
                        }

                        Timer {
                            id: cancelTimeout
                            interval: 8000
                            onTriggered: outgoingCard.cancelPending = false
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        visible: root.errorMessage.length > 0
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 18
        width: Math.min(errorText.implicitWidth + 44, parent.width - 40)
        height: Math.max(48, errorText.implicitHeight + 20)
        radius: 14
        color: AppSettings.dangerContainer
        z: 800
        Text {
            id: errorText
            anchors.centerIn: parent
            text: root.errorMessage
            color: AppSettings.text
            font.weight: 600
            width: parent.width - 28
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            maximumLineCount: 3
            elide: Text.ElideRight
        }
    }

    SettingsModal {
        open: root.settingsOpen
        onCloseRequested: root.settingsOpen = false
    }
}
