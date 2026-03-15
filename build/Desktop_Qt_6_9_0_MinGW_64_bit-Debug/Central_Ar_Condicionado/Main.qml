import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    visible: true
    width: 750 // Aumentei um pouco a largura para caber tudo folgado
    height: 450
    title: "Gerenciamento Ar Condicionado"
    color: "#f4f4f4"

    // Modelo teste
    ListModel {
        id: modeloLista
        ListElement { name: "Sala: CI 101"; status: "Ligado"; sensorTemp: "27"; targetTemp: "22"}
        ListElement { name: "Sala: CI 201"; status: "Desligado"; sensorTemp: "30"; targetTemp: "24"}
        ListElement { name: "Sala: CI 301"; status: "Desligado"; sensorTemp: "25"; targetTemp: "23"}
    }

    Connections {
        target: receiveData

        function onNewDeviceDetected(idEsp, status, temperature) {
            console.log("QML ouviu o sinal! Adicionando: " + idEsp)

            modeloLista.append({
                "name": idEsp,
                "status": status,
                "sensorTemp": temperature,
                "targetTemp": "24" // Coloca um padrão de 24°C quando um novo ESP entra
            })
        }
    }

    // A Lista
    ListView {
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

                        sendData.sendComand(model.name, novoStatus)
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
                                sendData.sendComand(model.name, "", novaTemp.toString())
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
                                sendData.sendComand(model.name, "", novaTemp.toString())
                                model.targetTemp = novaTemp.toString()
                            }
                        }
                    }
                }

                // Retorno do Sensor (Fixo à direita)
                Column {
                    Layout.alignment: Qt.AlignVCenter
                    Layout.rightMargin: 10

                    Text {
                        text: "Ambiente"
                        font.pixelSize: 11
                        color: "#888888"
                        horizontalAlignment: Text.AlignHCenter
                        width: parent.width
                    }
                    Text {
                        text: model.sensorTemp + "°C"
                        font.pixelSize: 22
                        font.bold: true
                        color: "#2196F3"
                    }
                }
            }
        }
    }
}
