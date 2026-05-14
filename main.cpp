#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "receivedata.h"
#include "senddata.h"
#include "savedata.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    // 1. Instanciamos apenas as classes que conversam com a Interface
    SaveData saveData;
    ReceiveData receiveData(&saveData);
    SendData sendData(receiveData.getSerialPort());

    QQmlApplicationEngine engine;

    // 2. Expondo as instâncias para o QML
    engine.rootContext()->setContextProperty("receiveData", &receiveData);
    engine.rootContext()->setContextProperty("sendData", &sendData);
    engine.rootContext()->setContextProperty("saveData", &saveData);

    // Carrega o arquivo QML (Padrão Qt 6)
    engine.loadFromModule("Central_Ar_Condicionado", "Main");

    return app.exec();
}