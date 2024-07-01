import QtQuick 2.15
import QtQuick.Controls
import org.deepin.filemanager.gui

AppletItem {
    property int flag: 1

    height: 100
    width: 500

    Rectangle {
        color: "lightyellow"
        height: 100
        width: 200

        Text {
            id: appletText

            text: "Applet: " + Applet
        }
    }
}
