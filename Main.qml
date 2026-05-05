import QtQuick 2.15
import QtQuick.Controls 2.15

ApplicationWindow {
    visible: true
    width: 750
    height: 450
    title: "Gerenciamento Ar Condicionado"
    color: "#f4f4f4"

    // O StackView gerencia a transição de telas
    StackView {
        id: stackView
        anchors.fill: parent

        // Define qual tela aparece primeiro (seu código atual)
        initialItem: "TelaLista.qml"
    }
}
