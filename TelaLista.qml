import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {

    ListModel {
        id: modeloLista
    }

    Timer {
        id: refreshTimer
        interval: 1500
        repeat: false
        onTriggered: {
            modeloLista.clear()
            sendData.requireEspsId()
        }
    }

    Connections {
        target: receiveData

        function onNewDeviceDetected(idEsp) {
            console.log("QML ouviu o sinal! Adicionando: " + idEsp)

            // Para n repetir id
            for (var i = 0; i < modeloLista.count; i++) {
                if (modeloLista.get(i).name === idEsp) return;
            }

            modeloLista.append({
                "name": idEsp,
                "status": "Desligado",
                "targetTemp": "22" // Coloca um padrão de 22°C quando um novo ESP entra
            })
        }
    }

    // A Lista
    ListView {

        header: Item {
                width: ListView.view.width
                height: 70
                z: 2

            RowLayout {
                anchors.fill: parent
                anchors.bottomMargin: 10
                spacing: 15

                // Botão refresh
                Button {
                    text: "🔄"
                    font.pixelSize: 22
                    width: 45
                    height: 45

                    Layout.preferredWidth: 45
                    Layout.preferredHeight: 45

                    background: Rectangle {
                        color: parent.down ? "#e0e0e0" : "#ffffff"
                        border.color: "#cccccc"
                        radius: 5 // Bordas levemente arredondadas para não ficar um quadrado seco
                    }

                    onClicked: {
                        console.log("Limpando lista e buscando dispositivos na rede...")
                        modeloLista.clear()

                        // Envia o comando de broadcast
                        sendData.requireEspsId()
                    }
                }
                Button {
                    text: "⚙️ Cadastrar Códigos IR"
                    font.pixelSize: 15
                    font.bold: true
                    Layout.fillWidth: true // Faz ele preencher o resto do espaço
                    Layout.preferredHeight: 45

                    background: Rectangle {
                        color: parent.down ? "#e0e0e0" : "white"
                        border.color: "#cccccc"
                        radius: 8
                    }

                    contentItem: Text {
                        text: parent.text
                        font: parent.font
                        color: "#333333"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        stackView.push("TelaCadastro.qml")
                    }
                }

                ComboBox {
                    id: comboPorta

                    model: receiveData.list_ports()

                    font.pixelSize: 15
                    Layout.fillWidth: true
                    Layout.preferredHeight: 45

                    // Customizando o fundo para manter o estilo do seu TextField
                    background: Rectangle {
                        color: "white"
                        // Fica azul se estiver focado ou com a lista aberta
                        border.color: comboPorta.activeFocus || comboPorta.popup.visible ? "#007BFF" : "#cccccc"
                        border.width: comboPorta.activeFocus || comboPorta.popup.visible ? 2 : 1
                        radius: 8
                    }

                    leftPadding: 15
                    rightPadding: 15
                }

                Button {
                    id: btnConectar
                    text: "🛜 Connectar a Rede"
                    font.pixelSize: 15
                    font.bold: true
                    Layout.fillWidth: true
                    Layout.preferredHeight: 45

                    background: Rectangle {
                        color: parent.down ? "#e0e0e0" : "white"
                        border.color: "#cccccc"
                        radius: 8
                    }

                    contentItem: Text {
                            id: textoDoBotao
                            text: parent.text
                            font: parent.font
                            color: "#333333"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            var porta = comboPorta.currentText.trim();

                            if (porta === "") {
                                console.log("Erro: Digite o nome da porta primeiro!");
                                return;
                            }

                            if(receiveData.conectar(porta)) {
                                btnConectar.text = "🛜 Desconectar a Rede";
                                textoDoBotao.color = "#FF3333";
                            } else {
                                btnConectar.text = "🛜 Connectar a Rede";
                                textoDoBotao.color = "#333333";
                            }
                        }
                }
            }
        }

        id: listaRegistros
        anchors.fill: parent
        anchors.margins: 20
        spacing: 12
        model: modeloLista

        // O Delegate
        delegate: Rectangle {
            width: ListView.view.width
            height: 75
            color: "white"
            radius: 8
            border.color: "#e0e0e0"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 15

                // Coluna de Informações Básicas Nome
                Column {
                    Layout.fillWidth: true
                    spacing: 2

                    RowLayout {
                        spacing: 6
                        width: parent.width

                        TextInput {
                            id: campoNome
                            text: model.name
                            font.pixelSize: 16
                            font.bold: true
                            color: "#333333"
                            Layout.fillWidth: true

                            // Borda sutil aparece só quando está em edição
                            Rectangle {
                                anchors.bottom: parent.bottom
                                width: parent.width
                                height: 2
                                color: parent.activeFocus ? "#007BFF" : "transparent"
                            }
                        }

                        // Botão confirmar — só aparece quando o texto foi alterado
                        Button {
                            text: "✔"
                            font.pixelSize: 14
                            Layout.preferredWidth: 32
                            Layout.preferredHeight: 32
                            visible: campoNome.text !== model.name && campoNome.text.trim() !== ""

                            background: Rectangle {
                                color: parent.down ? "#388E3C" : "#4CAF50"
                                radius: 6
                            }

                            palette.buttonText: "white"

                            onClicked: {
                                let novoId = campoNome.text.trim()
                                let idAntigo = model.name

                                // Pede pra ESP alterar o ID
                                sendData.require_espID_change(idAntigo, novoId)

                                // Aguarda um pouco e faz o refresh
                                refreshTimer.start()
                            }
                        }

                        // Botão cancelar edição
                        Button {
                            text: "✖"
                            font.pixelSize: 14
                            Layout.preferredWidth: 32
                            Layout.preferredHeight: 32
                            visible: campoNome.text !== model.name

                            background: Rectangle {
                                color: parent.down ? "#c62828" : "#e53935"
                                radius: 6
                            }

                            palette.buttonText: "white"

                            onClicked: campoNome.text = model.name
                        }
                    }
                }

                // Botão de Ligar/Desligar
                Button {
                    text: "⏻"
                    font.pixelSize: 18
                    Layout.preferredWidth: 40

                    onClicked: {
                        console.log("Alterando power de: " + model.name)
                        let novoStatus = (model.status === "Ligado") ? "Desligado" : "Ligado"

                        sendData.send_command_status(model.name, novoStatus)
                        modeloLista.setProperty(index, "status", novoStatus)
                    }

                    palette.buttonText: model.status === "Ligado" ? "#4CAF50" : "#d32f2f"
                }

                // Bloco de Controle da Temperatura Alvo (Setas + Display)
                RowLayout {
                    spacing: 5
                    Layout.rightMargin: 20

                    // Botão de Diminuir (▼)
                    Button {
                        text: "▼"
                        font.pixelSize: 18
                        Layout.preferredWidth: 40
                        onClicked: {
                            let tempAtual = parseInt(model.targetTemp)

                            if (tempAtual > 16) {
                                let novaTemp = tempAtual - 1
                                sendData.send_command_temp(model.name, novaTemp.toString())
                                modeloLista.setProperty(index, "targetTemp", novaTemp.toString())
                            }
                        }
                    }

                    // Display da Temperatura Alvo (A que muda ao clicar)
                    Text {
                        text: model.targetTemp + "°C"
                        font.pixelSize: 18
                        font.bold: true
                        color: "#FF9800"
                        Layout.minimumWidth: 45
                        horizontalAlignment: Text.AlignHCenter
                    }

                    // Botão de Aumentar (▲)
                    Button {
                        text: "▲"
                        font.pixelSize: 18
                        Layout.preferredWidth: 40
                        onClicked: {
                            let tempAtual = parseInt(model.targetTemp)

                            if (tempAtual < 30) {
                                let novaTemp = tempAtual + 1
                                sendData.send_command_temp(model.name, novaTemp.toString())
                                model.targetTemp = novaTemp.toString()
                            }
                        }
                    }
                }
            }
        }
    }
}