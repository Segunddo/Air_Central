import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root

    property int contagemAtual: 0

    function atualizarContagem() {
        if (saveData && typeof saveData.get_command_count === "function") {
            contagemAtual = saveData.get_command_count(comboComando.currentValue)
        } else {
            contagemAtual = 0;
        }
    }

    // 1. ATUALIZADO: Agora escuta o sinal correto do C++
    Connections {
        target: saveData

        function onCodes_size_changed() {
            atualizarContagem()
        }
    }

    ListModel {
        id: comandosModel
        ListElement { text: "Ligar"; value: "Ligar" }
        ListElement { text: "Desligar"; value: "Desligar" }
        ListElement { text: "16°C"; value: "16" }
        ListElement { text: "17°C"; value: "17" }
        ListElement { text: "18°C"; value: "18" }
        ListElement { text: "19°C"; value: "19" }
        ListElement { text: "20°C"; value: "20" }
        ListElement { text: "21°C"; value: "21" }
        ListElement { text: "22°C"; value: "22" }
        ListElement { text: "23°C"; value: "23" }
        ListElement { text: "24°C"; value: "24" }
        ListElement { text: "25°C"; value: "25" }
        ListElement { text: "26°C"; value: "26" }
        ListElement { text: "27°C"; value: "27" }
        ListElement { text: "28°C"; value: "28" }
        ListElement { text: "29°C"; value: "29" }
        ListElement { text: "30°C"; value: "30" }
    }

    // --- POPUP DE CONFIRMAÇÃO: APAGAR TUDO ---
    Popup {
        id: popupApagar
        width: 320
        height: 180
        anchors.centerIn: parent
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: "white"
            radius: 12
            border.color: "#d32f2f"
            border.width: 2
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 15

            Text {
                text: "⚠️ Atenção"
                font.bold: true
                font.pixelSize: 18
                color: "#d32f2f"
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: "Tem certeza que deseja apagar todos os códigos? Essa ação não pode ser desfeita."
                wrapMode: Text.WordWrap
                font.pixelSize: 14
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                spacing: 20

                Button {
                    text: "Cancelar"
                    onClicked: popupApagar.close()
                }

                Button {
                    text: "Sim, apagar"
                    background: Rectangle {
                        color: "#d32f2f"
                        radius: 6
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: {
                        saveData.delete_all_data();
                        // O atualizarContagem agora é chamado automaticamente pelo onCodes_size_changed
                        popupApagar.close()
                    }
                }
            }
        }
    }

    // --- POPUP DE CONFIRMAÇÃO: APAGAR ÚLTIMO CÓDIGO ---
    Popup {
        id: popupApagarUltimo
        width: 320
        height: 180
        anchors.centerIn: parent
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: "white"
            radius: 12
            border.color: "#f57c00"
            border.width: 2
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 15

            Text {
                text: "↩️ Desfazer Leitura"
                font.bold: true
                font.pixelSize: 18
                color: "#f57c00"
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: "Deseja apagar apenas o último código gravado para " + comboComando.currentText + "?"
                wrapMode: Text.WordWrap
                font.pixelSize: 14
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                spacing: 20

                Button {
                    text: "Cancelar"
                    onClicked: popupApagarUltimo.close()
                }

                Button {
                    text: "Sim, apagar"
                    background: Rectangle {
                        color: "#f57c00"
                        radius: 6
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: {
                        // 2. ATUALIZADO: Passando o valor atual do combobox
                        saveData.delete_data(comboComando.currentValue)
                        popupApagarUltimo.close()
                    }
                }
            }
        }
    }

    Rectangle {
        width: 450
        height: 500
        anchors.centerIn: parent
        color: "white"
        radius: 12
        border.color: "#e0e0e0"

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 30
            spacing: 20

            Text {
                text: "Configuração de Códigos IR"
                font.pixelSize: 20
                font.bold: true
                color: "#333333"
                Layout.alignment: Qt.AlignHCenter
                Layout.bottomMargin: 10
            }

            Text {
                text: "Selecione o comando a ser mapeado:"
                font.pixelSize: 14
                color: "#666666"
            }

            ComboBox {
                id: comboComando
                model: comandosModel
                textRole: "text"
                valueRole: "value"
                Layout.fillWidth: true
                Layout.preferredHeight: 45
                font.pixelSize: 16

                onCurrentValueChanged: {
                    atualizarContagem()
                }

                Component.onCompleted: {
                    atualizarContagem()
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: -10
                spacing: 10

                Text {
                    text: "Códigos registrados para este comando:"
                    font.pixelSize: 13
                    color: "#666666"
                    Layout.fillWidth: true
                }

                Rectangle {
                    width: 26
                    height: 26
                    radius: 13
                    color: root.contagemAtual > 0 ? "#4caf50" : "#e0e0e0"

                    Behavior on color { ColorAnimation { duration: 200 } }

                    Text {
                        text: root.contagemAtual
                        color: root.contagemAtual > 0 ? "white" : "#666666"
                        font.bold: true
                        font.pixelSize: 13
                        anchors.centerIn: parent
                    }
                }
            }

            Button {
                text: "📡 Ler Código do Controle"
                font.bold: true
                Layout.fillWidth: true
                Layout.preferredHeight: 45

                background: Rectangle {
                    color: parent.down ? "#d5e4f5" : "#e3f2fd"
                    border.color: "#90caf9"
                    radius: 8
                }

                contentItem: Text {
                    text: parent.text
                    font: parent.font
                    color: "#1565c0"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    let comandoEscolhido = comboComando.currentValue
                    receiveData.set_waited_command(comandoEscolhido)
                    sendData.require_ir_read()
                    console.log("Iniciando processo de gravação para:", comandoEscolhido)
                }
            }

            Item { Layout.fillHeight: true }

            // 3. ATUALIZADO: Agora o botão usa a "contagemAtual" para decidir se pode ser clicado
            Button {
                text: root.contagemAtual > 0 ? "↩️ Apagar último de " + comboComando.currentText : "↩️ Apagar último código"
                flat: true
                Layout.alignment: Qt.AlignHCenter

                enabled: root.contagemAtual > 0
                opacity: enabled ? 1.0 : 0.5

                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 13
                    color: "#f57c00"
                    horizontalAlignment: Text.AlignHCenter
                }

                onClicked: {
                    popupApagarUltimo.open()
                }
            }

            Button {
                text: "🗑️ Apagar todos os códigos"
                flat: true
                Layout.alignment: Qt.AlignHCenter

                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 13
                    color: "#d32f2f"
                    horizontalAlignment: Text.AlignHCenter
                }

                onClicked: {
                    popupApagar.open()
                }
            }

            Button {
                text: "← Voltar para a lista"
                flat: true
                Layout.alignment: Qt.AlignHCenter

                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 14
                    color: "#757575"
                    horizontalAlignment: Text.AlignHCenter
                }

                onClicked: {
                    stackView.pop()
                }
            }
        }
    }
}