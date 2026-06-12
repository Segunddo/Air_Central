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
        Component.onCompleted: {
                    // 1. O Auditório que você pediu (Com agendamento)
        append({
            "name": "Auditório - T03",
            "status": "Desligado",
            "targetTemp": "22",
        })
        }
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

    // Monitora o sinal de conclusão do Python para recarregar as regras visualmente
    Connections {
        target: (typeof saveData !== "undefined") ? saveData : null
        ignoreUnknownSignals: true
        function onAgendamentos_atualizados() {
            console.log("Interface atualizada com os novos dados estruturados do SACI!");
            refreshTimer.start()
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
            width: 450
            height: 600
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
                    modeloLista.setProperty(espIndex, "temAgendamento", historicoPopup.count > 0)
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
                anchors.margins: 20 // Mais margem nas bordas
                spacing: 15

                // Cabeçalho
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    Text {
                        text: "⏱️ Automações de Sala"
                        font.pixelSize: 18; font.bold: true; color: "#1e293b"
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        text: popupAgendamento.espName
                        font.pixelSize: 13; color: "#64748b"
                        Layout.alignment: Qt.AlignHCenter
                    }
                }

                // Bloco de Configuração da Nova Regra (Visual de "Card")
                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: formNovo.implicitHeight + 30 // Altura se ajusta ao conteúdo
                    color: "#f8fafc"
                    radius: 10
                    border.color: "#e2e8f0"

                    ColumnLayout {
                        id: formNovo
                        anchors.fill: parent
                        anchors.margins: 15
                        spacing: 12

                        // Linha 1: Dia da Semana e Horário
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 15

                            // Coluna Dia
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 4
                                Text { text: "Dia da Semana"; font.pixelSize: 12; font.bold: true; color: "#64748b" }
                                ComboBox {
                                    id: comboDia
                                    model: ["Seg", "Ter", "Qua", "Qui", "Sex", "Sab"]
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 40
                                }
                            }

                            // Coluna Horário
                            ColumnLayout {
                                spacing: 4
                                Text { text: "Horário"; font.pixelSize: 12; font.bold: true; color: "#64748b" }
                                RowLayout {
                                    spacing: 4
                                    SpinBox {
                                        id: spinHora; from: 0; to: 23; editable: true
                                        // AUMENTADO DE 80 PARA 120
                                        Layout.preferredWidth: 120; Layout.preferredHeight: 40
                                        textFromValue: function(value, locale) { return value.toString().padStart(2, '0'); }
                                        valueFromText: function(text, locale) { return parseInt(text, 10); }
                                        contentItem: TextInput {
                                            z: 2
                                            text: spinHora.textFromValue(spinHora.value, spinHora.locale)
                                            font.pixelSize: 15; font.bold: true; color: "#1e293b"
                                            horizontalAlignment: Qt.AlignHCenter; verticalAlignment: Qt.AlignVCenter
                                            readOnly: !spinHora.editable; validator: spinHora.validator
                                        }
                                    }
                                    Text { text: ":"; font.pixelSize: 16; font.bold: true; color: "#1e293b" }
                                    SpinBox {
                                        id: spinMin; from: 0; to: 59; editable: true
                                        // AUMENTADO DE 80 PARA 120
                                        Layout.preferredWidth: 120; Layout.preferredHeight: 40
                                        textFromValue: function(value, locale) { return value.toString().padStart(2, '0'); }
                                        valueFromText: function(text, locale) { return parseInt(text, 10); }
                                        contentItem: TextInput {
                                            z: 2
                                            text: spinMin.textFromValue(spinMin.value, spinMin.locale)
                                            font.pixelSize: 15; font.bold: true; color: "#1e293b"
                                            horizontalAlignment: Qt.AlignHCenter; verticalAlignment: Qt.AlignVCenter
                                            readOnly: !spinMin.editable; validator: spinMin.validator
                                        }
                                    }
                                }
                            }
                        }

                        // Linha 2: Ação e Temperatura
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 15

                            // Coluna Ação
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 4
                                Text { text: "Ação"; font.pixelSize: 12; font.bold: true; color: "#64748b" }
                                ComboBox {
                                    id: comboAcao; model: ["Ligar", "Desligar"]
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 40
                                }
                            }

                            // Coluna Temperatura
                            ColumnLayout {
                                // Fixamos a largura para alinhar com o bloco de cima e não dançar na tela
                                Layout.preferredWidth: 175
                                spacing: 4
                                Text {
                                    text: "Temperatura"
                                    font.pixelSize: 12; font.bold: true; color: "#64748b"
                                    visible: comboAcao.currentText === "Ligar"
                                }
                                SpinBox {
                                    id: spinTempPopup; from: 16; to: 30; value: 22
                                    visible: comboAcao.currentText === "Ligar"
                                    Layout.fillWidth: true; Layout.preferredHeight: 40
                                    textFromValue: function(value, locale) { return value + "°C"; }
                                    valueFromText: function(text, locale) { return parseInt(text.replace("°C", ""), 10); }
                                }
                            }
                        }

                        // Linha 3: Botão Gigante de Adicionar
                        Button {
                            text: "➕ Adicionar Regra"
                            Layout.fillWidth: true
                            Layout.preferredHeight: 42
                            Layout.topMargin: 8
                            background: Rectangle { color: parent.pressed ? "#16a34a" : "#22c55e"; radius: 8 }
                            contentItem: Text {
                                text: parent.text; color: "white"; font.pixelSize: 15;
                                font.bold: true; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                            }
                            onClicked: {
                                var hStr = spinHora.value.toString().padStart(2, '0')
                                var mStr = spinMin.value.toString().padStart(2, '0')
                                var horaFormatada = hStr + ":" + mStr
                                var acao = comboAcao.currentText
                                var diaSelecionado = comboDia.currentText
                                var temp = acao === "Ligar" ? spinTempPopup.value.toString() : "--"

                                if (typeof saveData !== "undefined") {
                                    saveData.adicionar_regra(popupAgendamento.espName, horaFormatada, acao, temp, diaSelecionado)
                                }
                                historicoPopup.append({ "hora": horaFormatada, "acao": acao, "temp": temp, "dia": diaSelecionado })
                                modeloLista.setProperty(popupAgendamento.espIndex, "temAgendamento", true)
                            }
                        }
                    }
                }

                Text { text: "Regras Salvas:"; font.pixelSize: 12; color: "#64748b"; font.bold: true; Layout.topMargin: 5 }

                // Lista de Regras
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: historicoPopup
                    spacing: 6
                    clip: true
                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 48
                        color: "#ffffff"
                        border.color: "#e2e8f0"
                        radius: 8
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 12

                            Rectangle {
                                color: "#eff6ff"
                                radius: 6
                                Layout.preferredWidth: 42; Layout.preferredHeight: 28
                                Text {
                                    text: model.dia ? model.dia : "Seg"
                                    font.pixelSize: 12; font.bold: true; color: "#2563eb"
                                    anchors.centerIn: parent
                                }
                            }

                            Text { text: model.hora; font.pixelSize: 16; font.bold: true; color: "#1e293b" }
                            Text {
                                text: model.acao === "Ligar" ? "Ligar a " + model.temp + "°C" : "Desligar"
                                font.pixelSize: 14; font.bold: true
                                color: model.acao === "Ligar" ? "#22c55e" : "#ef4444"
                                Layout.fillWidth: true
                            }

                            Button {
                                text: "✖"
                                Layout.preferredWidth: 32; Layout.preferredHeight: 32
                                contentItem: Text { text: parent.text; font.pixelSize: 14; font.bold: true; color: "#ef4444"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                background: Rectangle { color: parent.pressed ? "#fee2e2" : "transparent"; radius: 16 }
                                onClicked: {
                                    if (typeof saveData !== "undefined") saveData.remover_regra(popupAgendamento.espName, index)
                                    historicoPopup.remove(index)
                                    if (historicoPopup.count === 0) modeloLista.setProperty(popupAgendamento.espIndex, "temAgendamento", false)
                                }
                            }
                        }
                    }
                }

                Button {
                    text: "Fechar"
                    Layout.fillWidth: true; Layout.preferredHeight: 42
                    background: Rectangle { color: parent.pressed ? "#e2e8f0" : "#f1f5f9"; radius: 8 }
                    contentItem: Text { text: parent.text; color: "#64748b"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; font.bold: true }
                    onClicked: popupAgendamento.close()
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
            height: 70
            z: 2

            RowLayout {
                anchors.fill: parent
                anchors.bottomMargin: 15
                spacing: 8

                Button {
                    text: "🔄"
                    Layout.preferredWidth: 42; Layout.preferredHeight: 42
                    background: Rectangle { color: parent.pressed ? "#e2e8f0" : "#ffffff"; border.color: "#cbd5e1"; radius: 10 }
                    contentItem: Text { text: parent.text; font.pixelSize: 16; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    onClicked: {
                        modeloLista.clear()
                        if (typeof receiveData !== "undefined") comboPorta.model = receiveData.list_ports()
                        if (typeof sendData !== "undefined") sendData.requireEspsId()
                    }
                }

                // Botão de Importação Corrigido para Chamar o Script Python pelo Backend C++
                Button {
                    text: "📥 Importar SACI"
                    Layout.preferredWidth: 150; Layout.preferredHeight: 42
                    font.pixelSize: 13; font.bold: true
                    contentItem: Text { text: parent.text; font: parent.font; color: "#ffffff"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    background: Rectangle { color: parent.pressed ? "#16a34a" : "#22c55e"; radius: 10 }
                    onClicked: {
                        console.log("Executando parser de salas via script python assíncrono...")
                        if (typeof saveData !== "undefined") {
                            // Executa a carga para o Centro 'CI', com temperatura alvo padrão em 22°C
                            saveData.importar_saci("CI", 22)
                        } else {
                            refreshTimer.start()
                        }
                    }
                }

                Button {
                    text: "⚙️ Códigos IR"
                    Layout.preferredWidth: 130; Layout.preferredHeight: 42
                    font.pixelSize: 13; font.bold: true
                    contentItem: Text { text: parent.text; font: parent.font; color: "#334155"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    background: Rectangle { color: parent.pressed ? "#e2e8f0" : "#ffffff"; border.color: "#cbd5e1"; radius: 10 }
                    onClicked: { if (typeof stackView !== "undefined") stackView.push("TelaCadastro.qml") }
                }

                Item { Layout.fillWidth: true }

                ComboBox {
                    id: comboPorta
                    model: (typeof receiveData !== "undefined") ? receiveData.list_ports() : ["COM1", "COM2"]
                    font.pixelSize: 13
                    Layout.preferredWidth: 120; Layout.preferredHeight: 42
                    background: Rectangle { color: "#ffffff"; border.color: "#cbd5e1"; radius: 10 }
                    leftPadding: 12; rightPadding: 12
                }

                Button {
                    id: btnConectar
                    text: "🛜 Conectar"
                    Layout.preferredWidth: 130; Layout.preferredHeight: 42
                    font.pixelSize: 13; font.bold: true
                    contentItem: Text {
                        id: textoDoBotao
                        text: parent.text; font: parent.font; color: "#334155"
                        horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle { color: parent.pressed ? "#e2e8f0" : "#ffffff"; border.color: "#cbd5e1"; radius: 10 }
                    onClicked: {
                        var porta = comboPorta.currentText.trim()
                        if (porta === "" || typeof receiveData === "undefined") return
                        if (receiveData.conectar(porta)) {
                            btnConectar.text = "🛜 Desconectar"
                            textoDoBotao.color = "#ef4444"
                        } else {
                            btnConectar.text = "🛜 Conectar"
                            textoDoBotao.color = "#334155"
                        }
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
                            background: Rectangle { color: parent.pressed ? "#ef4444" : "#ef4444"; radius: 8 }
                            onClicked: campoNome.text = model.name
                        }
                    }
                    Text {
                        visible: model.temAgendamento
                        text: "⏰ Automações configuradas"
                        font.pixelSize: 11; color: "#3b82f6"; font.bold: true
                    }
                }
                Rectangle { Layout.preferredWidth: 1; Layout.fillHeight: true; color: "#f1f5f9"; Layout.margins: 5 }
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
