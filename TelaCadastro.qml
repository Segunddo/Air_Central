import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root

    // Um modelo simples para popular o ComboBox
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

    // Criamos um "Card" branco centralizado para imitar o estilo da lista
        Rectangle {
            width: 450
            height: 380
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
                    Layout.fillWidth: true
                    Layout.preferredHeight: 45
                    font.pixelSize: 16
                }

                // Botão de Ler Código (Ação Secundária)
                Button {
                    text: "📡 Ler Código do Controle"
                    font.bold: true
                    Layout.fillWidth: true
                    Layout.preferredHeight: 45

                    background: Rectangle {
                        color: parent.down ? "#d5e4f5" : "#e3f2fd" // Azul clarinho
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
                        // Passo 1: Avisa a classe ReceiveData o que ela deve esperar
                        receiveData.set_waited_command(comandoEscolhido)
                        // Passo 2: Manda a classe SendData disparar o gatilho pra ESP
                        sendData.require_ir_read()
                        console.log("Iniciando processo de gravação para:", comandoEscolhido)
                    }
                }

                // Botão de Salvar (Ação Principal)
                Button {
                    text: "💾 Salvar Código"
                    font.bold: true
                    Layout.fillWidth: true
                    Layout.preferredHeight: 45

                    background: Rectangle {
                        color: parent.down ? "#388e3c" : "#4CAF50" // Verde
                        radius: 8
                    }

                    contentItem: Text {
                        text: parent.text
                        font: parent.font
                        color: "white" // Texto branco pra contrastar com o verde
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        console.log("Salvando no JSON...")
                    }
                }

                Item { Layout.fillHeight: true } // Espaçador para empurrar o "Voltar" pra baixo

                // Botão Voltar discreto
                Button {
                    text: "← Voltar para a lista"
                    flat: true // Tira o fundo do botão, deixa só o texto
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
