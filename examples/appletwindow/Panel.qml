import QtQuick 2.15
import QtQuick.Controls
import org.deepin.filemanager.gui

ApplicationWindow {
    height: 600
    width: 800

    Text {
        id: text

        text: "Applets: " + Containment.applets
    }

    SplitView {
        anchors.bottom: parent.bottom
        anchors.top: text.bottom
        width: parent.width

        Rectangle {
            SplitView.preferredWidth: 150

            Control {
                contentItem: Containment.objectForProperty("flag", 1)
            }
        }

        Rectangle {
            id: content

            SplitView.fillHeight: true
            SplitView.fillWidth: true
            color: "lightblue"

            Column {
                id: layout

                Button {
                    id: button

                    text: "add item"

                    onClicked: {
                        let item = Containment.objectForProperty("flag", 1);
                        if (item) {
                            leftControl.contentItem = item;
                        }
                    }
                }

                Control {
                    id: leftControl

                    onContentItemChanged: {
                        contentItem.anchors.fill = leftControl;
                    }
                }
            }
        }
    }
}
