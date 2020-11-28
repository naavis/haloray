import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

ApplicationWindow {
    id: applicationWindow
    width: 640
    height: 480
    visible: true
    title: qsTr("HaloRay 3.0.0")

    menuBar: MenuBar {
        Menu {
            title: qsTr("File")

            MenuItem {
                text: qsTr("&Save image")
            }

            MenuItem {
                text: qsTr("&Quit")
                onClicked: Qt.quit()
            }
        }
    }

    RowLayout {
        ColumnLayout {
            PlaceholderControls { title: "General controls" }
            PlaceholderControls { title: "Crystal controls" }
            PlaceholderControls { title: "Camera controls" }
        }

        PlaceholderSkyWidget { }
    }
}
