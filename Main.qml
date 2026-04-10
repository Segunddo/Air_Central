import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    visible: true
    width: 750
    height: 450
    title: "Gerenciamento Ar Condicionado"
    color: "#f4f4f4"

    // Modelo teste
    ListModel {
        id: modeloLista
        ListElement { name: "Sala: CI 101"; status: "Ligado"; sensorTemp: "27"; targetTemp: "22"}
        ListElement { name: "Sala: CI 201"; status: "Desligado"; sensorTemp: "30"; targetTemp: "24"}
        ListElement { name: "Sala: CI 301"; status: "Desligado"; sensorTemp: "25"; targetTemp: "21"}
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

        // Botão refresh
        header: Item {
                width: ListView.view.width
                height: 60
                z: 2

                Button {
                    text: "🔄"
                    font.pixelSize: 22
                    width: 45
                    height: 45

                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter

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

                // Coluna de Informações Básicas (Nome e Status)
                Column {
                    Layout.fillWidth: true
                    spacing: 2

                    Text {
                        text: model.name
                        font.pixelSize: 16
                        font.bold: true
                        color: "#333333"
                    }
                    Text {
                        text: "Status: " + model.status
                        font.pixelSize: 13
                        color: model.status === "Ligado" ? "#4CAF50" : "#F44336"
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

                        sendData.sendComand(model.name, novoStatus, "-1", modeloLista.count)
                        model.status = novoStatus
                    }

                    palette.buttonText: model.status === "Ligado" ? "#d32f2f" : "#4CAF50"
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
                                sendData.sendComand(model.name, "-1", novaTemp.toString(), modeloLista.count)
                                model.targetTemp = novaTemp.toString()
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
                                sendData.sendComand(model.name, "-1", novaTemp.toString(), modeloLista.count)
                                model.targetTemp = novaTemp.toString()
                            }
                        }
                    }
                }
            }
        }
    }
}
