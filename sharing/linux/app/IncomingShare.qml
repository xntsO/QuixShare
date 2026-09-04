import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "."

Rectangle {
    id: root
    color: AppSettings.surface
    Layout.fillWidth: true
    Layout.fillHeight: true

    property var shareTargetId: 0
    property string direction: "receive"
    property string filename: "ThisIsAnImage.jpg"
    property string targetname: "Nearby device"
    property bool transferring: !awaitingLocalConfirmation
    property real progressValue: 0.64
    property string status: "kUnknown"
    property bool isFinalStatus: false
    property bool awaitingLocalConfirmation: false
    property int totalAttachmentsCount: 0
    property int transferredAttachmentsCount: 0
    property var totalBytes: 0
    property var transferredBytes: 0
    property bool resolutionRequested: false
    property bool cancelRequested: false
    readonly property bool compact: width < 500 || height < 440

    signal returnHomeRequested()

    function resolveOffer(acceptOffer) {
        if (resolutionRequested)
            return
        resolutionRequested = true
        responseTimeout.restart()
        if (acceptOffer)
            backend.accept(shareTargetId)
        else
            backend.reject(shareTargetId)
    }

    Timer {
        id: responseTimeout
        interval: 8000
        onTriggered: {
            if (root.awaitingLocalConfirmation)
                root.resolutionRequested = false
        }
    }

    onAwaitingLocalConfirmationChanged: {
        if (awaitingLocalConfirmation) {
            resolutionRequested = false
        } else {
            responseTimeout.stop()
        }
    }

    onIsFinalStatusChanged: {
        if (isFinalStatus) {
            cancelTimeout.stop()
            cancelRequested = false
        }
    }

    Timer {
        id: cancelTimeout
        interval: 8000
        onTriggered: root.cancelRequested = false
    }

    function baseName(path) {
        if (!path || path.length === 0) {
            return direction === "send" ? "Selected file" : transferSummary()
        }
        return AppSettings.baseName(path)
    }

    function transferSummary() {
        if (totalAttachmentsCount > 0) {
            return totalAttachmentsCount === 1 ? "1 item" : totalAttachmentsCount + " items"
        }
        if (totalBytes > 0) {
            return formatBytes(totalBytes)
        }
        return "Shared items"
    }

    function formatBytes(bytes) {
        let value = Number(bytes)
        if (!isFinite(value) || value <= 0) {
            return ""
        }
        const units = ["B", "KB", "MB", "GB", "TB"]
        let unitIndex = 0
        while (value >= 1024 && unitIndex < units.length - 1) {
            value /= 1024
            unitIndex += 1
        }
        return (unitIndex === 0 ? Math.round(value) : value.toFixed(value >= 10 ? 1 : 2)) + " " + units[unitIndex]
    }

    function statusText() {
        if (status === "kAwaitingLocalConfirmation") {
            return "Incoming share request"
        }
        if (status === "kConnecting") {
            return direction === "send" ? "Connecting" : "Preparing transfer"
        }
        if (status === "kAwaitingRemoteAcceptance") {
            return "Waiting for receiver"
        }
        if (status === "kInProgress") {
            const verb = direction === "send" ? "Sending" : "Receiving"
            if (totalAttachmentsCount > 0) {
                return verb + " " + Math.min(transferredAttachmentsCount + 1, totalAttachmentsCount) + " of " + totalAttachmentsCount + " items"
            }
            return verb
        }
        if (status === "kComplete") {
            return direction === "send" ? "Sent" : "Received"
        }
        if (status === "kReject") {
            return "Rejected"
        }
        if (status === "kCancelled") {
            return "Cancelled"
        }
        if (status === "kTimedOut") {
            return "Timed out"
        }
        if (isFinalStatus) {
            return "Transfer failed"
        }
        return direction === "send" ? "Starting send" : "Preparing transfer"
    }

    TransferCard {
        visible: root.transferring
        anchors.centerIn: parent
        width: Math.min(parent.width - (root.compact ? 24 : 48), 700)
        height: Math.min(root.compact ? 176 : 212, parent.height - 24)
        compact: root.compact
        shareTargetId: root.shareTargetId
        direction: root.direction
        filename: root.filename
        targetname: root.targetname
        progressValue: root.progressValue
        status: root.status
        isFinalStatus: root.isFinalStatus
        totalBytes: root.totalBytes
        totalAttachmentsCount: root.totalAttachmentsCount
        transferredAttachmentsCount: root.transferredAttachmentsCount
        cancelEnabled: !root.cancelRequested
        finalActionText: root.isFinalStatus ? "Go back home" : ""
        onCancelRequested: {
            root.cancelRequested = true
            cancelTimeout.restart()
            backend.cancel(root.shareTargetId)
        }
        onFinalActionRequested: {
            backend.clearTransfer(root.shareTargetId)
            root.returnHomeRequested()
        }
    }

    Item {
        visible: !root.transferring
        anchors.centerIn: parent
        width: Math.min(parent.width - (root.compact ? 24 : 48), 360)
        height: Math.min(root.compact ? 360 : 460, parent.height - 24)

        Rectangle {
            id: cardBackground
            anchors.fill: parent
            radius: 24
            color: AppSettings.surfaceContainer
            border.color: AppSettings.outline
            border.width: 1
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: root.compact ? 18 : 28
            spacing: root.compact ? 10 : 16

            Text {
                Layout.fillWidth: true
                font.pointSize: root.compact ? 16 : 18
                font.weight: 700
                color: AppSettings.primary
                text: root.direction === "send" ? "Outgoing share" : "Incoming share"
                horizontalAlignment: Text.AlignHCenter
            }

            Image {
                source: "qrc:/icons/file.svg"
                Layout.fillHeight: true
                Layout.fillWidth: true
                fillMode: Image.PreserveAspectCrop
                sourceSize.height: height
                sourceSize.width: width
                smooth: true
                antialiasing: true
            }

            Text {
                Layout.fillWidth: true
                color: AppSettings.mutedText
                font.pointSize: 11
                wrapMode: Text.WrapAnywhere
                text: root.baseName(root.filename)
                horizontalAlignment: Text.AlignHCenter
                maximumLineCount: 2
                elide: Text.ElideMiddle
            }

            Text {
                Layout.fillWidth: true
                color: AppSettings.text
                font.pointSize: root.compact ? 13 : 15
                font.weight: 700
                wrapMode: Text.WrapAnywhere
                text: root.targetname
                horizontalAlignment: Text.AlignHCenter
                maximumLineCount: 2
                elide: Text.ElideMiddle
            }

            RowLayout {
                Button {
                    id: cancelbutton
                    Layout.fillWidth: true
                    enabled: !root.resolutionRequested
                    text: root.resolutionRequested ? "Responding…" : "Reject"
                    onClicked: root.resolveOffer(false)

                    contentItem: Text {
                        text: cancelbutton.text
                        font.pointSize: 14
                        font.weight: 600
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        implicitHeight: 46
                        radius: 12
                        color: cancelbutton.hovered ? "#B62828" : "#D94C4C"
                        border.color: "transparent"
                        border.width: 1
                    }
                }

                Button {
                    id: acceptButton
                    Layout.fillWidth: true
                    enabled: !root.resolutionRequested
                    text: root.resolutionRequested ? "Responding…" : "Accept"
                    onClicked: root.resolveOffer(true)

                    contentItem: Text {
                        text: acceptButton.text
                        font.pointSize: 14
                        font.weight: 600
                        color: AppSettings.onPrimaryContainer
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        implicitHeight: 46
                        radius: 12
                        color: AppSettings.primaryContainer
                        border.color: AppSettings.primary
                        border.width: 1
                    }
                }
            }
        }
    }
}
