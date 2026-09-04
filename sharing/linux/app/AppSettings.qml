pragma Singleton

import QtQuick
import QtCore

QtObject {
    id: root

    // Release checklist: replace this one placeholder after the public
    // repository and final project name have been selected. Settings links
    // are derived from it so they cannot drift independently.
    readonly property string projectUrl: "https://github.com/xntsO/QuixShare"
    readonly property string issuesUrl: projectUrl + "/issues"
    readonly property string projectHelpUrl: projectUrl + "#contributing-and-security"
    readonly property string upstreamUrl: "https://github.com/google/nearby"
    readonly property string linuxForkUrl: "https://github.com/kidfromjupiter/nearby"
    readonly property string interfaceInspirationUrl: "https://github.com/Martichou/rquickshare"

    property Settings storage: Settings {
        id: persisted
        category: "merged-ui"
        property string themePreference: "system"
        property bool keepRunningOnClose: true
        property bool startMinimized: false
    }

    property alias themePreference: persisted.themePreference
    property alias keepRunningOnClose: persisted.keepRunningOnClose
    property alias startMinimized: persisted.startMinimized
    readonly property bool dark: themePreference === "dark"
                                 || (themePreference === "system"
                                     && Application.styleHints.colorScheme
                                        === Qt.ColorScheme.Dark)

    readonly property color appBackground: dark ? "#111318" : "#EAF2FF"
    readonly property color topBar: dark ? "#17191E" : "#F8FAFD"
    readonly property color navigation: dark ? "#181C24" : "#EEF4FF"
    readonly property color surface: dark ? "#1F2024" : "#FFFFFF"
    readonly property color surfaceContainer: dark ? "#282A2F" : "#F0F4F9"
    readonly property color surfaceHigh: dark ? "#33353A" : "#E7EDF5"
    readonly property color primary: dark ? "#A8C7FA" : "#0B57D0"
    readonly property color primaryContainer: dark ? "#0842A0" : "#D3E3FD"
    readonly property color onPrimaryContainer: dark ? "#D3E3FD" : "#041E49"
    readonly property color text: dark ? "#E3E3E3" : "#1F1F1F"
    readonly property color mutedText: dark ? "#C4C7C5" : "#5F6368"
    readonly property color outline: dark ? "#444746" : "#C4C7C5"
    readonly property color danger: dark ? "#FFB4AB" : "#B3261E"
    readonly property color dangerContainer: dark ? "#8C1D18" : "#F9DEDC"
    readonly property color successContainer: dark ? "#174D2C" : "#C4EED0"

    function toggleTheme() {
        themePreference = dark ? "light" : "dark"
    }

    function localDisplayPath(path) {
        let value = String(path || "")
        if (value.indexOf("file://") === 0) {
            value = value.slice(7)
            try {
                value = decodeURIComponent(value)
            } catch (error) {
                // Keep malformed percent sequences displayable instead of
                // breaking the delegate that renders the file name.
            }
        }
        return value
    }

    function baseName(path) {
        const value = localDisplayPath(path)
        const pieces = value.split("/")
        return pieces.length > 0 ? pieces[pieces.length - 1] : value
    }
}
