pragma ComponentBehavior: Bound

import QtQuick
import "."

Item {
    id: root
    implicitHeight: targetFlow.implicitHeight

    property bool interactionLocked: false
    property var activeTransferId: 0

    Flow {
        id: targetFlow
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(parent.width, 620)
        spacing: 12

        Repeater {
            model: backend.targets
            ShareTarget {
                required property var model
                shareTargetId: model.targetId
                deviceName: model.deviceName
                deviceType: model.type
                interactionEnabled: !root.interactionLocked
                opacity: !root.interactionLocked
                         || model.targetId == root.activeTransferId ? 1 : 0.35
            }
        }
    }
}
