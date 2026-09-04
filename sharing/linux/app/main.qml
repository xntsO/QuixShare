import QtQuick
import QtQuick.Window
import "."

Window {
    id: root
    width: 920
    height: 640
    minimumWidth: 560
    minimumHeight: 400
    visible: true
    title: qsTr("QuixShare")
    color: AppSettings.appBackground

    Component.onCompleted: {
        // Evaluate this once at startup. A tray becoming available later must
        // never make an already-visible window disappear unexpectedly.
        root.visible = !AppSettings.startMinimized || !appController.closeToTray
    }

    onClosing: function(close) {
        if (appController.closeToTray && AppSettings.keepRunningOnClose) {
            close.accepted = false
            root.hide()
        }
    }

    Loader {
        id: contentLoader
        anchors.fill: parent
        source: "AppContent.qml"
    }

    function reloadInnerContent() {
        const nextSource = "AppContent.qml?rev=" + Date.now()
        contentLoader.active = false
        contentLoader.source = ""
        contentLoader.active = true
        contentLoader.source = nextSource
    }
}
