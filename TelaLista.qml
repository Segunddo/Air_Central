import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: telaListaPrincipal
    anchors.fill: parent

    Rectangle {
        anchors.fill: parent
        color: "#f0f2f5"
        z: -1
    }

    ListModel {
        id: modeloLista
    }

    Timer {
        id: refreshTimer
        interval: 1500
        repeat: false
        onTriggered: {
            modeloLista.clear()
            if (typeof sendData !== "undefined") sendData.requireEspsId()
        }
    }

    Connections {
        target: receiveData
        ignoreUnknownSignals: true
        function onNewDeviceDetected(idEsp) {
            for (var i = 0; i < modeloLista.count; i++) {
                if (modeloLista.get(i).name === idEsp) return;
            }
            modeloLista.append({
                "name": idEsp,
                "status": "Desligado",
                "targetTemp": "22",
                "temAgendamento": false,
                "agendamentos": "[]"
            })
        }
    }

    // ==========================================
    // POPUP DE AGENDAMENTO
    // ==========================================
    Popup {
        id: popupAgendamento
        anchors.centerIn: parent
        width: 360
        height: 500
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        property int espIndex: -1
        property string espName: ""

        ListModel { id: historicoPopup }

        enter: Transition { NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 200 } }
        exit: Transition { NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 150 } }

        onOpened: {
            historicoPopup.clear()
            if(espName !== "") {
                var jsonString = "[]"
                if (typeof saveData !== "undefined") {
                    jsonString = saveData.obter_regras(espName)
                } else {
                    jsonString = modeloLista.get(espIndex).agendamentos
                }

                var jsonSalvo = JSON.parse(jsonString)
                for(var i = 0; i < jsonSalvo.length; i++) {
                    historicoPopup.append(jsonSalvo[i])
                }
                modeloLista.setProperty(espIndex, "agendamentoAtivo", historicoPopup.count > 0)
            }
        }

        background: Rectangle {
            color: "#ffffff"
            radius: 12
            border.color: "#e2e8f0"
            border.width: 1
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 15
            spacing: 12

            Text {
                text: "⏱️ Automações"
                font.pixelSize: 18; font.bold: true; color: "#1e293b"
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: popupAgendamento.espName
                font.pixelSize: 12; color: "#64748b"
                Layout.alignment: Qt.AlignHCenter
                Layout.bottomMargin: 5
            }

            // --- FORMULÁRIO ---
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 125
                color: "#f8fafc"
                radius: 8
                border.color: "#e2e8f0"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 8

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        SpinBox {
                            id: spinHora; from: 0; to: 23; Layout.preferredWidth: 100; editable: true
                            textFromValue: function(value, locale) { return value.toString().padStart(2, '0'); }
                            valueFromText: function(text, locale) { return parseInt(text, 10); }
                            contentItem: TextInput {
                                z: 2
                                text: spinHora.textFromValue(spinHora.value, spinHora.locale)
                                font.pixelSize: 16; font.bold: true; color: "#1e293b"
                                horizontalAlignment: Qt.AlignHCenter; verticalAlignment: Qt.AlignVCenter
                                readOnly: !spinHora.editable; validator: spinHora.validator
                            }
                        }

                        Text { text: ":"; font.pixelSize: 20; font.bold: true; color: "#1e293b" }

                        SpinBox {
                            id: spinMin; from: 0; to: 59; Layout.preferredWidth: 100; editable: true
                            textFromValue: function(value, locale) { return value.toString().padStart(2, '0'); }
                            valueFromText: function(text, locale) { return parseInt(text, 10); }
                            contentItem: TextInput {
                                z: 2
                                text: spinMin.textFromValue(spinMin.value, spinMin.locale)
                                font.pixelSize: 16; font.bold: true; color: "#1e293b"
                                horizontalAlignment: Qt.AlignHCenter; verticalAlignment: Qt.AlignVCenter
                                readOnly: !spinMin.editable; validator: spinMin.validator
                            }
                        }
                        Item { Layout.fillWidth: true }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        ComboBox {
                            id: comboAcao; model: ["Ligar", "Desligar"]; Layout.preferredWidth: 100
                            contentItem: Text { text: comboAcao.displayText; font.pixelSize: 14; font.bold: true; color: "#1e293b"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                        }

                        SpinBox {
                            id: spinTempPopup; from: 16; to: 30; value: 22; visible: comboAcao.currentText === "Ligar"; Layout.preferredWidth: 100
                            textFromValue: function(value, locale) { return value + "°C"; }
                            valueFromText: function(text, locale) { return parseInt(text.replace("°C", ""), 10); }
                            contentItem: TextInput {
                                z: 2
                                text: spinTempPopup.textFromValue(spinTempPopup.value, spinTempPopup.locale)
                                font.pixelSize: 14; font.bold: true; color: "#1e293b"
                                horizontalAlignment: Qt.AlignHCenter; verticalAlignment: Qt.AlignVCenter
                                readOnly: true
                            }
                        }

                        Item { Layout.fillWidth: true }

                        Button {
                            text: "➕"
                            Layout.preferredWidth: 40; Layout.preferredHeight: 40
                            background: Rectangle { color: parent.pressed ? "#16a34a" : "#22c55e"; radius: 8 }
                            contentItem: Text { text: parent.text; color: "white"; font.pixelSize: 18; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            onClicked: {
                                var hStr = spinHora.value.toString().padStart(2, '0')
                                var mStr = spinMin.value.toString().padStart(2, '0')
                                var horaFormatada = hStr + ":" + mStr
                                var acao = comboAcao.currentText
                                var temp = acao === "Ligar" ? spinTempPopup.value.toString() : "--"

                                if (typeof saveData !== "undefined") saveData.adicionar_regra(popupAgendamento.espName, horaFormatada, acao, temp)
                                historicoPopup.append({ "hora": horaFormatada, "acao": acao, "temp": temp })
                                modeloLista.setProperty(popupAgendamento.espIndex, "temAgendamento", true)
                            }
                        }
                    }
                }
            }

            // --- HISTÓRICO VISUAL ---
            Text { text: "Regras Salvas:"; font.pixelSize: 12; color: "#64748b"; font.bold: true; Layout.topMargin: 5 }

            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: historicoPopup
                spacing: 5
                clip: true

                delegate: Rectangle {
                    width: ListView.view.width
                    height: 45
                    color: "#ffffff"
                    border.color: "#e2e8f0"
                    radius: 6

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 10

                        Text { text: model.hora; font.pixelSize: 16; font.bold: true; color: "#1e293b" }
                        Text { text: model.acao === "Ligar" ? "Ligar a " + model.temp + "°C" : "Desligar"; font.pixelSize: 14; color: model.acao === "Ligar" ? "#22c55e" : "#ef4444"; Layout.fillWidth: true }

                        Button {
                            text: "✖"
                            Layout.preferredWidth: 35; Layout.preferredHeight: 35
                            contentItem: Text { text: parent.text; font.pixelSize: 16; font.bold: true; color: "#ef4444"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            background: Rectangle { color: parent.pressed ? "#fee2e2" : parent.hovered ? "#fff1f1" : "transparent"; radius: 17.5 }
                            onClicked: {
                                if (typeof saveData !== "undefined") saveData.remover_regra(popupAgendamento.espName, index)
                                historicoPopup.remove(index)
                                if (historicoPopup.count === 0) modeloLista.setProperty(popupAgendamento.espIndex, "temAgendamento", false)
                            }
                        }
                    }
                }
            }

            // --- BOTÕES RODAPÉ ---
            RowLayout {
                Layout.fillWidth: true
                spacing: 10
                Layout.topMargin: 5

                Button {
                    text: "Fechar"
                    Layout.fillWidth: true; Layout.preferredHeight: 40
                    background: Rectangle { color: parent.pressed ? "#e2e8f0" : "#f1f5f9"; radius: 8 }
                    contentItem: Text { text: parent.text; color: "#64748b"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; font.bold: true }
                    onClicked: popupAgendamento.close()
                }
            }
        }
    }
    // ==========================================
    // FIM DO POPUP
    // ==========================================

    ListView {
        id: listaRegistros
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15
        model: modeloLista
        clip: true

        header: Item {
            width: ListView.view.width
            height: 80
            z: 2
            RowLayout {
                anchors.fill: parent; anchors.bottomMargin: 20; spacing: 10

                Button {
                    text: "🔄"; font.pixelSize: 20; Layout.preferredWidth: 45; Layout.preferredHeight: 45
                    background: Rectangle { color: parent.pressed ? "#e2e8f0" : "#ffffff"; border.color: "#cbd5e1"; radius: 10 }
                    onClicked: { modeloLista.clear(); if (typeof sendData !== "undefined") sendData.requireEspsId(); }
                }

                // --- NOVO BOTÃO: IMPORTAR GRADE DO SASI ---
                Button {
                    text: "📥 Importar SASI"
                    font.pixelSize: 14; font.bold: true; Layout.fillWidth: true; Layout.preferredHeight: 45
                    contentItem: Text { text: parent.text; font: parent.font; color: "#ffffff"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    background: Rectangle { color: parent.pressed ? "#16a34a" : "#22c55e"; border.color: "#16a34a"; radius: 10 } // Verde pra dar destaque na importação
                    onClicked: {
                        console.log("Iniciando integração com a API do SASI...")
                        // saveData.baixar_horarios_sasi() -> Futura chamada pro C++

                        // Opcional: já dá um refresh visual na tela após importar
                        refreshTimer.start()
                    }
                }

                Button {
                    text: "⚙️ Códigos IR"; font.pixelSize: 14; font.bold: true; Layout.fillWidth: true; Layout.preferredHeight: 45
                    contentItem: Text { text: parent.text; font: parent.font; color: "#334155"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    background: Rectangle { color: parent.pressed ? "#e2e8f0" : "#ffffff"; border.color: "#cbd5e1"; radius: 10 }
                    onClicked: { if (typeof stackView !== "undefined") stackView.push("TelaCadastro.qml") }
                }

                ComboBox {
                    id: comboPorta; model: (typeof receiveData !== "undefined") ? receiveData.list_ports() : ["COM1", "COM2"]; font.pixelSize: 14; Layout.fillWidth: true; Layout.preferredHeight: 45
                    background: Rectangle { color: "#ffffff"; border.color: "#cbd5e1"; radius: 10 } leftPadding: 15; rightPadding: 15
                }

                Button {
                    id: btnConectar; text: "🛜 Conectar"; font.pixelSize: 14; font.bold: true; Layout.fillWidth: true; Layout.preferredHeight: 45
                    contentItem: Text { id: textoDoBotao; text: parent.text; font: parent.font; color: "#334155"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    background: Rectangle { color: parent.pressed ? "#e2e8f0" : "#ffffff"; border.color: "#cbd5e1"; radius: 10 }
                    onClicked: {
                        var porta = comboPorta.currentText.trim();
                        if (porta === "" || typeof receiveData === "undefined") return;
                        if(receiveData.conectar(porta)) { btnConectar.text = "🛜 Desconectar"; textoDoBotao.color = "#ef4444"; }
                        else { btnConectar.text = "🛜 Conectar"; textoDoBotao.color = "#334155"; }
                    }
                }
            }
        }

        delegate: Rectangle {
            width: ListView.view.width
            height: 85
            color: "#ffffff"
            radius: 12
            border.color: model.temAgendamento ? "#3b82f6" : "#e2e8f0"
            border.width: model.temAgendamento ? 2 : 1

            Rectangle { anchors.fill: parent; anchors.margins: -1; color: "transparent"; border.color: "#08000000"; radius: 13; z: -1 }

            RowLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 15

                Column {
                    Layout.fillWidth: true
                    spacing: 4

                    Text { text: "Dispositivo ESP32 Mesh"; font.pixelSize: 10; color: "#94a3b8"; font.bold: true }

                    RowLayout {
                        spacing: 8
                        width: parent.width

                        TextInput {
                            id: campoNome; text: model.name; font.pixelSize: 17; font.bold: true; color: "#1e293b"; Layout.fillWidth: true; selectByMouse: true
                            Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 2; color: parent.activeFocus ? "#3b82f6" : "transparent" }
                        }
                        Button {
                            text: "✔"; font.pixelSize: 12; Layout.preferredWidth: 32; Layout.preferredHeight: 32; visible: campoNome.text !== model.name && campoNome.text.trim() !== ""
                            contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            background: Rectangle { color: parent.pressed ? "#16a34a" : "#22c55e"; radius: 8 }
                            onClicked: { let novoId = campoNome.text.trim(); if (typeof sendData !== "undefined") sendData.require_espID_change(model.name, novoId); if (typeof sendData === "undefined") modeloLista.setProperty(index, "name", novoId); refreshTimer.start() }
                        }
                        Button {
                            text: "✖"; font.pixelSize: 12; Layout.preferredWidth: 32; Layout.preferredHeight: 32; visible: campoNome.text !== model.name
                            contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; font.bold: true }
                            background: Rectangle { color: parent.pressed ? "#dc2626" : "#ef4444"; radius: 8 }
                            onClicked: campoNome.text = model.name
                        }
                    }

                    Text {
                        visible: model.temAgendamento
                        text: "⏰ Automações configuradas"
                        font.pixelSize: 11
                        color: "#3b82f6"
                        font.bold: true
                    }
                }

                Rectangle { Layout.preferredWidth: 1; Layout.fillHeight: true; color: "#f1f5f9"; Layout.margins: 5 }

                // --- BOTÃO QUE ABRE O POPUP ---
                Button {
                    Layout.preferredWidth: 40; Layout.preferredHeight: 50
                    contentItem: Text { text: "⏱️"; font.pixelSize: 20; opacity: model.temAgendamento ? 1.0 : 0.4; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    background: Item {}
                    onClicked: {
                        popupAgendamento.espIndex = index
                        popupAgendamento.espName = model.name
                        popupAgendamento.open()
                    }
                }

                Button {
                    Layout.preferredWidth: 50; Layout.preferredHeight: 50
                    contentItem: Text { text: "⏻"; font.pixelSize: 22; color: model.status === "Ligado" ? "#ffffff" : "#64748b"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    background: Rectangle { radius: 25; color: model.status === "Ligado" ? "#22c55e" : "#f1f5f9"; border.color: model.status === "Ligado" ? "#16a34a" : "#cbd5e1"; border.width: 1 }
                    onClicked: { let novoStatus = (model.status === "Ligado") ? "Desligado" : "Ligado"; modeloLista.setProperty(index, "status", novoStatus); if (typeof sendData !== "undefined") sendData.send_command_status(model.name, novoStatus) }
                }

                // --- BLOCO DE CONTROLE DA TEMPERATURA ---
                Rectangle {
                    Layout.preferredWidth: 130; Layout.preferredHeight: 46; radius: 23; color: "#f8fafc"; border.color: "#e2e8f0"
                    RowLayout {
                        anchors.fill: parent; spacing: 0
                        Button {
                            text: "−"; font.pixelSize: 18; Layout.preferredWidth: 35; Layout.fillHeight: true
                            contentItem: Text { text: parent.text; font: parent.font; color: "#64748b"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            background: Item {}
                            onClicked: { let tempAtual = parseInt(model.targetTemp); if (tempAtual > 16) { let novaTemp = tempAtual - 1; modeloLista.setProperty(index, "targetTemp", novaTemp.toString()); if (typeof sendData !== "undefined") sendData.send_command_temp(model.name, novaTemp.toString()) } }
                        }

                        Text {
                            text: model.targetTemp + "°"
                            font.pixelSize: 17; font.bold: true; color: "#f97316"
                            Layout.preferredWidth: 60
                            horizontalAlignment: Text.AlignHCenter
                        }

                        Button {
                            text: "+"; font.pixelSize: 18; Layout.preferredWidth: 35; Layout.fillHeight: true
                            contentItem: Text { text: parent.text; font: parent.font; color: "#64748b"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                            background: Item {}
                            onClicked: { let tempAtual = parseInt(model.targetTemp); if (tempAtual < 30) { let novaTemp = tempAtual + 1; modeloLista.setProperty(index, "targetTemp", novaTemp.toString()); if (typeof sendData !== "undefined") sendData.send_command_temp(model.name, novaTemp.toString()) } }
                        }
                    }
                }
            }
        }
    }
}
