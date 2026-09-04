pragma Singleton

import QtQuick

QtObject {
    signal fileSelected(string filePath)
    signal filesSelected(var filePaths)
    signal cancelPendingShareRequested()
    signal shareTargetSelected(var shareTargetId)
}
