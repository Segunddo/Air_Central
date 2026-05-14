import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    visible: true
    width: 750
    height: 450
    title: "Gerenciamento Ar Condicionado"
    color: "#f4f4f4"

    StackView {
        id: stackView
        anchors.fill: parent

        // Define qual tela aparece primeiro (seu código atual)
        initialItem: "TelaLista.qml"
    }

    Connections {
            target: sendData

            function onErrorOccurred(errorMessage) {
                textoErro.text = errorMessage
                popupErro.open()
            }
        }

        Popup {
            id: popupErro
            anchors.centerIn: parent
            width: 300
            height: 150
            modal: true
            focus: true
            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

            background: Rectangle {
                color: "white"
                radius: 10
                border.color: "#dc3545"
                border.width: 2
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 10

                Text {
                    text: "⚠️ Erro de Conexão"
                    font.bold: true
                    font.pixelSize: 16
                    color: "#dc3545"
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    id: textoErro
                    text: "Mensagem de erro aqui"
                    wrapMode: Text.WordWrap
                    font.pixelSize: 14
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter
                    horizontalAlignment: Text.AlignHCenter
                }

                Button {
                    text: "Ok, entendi"
                    Layout.alignment: Qt.AlignHCenter
                    onClicked: popupErro.close()
                }
            }
        }
}